#include <vector>
#include "geometry.h"

class Model {
    std::vector<vec3> verts = {};    // array of vertices
    std::vector<vec3> norms = {};    // array of normal vectors
    std::vector<int> facet_vrt = {}; // per-triangle index in the above arrays
    std::vector<int> facet_nrm = {};
public:
    Model(const std::string filename);
    int nverts() const; // number of vertices
    int nfaces() const; // number of triangles
    vec3 vert(const int i) const;                          // 0 <= i < nverts()
    vec3 vert(const int iface, const int nthvert) const;   // 0 <= iface <= nfaces(), 0 <= nthvert < 3
    vec3 normal(const int iface, const int nthvert) const; // normal coming from the "vn x y z" entries in the .obj file
};

