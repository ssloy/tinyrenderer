#include <cmath>
#include <limits>
#include "our_gl.h"

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

void viewport(int x, int y, int w, int h) {
    Viewport = Matrix::identity();
    Viewport[0][3] = x+w/2.f;
    Viewport[1][3] = y+h/2.f;
    Viewport[2][3] = 255.f/2.f;
    Viewport[0][0] = w/2.f;
    Viewport[1][1] = h/2.f;
    Viewport[2][2] = 255.f/2.f;
}

void projection(float coeff) {
    Projection = Matrix::identity();
    Projection[3][2] = coeff;
}

void lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye-center).normalize();
    Vec3f x = cross(up,z).normalize();
    Vec3f y = cross(z,x).normalize();
    ModelView = Matrix::identity();
    for (int i=0; i<3; i++) {
        ModelView[0][i] = x[i];
        ModelView[1][i] = y[i];
        ModelView[2][i] = z[i];
        ModelView[i][3] = -center[i];
    }
}


Vec3f barycentric(Vec3i A, Vec3i B, Vec3i C, Vec3i P) {
    Vec3f u = cross(Vec3f(C[0]-A[0], B[0]-A[0], A[0]-P[0]),Vec3f(C[1]-A[1], B[1]-A[1], A[1]-P[1]));
    return std::abs(u[2])>.5 ? Vec3f(1.f-(u[0]+u[1])/u[2], u[1]/u[2], u[0]/u[2]) : Vec3f(-1,1,1); // dont forget that u[2] is an integer. If it is zero then triangle ABC is degenerate
}

/*

Vec3f barycentric(Vec3i A, Vec3i B, Vec3i C, Vec3i P) {
    Vec3i s[2];
    for (int i=2; i--; ) {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2])>1e-2) {
        u = u*(1./u.z);
        return u;
    }
    return Vec3f(-1,1,1);
}
*/

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
    for (P[0]=bboxmin[0]; P[0]<=bboxmax[0]; P[0]++) {
        for (P[1]=bboxmin[1]; P[1]<=bboxmax[1]; P[1]++) {
            Vec3f c = barycentric(pts[0], pts[1], pts[2], P);
            P[2] = std::max(0, std::min(255, int(pts[0][2]*c[0] + pts[1][2]*c[1] + pts[2][2]*c[2] + .5))); // clamping to 0-255 since it is stored in unsigned char
            if (c[0]<0 || c[1]<0 || c[2]<0 || zbuffer.get(P[0], P[1])[0]>P[2]) continue;
            TGAColor color;
            bool discard = shader.fragment(c, color);
            if (!discard) {
                zbuffer.set(P[0], P[1], TGAColor(P[2]));
                image.set(P[0], P[1], color);
            }
        }
    }
}

