#include <fstream>
#include <sstream>
#include "model.h"
#include <algorithm>

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
            vec3 v;
            for (int i : {0,1,2}) iss >> v[i];
            verts.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            int f,t,n, cnt = 0;
            iss >> trash;
            while (iss >> f >> trash >> t >> trash >> n) {
                facet_vrt.push_back(--f);
                cnt++;
            }
            if (3!=cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            }
        }
    }
    std::cerr << "# v# " << nverts() << " f# "  << nfaces() << std::endl;

    std::vector<int> idx(nfaces());    // permutation, a map from new to old facet indices
    for (int i = 0 ; i<nfaces() ; i++) // we start with the identity
        idx[i] = i;

    std::sort(idx.begin(), idx.end(),
              [&](const int& a, const int& b) { // given two triangles, compare their min z coordinate
                  float aminz = std::min(vert(a, 0).z,
                                         std::min(vert(a, 1).z,
                                                  vert(a, 2).z));
                  float bminz = std::min(vert(b, 0).z,
                                         std::min(vert(b, 1).z,
                                                  vert(b, 2).z));
                  return aminz < bminz;
              });

    std::vector<int> facet_vrt2(nfaces()*3); // allocate an array to store permutated facets
    for (int i=0; i<nfaces(); i++)           // for each (new) facet
        for (int j=0; j<3; j++)              // copy its three vertices from the old array
            facet_vrt2[i*3+j] = facet_vrt[idx[i]*3+j];

    facet_vrt = facet_vrt2;                  // store the sorted triangles
}

int Model::nverts() const { return verts.size(); }
int Model::nfaces() const { return facet_vrt.size()/3; }

vec3 Model::vert(const int i) const {
    return verts[i];
}

vec3 Model::vert(const int iface, const int nthvert) const {
    return verts[facet_vrt[iface*3+nthvert]];
}

