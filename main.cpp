#include <limits>
#include "graphics.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective; // "OpenGL" state matrices

struct FlatShader : IShader {
    const Model &model;
    vec3 uniform_l; // light direction in clip coordinates
    vec3 tri_eye[3];
    vec3 varying_nrm[3]; // normal per vertex to be interpolated by FS

    FlatShader(const vec3 l, const Model &m) : model(m) {
        uniform_l = normalized((ModelView*vec4{l.x, l.y, l.z, 0.}).xyz()); // transform the light vector to view coordinates
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
        TGAColor gl_FragColor = {2, 58, 240, 255};
        vec3 n = normalized(varying_nrm[0] * bar[0] + varying_nrm[1] * bar[1] + varying_nrm[2] * bar[2]); // per-vertex normal interpolation
        double diff = std::max(0., n * uniform_l); // diffuse light intensity

        double intensity = .15 + diff;   // a bit of ambient light + diffuse light
        if (intensity>.66) intensity = 1;
        else if (intensity>.33) intensity = .66;
        else intensity = .33;

        for (int i : {0,1,2})
            gl_FragColor[i] = std::min<int>(intensity * gl_FragColor[i], 255);
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

    sobel_edge_detection(.15, zbuffer, framebuffer);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

