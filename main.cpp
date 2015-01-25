#include <vector>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

const int width  = 800;
const int height = 800;
Model *model = NULL;
TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
Vec3f light_dir = Vec3f(1,1,1).normalize();
Vec3f eye(1,1,3);
Vec3f center(0,0,0);


struct Shader : public IShader {
    virtual ~Shader() {}
    Vec2i varying_uv[3];
    float varying_inty[3];

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec2i uv   = varying_uv[0]*bar.x + varying_uv[1]*bar.y + varying_uv[2]*bar.z;
//        float inty = varying_inty[0]*bar.x + varying_inty[1]*bar.y + varying_inty[2]*bar.z;
 //       inty = std::max(0.f, std::min(1.f, inty));
//        color = model->diffuse(uv)*inty;
        float inty = model->norm(uv)*light_dir;
        color = model->diffuse(uv)*inty;
        return false;
    }
};


int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    lookat(eye, center, Vec3f(0,1,0));
    viewport(width/8, height/8, width*3/4, height*3/4);
    projection(-1.f/(eye-center).norm());

    TGAImage image(width, height, TGAImage::RGB);
    Shader shader;
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec3i screen_coords[3];
        Vec3f world_coords[3];
        for (int j=0; j<3; j++) {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] =  Vec3f(Viewport*Projection*ModelView*Matrix(v));
            world_coords[j]  = v;
            shader.varying_inty[j] = model->norm(i, j)*light_dir;
            shader.varying_uv[j]   = model->uv(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");

    zbuffer.flip_vertically();
    zbuffer.write_tga_file("zbuffer.tga");
    delete model;
    return 0;
}

