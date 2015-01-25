#include <vector>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const int width  = 800;
const int height = 800;

Model *model = NULL;

TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

Vec3f light_dir = Vec3f(1,1,1).normalize();
//Vec3f light_dir = Vec3f(1,1,1).normalize();
Vec3f eye(1,1,3);
//Vec3f eye(0,0,300);
Vec3f center(0,0,0);

Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = 255.f/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = 255.f/2.f;
    return m;
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
    return res;
}

Vec3f barycentric(Vec3i A, Vec3i B, Vec3i C, Vec3i P) {
    Vec3f u = Vec3f(C.x-A.x, B.x-A.x, A.x-P.x)^Vec3f(C.y-A.y, B.y-A.y, A.y-P.y);
    return std::abs(u.z)>.5 ? Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z) : Vec3f(-1,1,1); // dont forget that u.z is an integer. If it is zero then triangle ABC is degenerate
}

struct Shader {
    Vec2i varying_uv[3];
    float varying_inty[3];

    bool fragment(Vec3f bar, TGAColor &color) {
        Vec2i uv   = varying_uv[0]*bar.x + varying_uv[1]*bar.y + varying_uv[2]*bar.z;
//        float inty = varying_inty[0]*bar.x + varying_inty[1]*bar.y + varying_inty[2]*bar.z;
 //       inty = std::max(0.f, std::min(1.f, inty));
//        color = model->diffuse(uv)*inty;
        float inty = model->norm(uv)*light_dir;
        color = model->diffuse(uv)*inty;
        return false;
    }
};

void triangle2(Vec3i *pts, Shader &shader, TGAImage &image, TGAImage &zbuffer) {
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

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    Matrix ModelView  = lookat(eye, center, Vec3f(0,1,0));
    Matrix ViewPort   = viewport(width/8, height/8, width*3/4, height*3/4);
    Matrix Projection = Matrix::identity(4);
    Projection[3][2] = -1.f/(eye-center).norm();
    Matrix MVPVP    = ViewPort*Projection*ModelView;

    TGAImage image(width, height, TGAImage::RGB);
    Shader shader;
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec3i screen_coords[3];
        Vec3f world_coords[3];
        for (int j=0; j<3; j++) {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] =  Vec3f(ViewPort*Projection*ModelView*Matrix(v));
            world_coords[j]  = v;
            shader.varying_inty[j] = model->norm(i, j)*light_dir;
            shader.varying_uv[j]   = model->uv(i, j);
        }
//        triangle(screen_coords[0], screen_coords[1], screen_coords[2], intensity[0], intensity[1], intensity[2], image, zbuffer);
        triangle2(screen_coords, shader, image, zbuffer);
    }
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");

    zbuffer.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    zbuffer.write_tga_file("zbuffer.tga");
    delete model;
    return 0;
}

