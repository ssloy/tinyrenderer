#include <vector>
#include <cstdlib>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model *model        = NULL;
float *shadowbuffer = NULL;

const int width  = 800;
const int height = 800;
int occlusion[1024*1024];

Vec3f light_dir(1,1,1);
Vec3f       eye(1,1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);


struct ZShader : public IShader {
    mat<4,3,float> varying_tri;

    virtual Vec4f vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = Projection*ModelView*embed<4>(model->vert(iface, nthvert));
        varying_tri.set_col(nthvert, gl_Vertex);
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor &color) {
        color = TGAColor(255, 255, 255)*((gl_FragCoord.z+1.f)/2.f);
        return false;
    }
};

struct Shader : public IShader {
    mat<2,3,float> varying_uv;
    mat<4,3,float> varying_tri;

    virtual Vec4f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = Projection*ModelView*embed<4>(model->vert(iface, nthvert));
        varying_tri.set_col(nthvert, gl_Vertex);
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor &color) {
        Vec2f uv = varying_uv*bar;
        if (std::abs(shadowbuffer[int(gl_FragCoord.x+gl_FragCoord.y*width)]-gl_FragCoord.z<1e-2)) {
            occlusion[int(uv.x*1024) + int(uv.y*1024)*1024]++;
        }

        color = TGAColor(255, 0, 0);
        return false;
    }
};

int main(int argc, char** argv) {
    if (2>argc) {
        std::cerr << "Usage: " << argv[0] << "obj/model.obj" << std::endl;
        return 1;
    }
    float *zbuffer = new float[width*height];
    for (int i=1024*1024; i--; occlusion[i] = 0);
    model = new Model(argv[1]);

    const int nrenders = 1;
    for (int iter=0; iter<nrenders; iter++) {
        std::cerr << iter << " from " << nrenders << std::endl; 
        float u = (float)rand()/(float)RAND_MAX;
        float v = (float)rand()/(float)RAND_MAX;
        float theta = 2.f*M_PI*u;
        float phi   = acos(2.f*v - 1.f);
        float r = 5;
        eye.x = r*sin(phi)*cos(theta);
        eye.y = r*sin(phi)*sin(theta);
        eye.z = r*cos(phi);
        eye.y = std::abs(eye.y); // uper hemisphere only
        std::cout <<"v " << eye << std::endl;
        for (int i=0; i<3; i++) up[i] = (float)rand()/(float)RAND_MAX;

        shadowbuffer   = new float[width*height];
        for (int i=width*height; i--; shadowbuffer[i] = zbuffer[i] = -std::numeric_limits<float>::max());

        TGAImage frame(width, height, TGAImage::RGB);
        lookat(eye, center, up);
        viewport(width/8, height/8, width*3/4, height*3/4);
        projection(-1.f/(eye-center).norm());
        light_dir = proj<3>((Projection*ModelView*embed<4>(light_dir, 0.f))).normalize();

        ZShader zshader;
        for (int i=0; i<model->nfaces(); i++) {
            for (int j=0; j<3; j++) {
                zshader.vertex(i, j);
            }
            triangle(zshader.varying_tri, zshader, frame, shadowbuffer);
        }
        Shader shader;
        for (int i=0; i<model->nfaces(); i++) {
            for (int j=0; j<3; j++) {
                shader.vertex(i, j);
            }
            triangle(shader.varying_tri, shader, frame, zbuffer);
        }
        frame.flip_vertically(); // to place the origin in the bottom left corner of the image
        frame.write_tga_file("framebuffer.tga");
    }
    TGAImage ocim(1024, 1024, TGAImage::GRAYSCALE);
    for (int i=0; i<1024; i++) {
        for (int j=0; j<1024; j++) {
            ocim.set(i, j, 255.*occlusion[i+j*1024]/float(nrenders));
        }
    }
    ocim.flip_vertically();
    ocim.write_tga_file("occlusion.tga");

    delete [] zbuffer;
    delete model;
    delete [] shadowbuffer;
    return 0;
}

