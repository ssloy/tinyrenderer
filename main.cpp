#include <cstdlib>
#include "graphics.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective; // "OpenGL" state matrices

struct RandomShader : IShader {
    const Model &model;
    TGAColor uniform_color = {};
    vec3 tri_eye[3];

    RandomShader(const Model &m) : model(m) {
    }

    virtual vec4 vertex(const int face, const int vert) {
        vec3 v = model.vert(face, vert);                          // current vertex in object coordinates
        vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1.};
        tri_eye[vert] = gl_Position.xyz();                        // in eye coordinates
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        TGAColor gl_FragColor = uniform_color;                                    // output color of the fragment
        return {false, gl_FragColor};                                             // do not discard the pixel
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width  = 800;      // output image size
    constexpr int height = 800;
    constexpr vec3    eye{-1, 0, 2}; // camera position
    constexpr vec3 center{ 0, 0, 0}; // camera direction
    constexpr vec3     up{ 0, 1, 0}; // camera up vector

    lookat(eye, center, up);                              // build the ModelView   matrix
    perspective(norm(eye-center));                        // build the Perspective matrix
    viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport    matrix

    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});
    std::vector<double> zbuffer(width*height, -1000.);

    for (int m=1; m<argc; m++) {                  // iterate through all input objects
        Model model(argv[m]);
        RandomShader shader(model);
        for (int f=0; f<model.nfaces(); f++) {    // iterate through all triangles
            shader.uniform_color = { std::rand()%255, std::rand()%255, std::rand()%255, 255 };
            vec4 clip[3] = { shader.vertex(f, 0), // assemble the primitive
                             shader.vertex(f, 1),
                             shader.vertex(f, 2) };
            rasterize(clip, shader, zbuffer, framebuffer); // rasterize the primitive
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

