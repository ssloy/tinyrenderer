#include "our_gl.h"
#include "model.h"

extern mat<4,4> Viewport, ModelView, Perspective; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;     // the depth buffer

struct BlankShader : IShader {
    const Model &model;

    BlankShader(const Model &m) : model(m) {}

    virtual vec4 vertex(const int face, const int vert) {
        vec4 gl_Position = ModelView * model.vert(face, vert);
        return Perspective * gl_Position;
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        return {false, {255, 255, 255, 255}};
    }
};

struct PhongShader : IShader {
    const Model &model;
    vec4 l;              // light direction in eye coordinates
    vec2  varying_uv[3]; // triangle uv coordinates, written by the vertex shader, read by the fragment shader
    vec4 varying_nrm[3]; // normal per vertex to be interpolated by the fragment shader
    vec4 tri[3];         // triangle in view coordinates

    PhongShader(const vec3 light, const Model &m) : model(m) {
        l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.})); // transform the light vector to view coordinates
    }

    virtual vec4 vertex(const int face, const int vert) {
        varying_uv[vert]  = model.uv(face, vert);
        varying_nrm[vert] = ModelView.invert_transpose() * model.normal(face, vert);
        vec4 gl_Position = ModelView * model.vert(face, vert);
        tri[vert] = gl_Position;
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        mat<2,4> E = { tri[1]-tri[0], tri[2]-tri[0] };
        mat<2,2> U = { varying_uv[1]-varying_uv[0], varying_uv[2]-varying_uv[0] };
        mat<2,4> T = U.invert() * E;
        mat<4,4> D = {normalized(T[0]),  // tangent vector
                      normalized(T[1]),  // bitangent vector
                      normalized(varying_nrm[0]*bar[0] + varying_nrm[1]*bar[1] + varying_nrm[2]*bar[2]), // interpolated normal
                      {0,0,0,1}}; // Darboux frame
        vec2 uv = varying_uv[0] * bar[0] + varying_uv[1] * bar[1] + varying_uv[2] * bar[2];
        vec4 n = normalized(D.transpose() * model.normal(uv));
        vec4 r = normalized(n * (n * l)*2 - l);                   // reflected light direction
        double ambient  = .4;                                     // ambient light intensity
        double diffuse  = 1.*std::max(0., n * l);                 // diffuse light intensity
        double specular = (1.+3.*sample2D(model.specular(), uv)[0]/255.) * std::pow(std::max(r.z, 0.), 35);  // specular intensity, note that the camera lies on the z-axis (in eye coordinates), therefore simple r.z, since (0,0,1)*(r.x, r.y, r.z) = r.z
        TGAColor gl_FragColor = sample2D(model.diffuse(), uv);
//      TGAColor gl_FragColor = {255, 255, 255, 255};
        for (int channel : {0,1,2})
            gl_FragColor[channel] = std::min<int>(255, gl_FragColor[channel]*(ambient + diffuse + specular));
        return {false, gl_FragColor};                             // do not discard the pixel
    }
};

void drop_zbuffer(std::string filename, std::vector<double> &zbuffer, int width, int height) {
    TGAImage zimg(width, height, TGAImage::GRAYSCALE, {0,0,0,0});
    double minz = +1000;
    double maxz = -1000;
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            double z = zbuffer[x+y*width];
            if (z<-100) continue;
            minz = std::min(z, minz);
            maxz = std::max(z, maxz);
        }
    }
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            double z = zbuffer[x+y*width];
            if (z<-100) continue;
            z = (z - minz)/(maxz-minz) * 255;
            zimg.set(x, y, {z, 255, 255, 255});
        }
    }
    zimg.write_tga_file(filename);
}

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

    // usual rendering pass
    lookat(eye, center, up);
    init_perspective(norm(eye-center));
    init_viewport(width/16, height/16, width*7/8, height*7/8);
    init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});
//    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int m=1; m<argc; m++) {                    // iterate through all input objects
        Model model(argv[m]);                       // load the data
        PhongShader shader(light, model);
        for (int f=0; f<model.nfaces(); f++) {      // iterate through all facets
            Triangle clip = { shader.vertex(f, 0),  // assemble the primitive
                              shader.vertex(f, 1),
                              shader.vertex(f, 2) };
            rasterize(clip, shader, framebuffer);   // rasterize the primitive
        }
    }
    framebuffer.write_tga_file("framebuffer.tga");
    drop_zbuffer("zbuffer1.tga", zbuffer, width, height);

    std::vector<bool> mask(width*height, false);
    std::vector<double> zbuffer_copy = zbuffer;
    mat<4,4> M = (Viewport * Perspective * ModelView).invert();

    { // shadow rendering pass
        lookat(light, center, up);
        init_perspective(norm(eye-center));
        init_viewport(shadoww/16, shadowh/16, shadoww*7/8, shadowh*7/8);
        init_zbuffer(shadoww, shadowh);
        TGAImage trash(shadoww, shadowh, TGAImage::RGB, {177, 195, 209, 255});

        for (int m=1; m<argc; m++) {                    // iterate through all input objects
            Model model(argv[m]);                       // load the data
            BlankShader shader{model};
            for (int f=0; f<model.nfaces(); f++) {      // iterate through all facets
                Triangle clip = { shader.vertex(f, 0),  // assemble the primitive
                                  shader.vertex(f, 1),
                                  shader.vertex(f, 2) };
                rasterize(clip, shader, trash);         // rasterize the primitive
            }
        }
        trash.write_tga_file("shadowmap.tga");
    }

    drop_zbuffer("zbuffer2.tga", zbuffer, shadoww, shadowh);

    mat<4,4> N = Viewport * Perspective * ModelView;


    // post-processing
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            vec4 fragment = M * vec4{x, y, zbuffer_copy[x+y*width], 1.};
            vec4 q = N * fragment;
            vec3 p = q.xyz()/q.w;
            bool lit =  (fragment.z<-100 ||                                   // it's the background or
                        (p.x<0 || p.x>=shadoww || p.y<0 || p.y>=shadowh) ||   // it is out of bounds of the shadow buffer
                        (p.z > zbuffer[int(p.x) + int(p.y)*shadoww] - .03));  // it is visible
            mask[x+y*width] = lit;
        }
    }

    TGAImage maskimg(width, height, TGAImage::GRAYSCALE);
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            if (mask[x+y*width]) continue;
            maskimg.set(x, y, {255, 255, 255, 255});
        }
    }
    maskimg.write_tga_file("mask.tga");

    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            if (mask[x+y*width]) continue;
            TGAColor c = framebuffer.get(x, y);
            vec3 a = {c[0], c[1], c[2]};
            if (norm(a)<80) continue;
            a = normalized(a)*80;
            framebuffer.set(x, y, { a[0], a[1], a[2], 255 });
        }
    }
    framebuffer.write_tga_file("shadow.tga");


    return 0;
}

