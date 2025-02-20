#include "geometry.h"
#include "tgaimage.h"

class Model {
    std::vector<vec3> verts = {};     // array of vertices
    std::vector<vec3> norms = {};     // per-vertex array of normal vectors
    std::vector<vec2> tex_coord = {}; // per-vertex array of tex coords
    std::vector<int> facet_vrt = {};  // )
    std::vector<int> facet_nrm = {};  //  > per-triangle indices in the above arrays
    std::vector<int> facet_tex = {};  // )
    TGAImage diffusemap  = {};        // diffuse color texture
    TGAImage normalmap   = {};        // normal map texture
    TGAImage specularmap = {};        // specular map texture
public:
    Model(const std::string filename);
    int nverts() const;
    int nfaces() const;
    vec3 normal(const int iface, const int nthvert) const; // per-triangle corner normal vertex
    vec3 normal(const vec2 &uv) const;                     // normal vector from the normal map texture
    vec3 vert(const int i) const;
    vec3 vert(const int iface, const int nthvert) const;
    vec2 uv(const int iface, const int nthvert) const;
    const TGAImage& diffuse() const;
    const TGAImage& specular() const;
};

