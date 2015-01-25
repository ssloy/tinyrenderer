#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include <vector>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

Matrix viewport(int x, int y, int w, int h);
Matrix projection(float coeff=0.f);
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
    virtual ~IShader() {}
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

void triangle(Vec3i *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer);

#endif //__OUR_GL_H__

