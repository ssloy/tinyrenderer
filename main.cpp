#include <random>
#include <algorithm>
#include <cmath>

#include "our_gl.h"
#include "model.h"

extern mat<4,4> Viewport, ModelView, Perspective; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;     // the depth buffer

struct ToonShader : IShader {
    vec4 color;
    const Model &model;
    vec4 l;              // light direction in eye coordinates
    vec4 varying_nrm[3]; // normal per vertex to be interpolated by the fragment shader

    ToonShader(const vec4 color, const vec3 light, const Model &m) : color(color), model(m) {
        l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.})); // transform the light vector to view coordinates
    }

    virtual vec4 vertex(const int face, const int vert) {
        varying_nrm[vert] = ModelView.invert_transpose() * model.normal(face, vert);
        vec4 gl_Position = ModelView * model.vert(face, vert);
        return Perspective * gl_Position;
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        vec4 n = normalized(varying_nrm[0] * bar[0] + varying_nrm[1] * bar[1] + varying_nrm[2] * bar[2]); // per-vertex normal interpolation

        double diffuse = std::max(0., n * l); // diffuse light intensity

        double intensity = .15 + diffuse;   // a bit of ambient light + diffuse light
        if (intensity>.66) intensity = 1;
        else if (intensity>.33) intensity = .66;
        else intensity = .33;

        TGAColor gl_FragColor;
        for (int channel : {0,1,2})
            gl_FragColor[channel] = std::min<int>(255, color[channel]*intensity);
        return {false, gl_FragColor};                             // do not discard the pixel
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width  = 800;      // output image size
    constexpr int height = 800;
    constexpr vec3  light{ 1, 1, 1}; // light source
    constexpr vec3    eye{-1, 0, 2}; // camera position
    constexpr vec3 center{ 0, 0, 0}; // camera direction
    constexpr vec3     up{ 0, 1, 0}; // camera up vector

    // usual rendering pass
    lookat(eye, center, up);
    init_perspective(norm(eye-center));
    init_viewport(width/16, height/16, width*7/8, height*7/8);
    init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});

    constexpr vec4 colors[] = {{22*4, 56*4, 147*4, 255}, {123, 98, 88, 255}};

    for (int m=1; m<argc; m++) {                    // iterate through all input objects
        Model model(argv[m]);                       // load the data
        ToonShader shader(colors[(m-1)%2], light, model);
        for (int f=0; f<model.nfaces(); f++) {      // iterate through all facets
            Triangle clip = { shader.vertex(f, 0),  // assemble the primitive
                              shader.vertex(f, 1),
                              shader.vertex(f, 2) };
            rasterize(clip, shader, framebuffer);   // rasterize the primitive
        }
    }

    // post-processing: edge detection
    constexpr double threshold = .15;
    for (int y = 1; y < framebuffer.height() - 1; ++y) {
        for (int x = 1; x < framebuffer.width() - 1; ++x) {
            vec2 sum;
            for (int j = -1; j <= 1; ++j) {
                for (int i = -1; i <= 1; ++i) {
                    constexpr int Gx[3][3] = { {-1,  0,  1}, {-2, 0, 2}, {-1, 0, 1} };
                    constexpr int Gy[3][3] = { {-1, -2, -1}, { 0, 0, 0}, { 1, 2, 1} };
                    sum = sum + vec2{
                        Gx[j + 1][i + 1] * zbuffer[x+i + (y+j)*width],
                        Gy[j + 1][i + 1] * zbuffer[x+i + (y+j)*width]
                    };
                }
            }
            if (norm(sum)>threshold)
                framebuffer.set(x, y, TGAColor{0, 0, 0, 255});
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

