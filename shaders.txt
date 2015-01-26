struct FlatShader : public IShader {
    mat<3,3,float> varying_tri;

    virtual ~FlatShader() {}

    virtual Vec3i vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        gl_Vertex = Projection*ModelView*gl_Vertex;
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));
        gl_Vertex = Viewport*gl_Vertex;
        return proj<3>(gl_Vertex/gl_Vertex[3]);
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec3f n = cross(varying_tri.col(1)-varying_tri.col(0),varying_tri.col(2)-varying_tri.col(0)).normalize();
        float intensity = CLAMP(n*light_dir, 0.f, 1.f);
        color = TGAColor(255, 255, 255)*intensity;
        return false;
    }
};

struct GouraudShader : public IShader {
    mat<3,3,float> varying_tri;
    Vec3f          varying_ity;

    virtual ~GouraudShader() {}

    virtual Vec3i vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        gl_Vertex = Projection*ModelView*gl_Vertex;
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));

        varying_ity[nthvert] = CLAMP(model->normal(iface, nthvert)*light_dir, 0.f, 1.f);

        gl_Vertex = Viewport*gl_Vertex;
        return proj<3>(gl_Vertex/gl_Vertex[3]);
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_ity*bar;
        color = TGAColor(255, 255, 255)*intensity;
        return false;
    }
};


struct ToonShader : public IShader {
    mat<3,3,float> varying_tri;
    Vec3f          varying_ity;

    virtual ~ToonShader() {}

    virtual Vec3i vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        gl_Vertex = Projection*ModelView*gl_Vertex;
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));

        varying_ity[nthvert] = CLAMP(model->normal(iface, nthvert)*light_dir, 0.f, 1.f);

        gl_Vertex = Viewport*gl_Vertex;
        return proj<3>(gl_Vertex/gl_Vertex[3]);
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_ity*bar;
        if (intensity>.85) intensity = 1;
        else if (intensity>.60) intensity = .80;
        else if (intensity>.45) intensity = .60;
        else if (intensity>.30) intensity = .45;
        else if (intensity>.15) intensity = .30;
        color = TGAColor(255, 155, 0)*intensity;
        return false;
    }
};
