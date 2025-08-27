#include <fstream>
#include <sstream>
#include "model.h"

Model::Model(const std::string filename) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            vec4 v = {0,0,0,1};
            for (int i : {0,1,2}) iss >> v[i];
            verts.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            vec4 n;
            for (int i : {0,1,2}) iss >> n[i];
            norms.push_back(normalized(n));
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            vec2 uv;
            for (int i : {0,1}) iss >> uv[i];
            tex.push_back({uv.x, 1-uv.y});
        } else if (!line.compare(0, 2, "f ")) {
            int f,t,n, cnt = 0;
            iss >> trash;
            while (iss >> f >> trash >> t >> trash >> n) {
                facet_vrt.push_back(--f);
                facet_tex.push_back(--t);
                facet_nrm.push_back(--n);
                cnt++;
            }
            if (3!=cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            }
        }
    }
    std::cerr << "# v# " << nverts() << " f# "  << nfaces() << std::endl;
    auto load_texture = [&filename](const std::string suffix, TGAImage &img) {
        size_t dot = filename.find_last_of(".");
        if (dot==std::string::npos) return;
        std::string texfile = filename.substr(0,dot) + suffix;
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
    };
    load_texture("_diffuse.tga",    diffusemap );
    load_texture("_nm_tangent.tga", normalmap);
    load_texture("_spec.tga",       specularmap);
}

int Model::nverts() const { return verts.size(); }
int Model::nfaces() const { return facet_vrt.size()/3; }

vec4 Model::vert(const int i) const {
    return verts[i];
}

vec4 Model::vert(const int iface, const int nthvert) const {
    return verts[facet_vrt[iface*3+nthvert]];
}

vec4 Model::normal(const int iface, const int nthvert) const {
    return norms[facet_nrm[iface*3+nthvert]];
}

vec4 Model::normal(const vec2 &uv) const {
    TGAColor c = normalmap.get(uv[0]*normalmap.width(), uv[1]*normalmap.height());
    return normalized(vec4{(double)c[2],(double)c[1],(double)c[0],0}*2./255. - vec4{1,1,1,0});
}

vec2 Model::uv(const int iface, const int nthvert) const {
    return tex[facet_tex[iface*3+nthvert]];
}

const TGAImage& Model::diffuse()  const { return diffusemap;  }
const TGAImage& Model::specular() const { return specularmap; }

