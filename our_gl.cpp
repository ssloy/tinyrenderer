#include <vector>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = 255.f/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = 255.f/2.f;
    Viewport = m;
    return m;
}

Matrix projection(float coeff) {
    Projection = Matrix::identity(4);
    Projection[3][2] = coeff;
    return Projection;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye-center).normalize();
    Vec3f x = (up^z).normalize();
    Vec3f y = (z^x).normalize();
    Matrix res = Matrix::identity(4);
    for (int i=0; i<3; i++) {
        res[0][i] = x[i];
        res[1][i] = y[i];
        res[2][i] = z[i];
        res[i][3] = -center[i];
    }
    ModelView = res;
    return res;
}

Vec3f barycentric(Vec3i A, Vec3i B, Vec3i C, Vec3i P) {
    Vec3f u = Vec3f(C.x-A.x, B.x-A.x, A.x-P.x)^Vec3f(C.y-A.y, B.y-A.y, A.y-P.y);
    return std::abs(u.z)>.5 ? Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z) : Vec3f(-1,1,1); // dont forget that u.z is an integer. If it is zero then triangle ABC is degenerate
}

void triangle(Vec3i *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer) {
    Vec2i bboxmin( std::numeric_limits<int>::max(),  std::numeric_limits<int>::max());
    Vec2i bboxmax(-std::numeric_limits<int>::max(), -std::numeric_limits<int>::max());
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j]);
        }
    }
    Vec3i P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f c = barycentric(pts[0], pts[1], pts[2], P);
            P.z = std::max(0, std::min(255, int(pts[0].z*c.x + pts[1].z*c.y + pts[2].z*c.z + .5))); // clamping to 0-255 since it is stored in unsigned char
            if (c.x<0 || c.y<0 || c.z<0 || zbuffer.get(P.x, P.y)[0]>P.z) continue;
            TGAColor color;
            bool discard = shader.fragment(c, color);
            if (!discard) {
                zbuffer.set(P.x, P.y, TGAColor(P.z));
                image.set(P.x, P.y, color);
            }
        }
    }
}

