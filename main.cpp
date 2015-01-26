#include <vector>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"
using namespace std;

const int width  = 800;
const int height = 800;
Model *model = NULL;
TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
Vec3f light_dir = (Vec3f){1,1,1}.normalize();
//Vec3f eye(1,1,3);
//Vec3f center(0,0,0);

struct Shader : public IShader {
    virtual ~Shader() {}
    Vec2i varying_uv[3];
    float varying_inty[3];

    virtual Vec3f vertex(int iface, int nthvert) {
        varying_inty[nthvert] = model->normal(iface, nthvert)*light_dir;
        varying_uv[nthvert]   = model->uv(iface, nthvert);
        Vec3f gl_Vertex = model->vert(iface, nthvert);
        Vec3f gl_Position = proj<3>(Viewport*dive<4>(gl_Vertex));//Projection*ModelView*Matrix(gl_Vertex);
        return gl_Position;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec2f tuv=tuv.fill(0);
        for(size_t i=3;i--;)
        {
            tuv=tuv+(Vec2f){static_cast<float>(varying_uv[i][0]),static_cast<float>(varying_uv[i][1])}*bar[i];
        }
        Vec2i uv=(Vec2i){static_cast<int>(tuv[0]),static_cast<int>(tuv[1])};


//        float inty = varying_inty[0]*bar.x + varying_inty[1]*bar.y + varying_inty[2]*bar.z;
 //       inty = std::max(0.f, std::min(1.f, inty));
//        color = model->diffuse(uv)*inty;
        float inty = model->normal(uv)*light_dir;
        color = model->diffuse(uv)*inty;
        return false;
    }
};

template<typename V>class allPointsOfSquare
{
    vec<V::DimN,typename V::NumberT> topLeft;
    vec<V::DimN,typename V::NumberT> bottomRight;
    vec<V::DimN,typename V::NumberT> pos;
public:
    allPointsOfSquare(const V& topLeft,const V& bottomRight):topLeft(topLeft),bottomRight(bottomRight),pos(topLeft)
    {
    }
    const vec<V::DimN,typename V::NumberT>& operator *() const
    {
        return(pos);
    }
    bool next()
    {
        bool ret=pos!=bottomRight;
        for(size_t i=V::DimN;i--;)
        {
            pos[i]++;
            if(pos[i]>bottomRight[i])
            {
                pos[i]=topLeft[i];
            }
            else
            {
                break;
            }
        }
        return(ret);
    }
};



void fillTria(mat<3,3,float > coord,IShader& shader,TGAImage& image,TGAImage& zbuffer)
{
    //находим углы прямоугольника, в котором лежит треугольник.
    //это соответственно минимумы и максимумы сторон, его содержащих

    Vec3f topLeft=coord.minimums();
    Vec3f bottomRight=coord.maximums();

    coord.setCol(1,2);

    mat<3,3,float > bcm=coord.invertT();

    mat<3,2,float > directions;

    for(size_t i=3;i--;)
    {
        directions[i]=proj<2>(coord[ (i+1) % 3 ])-proj<2>(coord[i]);
    }

    Vec2f sweep;

    for(sweep[0]=floor(topLeft[0]);sweep[0]<=ceil(bottomRight[0]);sweep[0]++)
    for(sweep[1]=floor(topLeft[1]);sweep[1]<=ceil(bottomRight[1]);sweep[1]++)
    {
            size_t i=0;
            for(;i<3;i++)
            {
                Vec2f curDirection=(sweep)-proj<2>(coord[i]);

                tempMat crss;
                crss[0]=proj<2>(curDirection);
                crss[1]=proj<2>(directions[i]);

                if(crss.det()>0.6)
                {
                    break;
                }
            }
            if(i==3)
            {
                Vec3f c=bcm*dive<3>(sweep);
                TGAColor color=TGAColor(255,255,255);
                bool discard = shader.fragment(c, color);
                const int    Z=max(0.0f,min(255.0f,coord.col(2)*c+0.5f  ));

                if ((!discard) && (Z>zbuffer.get((sweep)[0], (sweep)[1])[0]))
                {
                    zbuffer.set((sweep)[0], (sweep)[1], TGAColor(Z));
                    image.set((sweep)[0], (sweep)[1], color);
                    std::cerr<<"+";
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

//    lookat(eye, center, Vec3f(0,1,0));
    viewport(width/8, height/8, width*3/4, height*3/4);
//    projection(-1.f/(eye-center).norm());

    TGAImage image(width, height, TGAImage::RGB);
    Shader shader;
    for (int i=0; i<model->nfaces(); i++) {
        mat<3,3,float> screen_coords;
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
       // triangle(screen_coords, shader, image, zbuffer);
       fillTria(screen_coords, shader, image, zbuffer);
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");

    zbuffer.flip_vertically();
    zbuffer.write_tga_file("zbuffer.tga");
    delete model;
    return 0;
}

