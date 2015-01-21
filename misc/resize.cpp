#include <iostream>
#include <limits>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

template <class t> struct Vec3 {
    union {
        struct {t x, y, z;};
        struct { t ivert, iuv, inorm; };
        t raw[3];
    };
    Vec3() : x(0), y(0), z(0) {}
    Vec3(t _x, t _y, t _z) : x(_x),y(_y),z(_z) {}
    inline Vec3<t> operator ^(const Vec3<t> &v) const { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
    inline Vec3<t> operator +(const Vec3<t> &v) const { return Vec3<t>(x+v.x, y+v.y, z+v.z); }
    inline Vec3<t> operator -(const Vec3<t> &v) const { return Vec3<t>(x-v.x, y-v.y, z-v.z); }
    inline Vec3<t> operator *(float f)          const { return Vec3<t>(x*f, y*f, z*f); }
    inline t       operator *(const Vec3<t> &v) const { return x*v.x + y*v.y + z*v.z; }
    float norm () const { return sqrtf(x*x+y*y+z*z); }
    Vec3<t> & normalize(t l=1) { *this = (*this)*(l/norm()); return *this; }
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};

typedef Vec3<float> Vec3f;

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
    s << "v " << v.x << " " << v.y << " " << v.z << std::endl;
    return s;
}

int main(int argc, char** argv) {
    if (2!=argc) {
        return 1;
    }

    std::vector<Vec3f> verts;

    std::ifstream in;
    in.open (argv[1], std::ifstream::in);
    if (in.fail()) return 1;
    std::string line;
    while(!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.find("v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts.push_back(v);
        }
    }
    Vec3f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec3f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i=0; i<(int)verts.size(); i++) {
        Vec3f v = verts[i];
        for (int j=0; j<3; j++) {
            bboxmin.raw[j] = std::min(bboxmin.raw[j], v.raw[j]);
            bboxmax.raw[j] = std::max(bboxmax.raw[j], v.raw[j]);
        }
    }
    float maxside = -std::numeric_limits<float>::max();
    for (int j=0; j<3; j++) maxside = std::max(maxside, bboxmax.raw[j]-bboxmin.raw[j]);
    Vec3f center = (bboxmax+bboxmin)*.5f;
    for (int i=0; i<(int)verts.size(); i++) {
        verts[i] = (verts[i]-center)*(2.f/maxside);
        std::cout << verts[i];
    }
}


