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
    constexpr vec3  light{ 1, 1, 1}; // light source
    constexpr vec3    eye{-1, 0, 2}; // camera position
    constexpr vec3 center{ 0, 0, 0}; // camera direction
    constexpr vec3     up{ 0, 1, 0}; // camera up vector

    std::vector<double> zshadow;
    mat<4,4> ShadowMatrix;
    { // shadow rendering pass
        lookat(light, center, up);
        init_perspective(norm(eye-center));
        init_viewport(shadoww/16, shadowh/16, shadoww*7/8, shadowh*7/8);
        init_zbuffer(shadoww, shadowh);
        TGAImage framebuffer(shadoww, shadowh, TGAImage::RGB, {177, 195, 209, 255});

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
        zshadow = zbuffer;
        ShadowMatrix = Viewport * Perspective * ModelView;
        framebuffer.write_tga_file("shadowmap.tga");
    }

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

    // post-processing
#pragma omp parallel for
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            vec4 fragment = (Viewport * Perspective * ModelView).invert() * vec4{x, y, zbuffer[x+y*width], 1.};
            vec4 q = ShadowMatrix * fragment;
            vec3 p = q.xyz()/q.w;
            if (p.x<0 || p.x>=shadoww || p.y<0 || p.y>=shadowh) continue;
            if (p.z < zshadow[int(p.x) + int(p.y)*shadoww] - .03) { // small bias to avoid z-fighting
                TGAColor c = framebuffer.get(x, y);
                framebuffer.set(x, y, { c[0]/2, c[1]/2, c[2]/2, c[3] });
            }
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
                        Gx[j + 1][i + 1] * zbuffer[x+i + (y+j)*width],
                        Gy[j + 1][i + 1] * zbuffer[x+i + (y+j)*width]
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

