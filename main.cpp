#include <limits>
#include <algorithm>
#include <random>
#include "graphics.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective, Viewport; // "OpenGL" state matrices

struct FlatShader : IShader {
    const Model &model;
    vec3 tri_eye[3];
    vec3 varying_nrm[3]; // normal per vertex to be interpolated by FS

    FlatShader(const Model &m) : model(m) {
    }

    virtual vec4 vertex(const int face, const int vert) {
        vec3 v = model.vert(face, vert);                          // current vertex in object coordinates
        vec3 n = model.normal(face, vert);
        varying_nrm[vert] = (ModelView.invert_transpose() * vec4{n.x, n.y, n.z, 0.}).xyz();
        vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1.};
        tri_eye[vert] = gl_Position.xyz();                        // in eye coordinates
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        TGAColor gl_FragColor = {255, 255, 255, 255};
        return {false, gl_FragColor}; // do not discard the pixel
    }
};

void sobel_edge_detection(const double threshold, const std::vector<double> &zbuffer, TGAImage &framebuffer) {
    const int Gx[3][3] = { {-1,  0,  1}, {-2, 0, 2}, {-1, 0, 1} };
    const int Gy[3][3] = { {-1, -2, -1}, { 0, 0, 0}, { 1, 2, 1} };

    for (int y = 1; y < framebuffer.height() - 1; ++y) {
        for (int x = 1; x < framebuffer.width() - 1; ++x) {
            double sumX = 0, sumY = 0;
            for (int j = -1; j <= 1; ++j) {
                for (int i = -1; i <= 1; ++i) {
                    sumX += Gx[j + 1][i + 1] * zbuffer[x+i + (y+j)*framebuffer.width()];
                    sumY += Gy[j + 1][i + 1] * zbuffer[x+i + (y+j)*framebuffer.width()];
                }
            }
            double norm = std::sqrt(sumX * sumX + sumY * sumY);
            if (norm>threshold)
                framebuffer.set(x, y, TGAColor{0, 0, 0, 255});
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width  = 800;      // output image size
    constexpr int height = 800;
    constexpr vec3   light{1,1,1}; // light source
    constexpr vec3    eye{-1,0,2};   // camera position
    constexpr vec3 center{0,0,0};    // camera direction
    constexpr vec3     up{0,1,0};    // camera up vector

    constexpr int shadowwidth  = 8000;      // output image size
    constexpr int shadowheight = 8000;

    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});
    std::vector<double> zbuffer(width*height, -1000);
    std::vector<double> shadow(shadowwidth*shadowheight, -1000);

    mat<4,4> ShadowMatrix;
    {
        lookat(light, center, up);
        perspective(1000);
    viewport(shadowwidth/6, shadowheight/6, shadowwidth*2/3, shadowheight*2/3); // build the Viewport    matrix

        ShadowMatrix = Viewport * Perspective * ModelView;

        TGAImage trash(shadowwidth, shadowheight, TGAImage::RGB);
        for (int m=1; m<argc; m++) { // iterate through all input objects
            Model model(argv[m]);
            for (int f=0; f<model.nfaces(); f++) { // iterate through all triangles
                FlatShader shader(model);
                vec4 clip[3];
                for (int v : {0,1,2}) {            // assemble the primitive
                    clip[v] = shader.vertex(f, v);
                }
                rasterize(clip, shader, shadow, trash); // rasterize the primitive
            }
        }
        trash.write_tga_file("shadowbuffer.tga");
    }

    lookat(eye, center, up);                              // build the ModelView   matrix
    perspective(norm(eye-center));                        // build the Perspective matrix
    viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport    matrix


    for (int m=1; m<argc; m++) { // iterate through all input objects
        Model model(argv[m]);
        for (int f=0; f<model.nfaces(); f++) { // iterate through all triangles
            FlatShader shader(model);
            vec4 clip[3];
            for (int v : {0,1,2}) {            // assemble the primitive
                clip[v] = shader.vertex(f, v);
            }
            rasterize(clip, shader, zbuffer, framebuffer); // rasterize the primitive
        }
    }

    constexpr double ao_radius = .1;  // ssao ball radius in normalized device coordinates
    constexpr int nsamples = 128;     // number of samples in the ball
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(-ao_radius, ao_radius);
    auto smoothstep = [](double edge0, double edge1, double x) {         // smoothstep returns 0 if the input is less than the left edge,
            double t = std::clamp((x - edge0)/(edge1 - edge0), 0., 1.);  // 1 if the input is greater than the right edge,
            return t*t*(3 - 2*t);                                        // Hermite interpolation inbetween. The derivative of the smoothstep function is zero at both edges.
    };

#pragma omp parallel for
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            vec4 fragment = Viewport.invert() * vec4{x, y, zbuffer[x+y*width], 1.}; // for each fragment in the framebuffer
            double vote   = 0;
            double voters = 0;
            for(int i=0; i<nsamples; i++) {                                         // compute a very rough approximation of the solid angle
                vec4 p = Viewport * (fragment + vec4{dist(gen), dist(gen), dist(gen), 0.});
                if (p.x<0 || p.x>=width || p.y<0 || p.y>=height) continue;
                double d = zbuffer[int(p.x) + int(p.y)*width];
                if (fragment.z + 5*ao_radius < d) continue;                         // range check to remove the dark halo
                voters++;
                vote += d > p.z;
            }
            double ssao = smoothstep(0, 1, 1 - vote/voters*.4);
            TGAColor c = framebuffer.get(x, y);
            framebuffer.set(x, y, { c[0]*ssao, c[1]*ssao, c[2]*ssao, c[3] });
        }
    }

#pragma omp parallel for
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            vec4 fragment = (Viewport * Perspective * ModelView).invert() * vec4{x, y, zbuffer[x+y*width], 1.}; // for each fragment in the framebuffer
            vec4 q = ShadowMatrix * fragment;
            vec3 p = q.xyz()/q.w;
                if (p.x<0 || p.x>=shadowwidth || p.y<0 || p.y>=shadowheight) continue;

            if (p.z < shadow[int(p.x) + int(p.y)*shadowwidth] - .03) { // small bias to avoid z-fighting
                TGAColor c = framebuffer.get(x, y);
                framebuffer.set(x, y, { c[0]/2, c[1]/2, c[2]/2, c[3] });
            }
        }
    }

        sobel_edge_detection(.15, zbuffer, framebuffer);
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

