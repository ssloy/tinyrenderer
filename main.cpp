#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model *model     = NULL;
const int width  = 800;
const int height = 800;

Vec3f light_dir(1,1,1);
Vec3f       eye(1,1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);

struct Shader : public IShader {
    mat<3,3,float> varying_tri;
    mat<2,3,float> varying_uv;

    virtual ~Shader() {}

    virtual Vec3i vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        gl_Vertex = Projection*ModelView*gl_Vertex;
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));

        varying_uv.set_col(nthvert, model->uv(iface, nthvert));

        gl_Vertex = Viewport*gl_Vertex;
        return proj<3>(gl_Vertex/gl_Vertex[3]);
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec2i uv = varying_uv*bar;
        Vec3f n = model->normal(uv);
        Vec3f reflected_light = n*(n*light_dir*2.f) - light_dir;
        float diffuse_ity  = std::max(n*light_dir, 0.f);
        float ambient_ity  = .1f;
        float specular_ity = pow(std::max(reflected_light.z/reflected_light.norm(), 0.0f), model->specular(uv));

        float ity = CLAMP(.1f+n*light_dir, 0.f, 1.f);
        TGAColor diff = model->diffuse(uv)*ity;
        for (int c=0; c<3; c++) color[c] = std::min(5 + diff[c]*(diffuse_ity + .6f*specular_ity), 255.f);

        return false;
    }
};

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    lookat(eye, center, up);
    viewport(width/8, height/8, width*3/4, height*3/4);
    projection(-1.f/(eye-center).norm());
    light_dir.normalize();

    TGAImage image  (width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    Shader shader;
    for (int i=0; i<model->nfaces(); i++) {
        Vec3i screen_coords[3];
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.  flip_vertically(); // to place the origin in the bottom left corner of the image
    zbuffer.flip_vertically();
    image.  write_tga_file("output.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}
