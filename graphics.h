#include "tgaimage.h"
#include "geometry.h"

void lookat(const vec3 eye, const vec3 center, const vec3 up);
void perspective(const double f);
void viewport(const int x, const int y, const int w, const int h);

struct IShader {
    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const = 0;
};

void rasterize(const vec4 clip[3], const IShader &shader, std::vector<double> &zbuffer, TGAImage &framebuffer);

