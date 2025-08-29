#include <random>
#include <algorithm>
#include <cmath>

#include "our_gl.h"
#include "model.h"

extern mat<4,4> Viewport, ModelView, Perspective; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;     // the depth buffer

struct EmptyShader : IShader {
    const Model &model;

    EmptyShader(const Model &m) : model(m) {}

    virtual vec4 vertex(const int face, const int vert) {
        vec4 gl_Position = ModelView * model.vert(face, vert);
        return Perspective * gl_Position;
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        TGAColor gl_FragColor = {255, 255, 255, 255};
        return {false, gl_FragColor};
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width  = 800;      // output image size
    constexpr int height = 800;
    constexpr int shadoww = 8000;    // shadow map buffer size
    constexpr int shadowh = 8000;
    constexpr vec3    eye{-1, 0, 2}; // camera position
    constexpr vec3 center{ 0, 0, 0}; // camera direction
    constexpr vec3     up{ 0, 1, 0}; // camera up vector

    // usual rendering pass
    lookat(eye, center, up);
    init_perspective(norm(eye-center));
    init_viewport(width/16, height/16, width*7/8, height*7/8);
    init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});

    for (int m=1; m<argc; m++) {                    // iterate through all input objects
        Model model(argv[m]);                       // load the data
        EmptyShader shader{model};
        for (int f=0; f<model.nfaces(); f++) {      // iterate through all facets
            Triangle clip = { shader.vertex(f, 0),  // assemble the primitive
                              shader.vertex(f, 1),
                              shader.vertex(f, 2) };
            rasterize(clip, shader, framebuffer);   // rasterize the primitive
        }
    }
    framebuffer.write_tga_file("framebuffer.tga");

    std::vector<double> mask(width*height, 0);
    std::vector<double> zbuffer_copy = zbuffer;
    mat<4,4> M = (Viewport * Perspective * ModelView).invert();

    // shadow rendering pass
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    constexpr int n = 1000;

    auto smoothstep = [](double edge0, double edge1, double x) {         // smoothstep returns 0 if the input is less than the left edge,
            double t = std::clamp((x - edge0)/(edge1 - edge0), 0., 1.);  // 1 if the input is greater than the right edge,
            return t*t*(3 - 2*t);                                        // Hermite interpolation inbetween. The derivative of the smoothstep function is zero at both edges.
    };

    for (int i = 0; i < n; ++i) {
        std::cerr << i << std::endl;
        double y = dist(gen);
        double theta = 2.0 * M_PI * dist(gen);
        double r = std::sqrt(1.0 - y*y);
        vec3 light = vec3{r * std::cos(theta), y, r * std::sin(theta)}*1.5;

        std::cout << "v " << light << std::endl;

        lookat(light, center, up);
        ModelView[3][3] =    norm(light-center);

        init_perspective(norm(eye-center));
        init_viewport(shadoww/16, shadowh/16, shadoww*7/8, shadowh*7/8);
        init_zbuffer(shadoww, shadowh);
        TGAImage trash(shadoww, shadowh, TGAImage::RGB, {177, 195, 209, 255});

        for (int m=1; m<argc; m++) {                    // iterate through all input objects
            Model model(argv[m]);                       // load the data
            EmptyShader shader{model};
            for (int f=0; f<model.nfaces(); f++) {      // iterate through all facets
                Triangle clip = { shader.vertex(f, 0),  // assemble the primitive
                    shader.vertex(f, 1),
                    shader.vertex(f, 2) };
                rasterize(clip, shader, trash);         // rasterize the primitive
            }
        }
//      trash.write_tga_file(std::string("shadowmap") + std::to_string(i) + std::string(".tga"));

        mat<4,4> N = Viewport * Perspective * ModelView;

        // post-processing
#pragma omp parallel for
        for (int x=0; x<width; x++) {
            for (int y=0; y<height; y++) {
                vec4 fragment = M * vec4{x, y, zbuffer_copy[x+y*width], 1.};
                vec4 q = N * fragment;
                vec3 p = q.xyz()/q.w;
                double lit =  (fragment.z<-100 ||                                     // it's the background or
                               (p.x>=0 && p.x<shadoww && p.y>=0 && p.y<shadowh &&     // it is not out of bounds of the shadow buffer
                               (p.z > zbuffer[int(p.x) + int(p.y)*shadoww] - .03)));  // it is visible
                mask[x+y*width] += (lit - mask[x+y*width])/(i+1.);
            }
        }

    }

#pragma omp parallel for
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            double m = smoothstep(-1, 1, mask[x+y*width]);
            TGAColor c = framebuffer.get(x, y);
            framebuffer.set(x, y, { c[0]*m, c[1]*m, c[2]*m, c[3] });
        }
    }
    framebuffer.write_tga_file("shadow.tga");

    // post-processing 2: edge detection
    constexpr double threshold = .15;
    for (int y = 1; y < framebuffer.height() - 1; ++y) {
        for (int x = 1; x < framebuffer.width() - 1; ++x) {
            vec2 sum;
            for (int j = -1; j <= 1; ++j) {
                for (int i = -1; i <= 1; ++i) {
                    constexpr int Gx[3][3] = { {-1,  0,  1}, {-2, 0, 2}, {-1, 0, 1} };
                    constexpr int Gy[3][3] = { {-1, -2, -1}, { 0, 0, 0}, { 1, 2, 1} };
                    sum = sum + vec2{
                        Gx[j + 1][i + 1] * zbuffer_copy[x+i + (y+j)*width],
                        Gy[j + 1][i + 1] * zbuffer_copy[x+i + (y+j)*width]
                    };
                }
            }
            if (norm(sum)>threshold)
                framebuffer.set(x, y, TGAColor{0, 0, 0, 255});
        }
    }
    framebuffer.write_tga_file("edges.tga");

    return 0;
}

