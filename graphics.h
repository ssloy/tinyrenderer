#include "tgaimage.h"
#include "geometry.h"

void lookat(const vec3 eye, const vec3 center, const vec3 up);
void perspective(const double f);
void viewport(const int x, const int y, const int w, const int h);

struct IShader {
    static TGAColor sample2D(const TGAImage &img, const vec2 &uvf) {
        return img.get(uvf[0] * img.width(), uvf[1] * img.height());
    }
    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const = 0;
};

void rasterize(const vec4 clip[3], const IShader &shader, std::vector<double> &zbuffer, TGAImage &framebuffer);

