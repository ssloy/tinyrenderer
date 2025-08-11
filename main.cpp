#include "graphics.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective; // "OpenGL" state matrices

struct FlatShader : IShader {
    const Model &model;
    vec3 uniform_l; // light direction in clip coordinates
    vec3 tri_eye[3];

    FlatShader(const vec3 l, const Model &m) : model(m) {
        uniform_l = normalized((ModelView*vec4{l.x, l.y, l.z, 0.}).xyz()); // transform the light vector to view coordinates
    }

    virtual vec4 vertex(const int face, const int vert) {
        vec3 v = model.vert(face, vert);                          // current vertex in object coordinates
        vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1.};
        tri_eye[vert] = gl_Position.xyz();                        // in eye coordinates
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        TGAColor gl_FragColor;                                                    // output color of the fragment
        vec3 n = normalized(cross(tri_eye[1]-tri_eye[0], tri_eye[2]-tri_eye[0])); // triangle normal in eye coordinates
        double diff = std::max(0., n * uniform_l);                                // diffuse light intensity
        for (int channel : {0,1,2})
            gl_FragColor[channel] = std::min<int>(30 + 255*diff, 255);            // a bit of ambient light + diffuse light
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
    constexpr vec3  light{ 1, 1, 1}; // light source
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
        FlatShader shader(light, model);
        for (int f=0; f<model.nfaces(); f++) {    // iterate through all triangles
            vec4 clip[3] = { shader.vertex(f, 0), // assemble the primitive
                             shader.vertex(f, 1),
                             shader.vertex(f, 2) };
            rasterize(clip, shader, zbuffer, framebuffer); // rasterize the primitive
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

