#include <limits>
#include <algorithm>
#include <random>
#include "graphics.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective, Viewport; // "OpenGL" state matrices

struct FlatShader : IShader {
    const Model &model;

    FlatShader(const vec3 l, const Model &m) : model(m) {
    }

    virtual vec4 vertex(const int face, const int vert) {
        vec3 v = model.vert(face, vert);                          // current vertex in object coordinates
        vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1.};
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        TGAColor gl_FragColor = {2, 58, 240, 255};
        return {false, gl_FragColor}; // do not discard the pixel
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width  = 800;      // output image size
    constexpr int height = 800;
    constexpr vec3 light_dir{1,1,1}; // light source
    constexpr vec3    eye{-1,0,2};   // camera position
    constexpr vec3 center{0,0,0};    // camera direction
    constexpr vec3     up{0,1,0};    // camera up vector

    lookat(eye, center, up);                              // build the ModelView   matrix
    perspective(norm(eye-center));                        // build the Perspective matrix
    viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport    matrix

    TGAImage framebuffer(width, height, TGAImage::RGB);
    for (int y = 0; y < framebuffer.height(); ++y)
        for (int x = 0; x < framebuffer.width(); ++x)
            framebuffer.set(x, y, {177, 195, 209, 255});

    std::vector<double> zbuffer(width*height, -1000);

    for (int m=1; m<argc; m++) { // iterate through all input objects
        Model model(argv[m]);
        for (int f=0; f<model.nfaces(); f++) { // iterate through all triangles
            FlatShader shader(light_dir, model);
            vec4 clip[3];
            for (int v : {0,1,2}) {            // assemble the primitive
                clip[v] = shader.vertex(f, v);
            }
            rasterize(clip, shader, zbuffer, framebuffer); // rasterize the primitive
        }
    }

    constexpr double ao_radius = .1; // ssao ball radius in normalized device coordinates
    constexpr int nsamples = 256;    // number of samples in the ball
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(-ao_radius, ao_radius);
    auto smoothstep = [](double edge0, double edge1, double x) {         // smoothstep returns 0 if the input is less than the left edge,
            double t = std::clamp((x - edge0)/(edge1 - edge0), 0., 1.);  // 1 if the input is greater than the right edge,
            return t*t*(3 - 2*t);                                        // Hermite interpolation inbetween. The derivative of the smoothstep function is zero at both edges.
    };

    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            vec4 fragment = Viewport.invert() * vec4{x, y, zbuffer[x+y*width], 1};  // for each fragment in the framebuffer

            double occlusion = 0;
            for(int i=0; i<nsamples; i++) {                                         // compute a very rough approximation of the solid angle
                vec4 p = Viewport * (fragment + vec4{dist(gen), dist(gen), dist(gen), 0.} );
                if (p.x<0 || p.x>=width || p.y<0 || p.y>=height) continue;
                occlusion += zbuffer[int(p.x) + int(p.y)*width] - ao_radius/2 > p.z;
            }
            double ssao = smoothstep(.1, .75, 1 - occlusion / nsamples);

            TGAColor c = framebuffer.get(x, y);
            framebuffer.set(x, y, { c[0]*ssao, c[1]*ssao, c[2]*ssao, c[3] });
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

