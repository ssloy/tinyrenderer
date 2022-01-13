#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model {
    std::vector<vec3> verts{};     // array of vertices
    std::vector<vec2> tex_coord{}; // per-vertex array of tex coords
    std::vector<vec3> norms{};     // per-vertex array of normal vectors
    std::vector<int> facet_vrt{};
    std::vector<int> facet_tex{};  // per-triangle indices in the above arrays
    std::vector<int> facet_nrm{};
    TGAImage diffusemap{};         // diffuse color texture
    TGAImage normalmap{};          // normal map texture
    TGAImage specularmap{};        // specular map texture
    void load_texture(const std::string filename, const std::string suffix, TGAImage &img);
public:
    Model(const std::string filename);
    int nverts() const;
    int nfaces() const;
    vec3 normal(const int iface, const int nthvert) const; // per triangle corner normal vertex
    vec3 normal(const vec2 &uv) const;                     // fetch the normal vector from the normal map texture
    vec3 vert(const int i) const;
    vec3 vert(const int iface, const int nthvert) const;
    vec2 uv(const int iface, const int nthvert) const;
    const TGAImage& diffuse()  const { return diffusemap;  }
    const TGAImage& specular() const { return specularmap; }
};

