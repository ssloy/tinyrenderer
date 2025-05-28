#include <limits>
#include "graphics.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective; // "OpenGL" state matrices

struct RandomShader : IShader {
    const Model &model;
    vec3 uniform_l; // light direction in clip coordinates
    TGAColor varying_color[3];

    RandomShader(const vec3 l, const Model &m) : model(m) {
        uniform_l = normalized((ModelView*vec4{l.x, l.y, l.z, 0.}).xyz()); // transform the light vector to view coordinates
    }

    virtual vec4 vertex(const int face, const int vert) {
        for (int c=0; c<3; c++)
            varying_color[vert][c] = std::rand()%255;
        vec3 v = model.vert(face, vert);
        vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1.};
        return Perspective * gl_Position;
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
       TGAColor gl_FragColor;
       for (int c : {0,1,2})
           for (int v : {0,1,2})
               gl_FragColor[c] += bar[v] * varying_color[v][c];
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
    std::vector<double> zbuffer(width*height, -std::numeric_limits<double>::max());

    for (int m=1; m<argc; m++) { // iterate through all input objects
        Model model(argv[m]);
        for (int f=0; f<model.nfaces(); f++) { // iterate through all triangles
            RandomShader shader(light_dir, model);
            vec4 clip[3];
            for (int v : {0,1,2}) {            // assemble the primitive
                clip[v] = shader.vertex(f, v);
            }
            rasterize(clip, shader, zbuffer, framebuffer); // rasterize the primitive
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

