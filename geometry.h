#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>

template <class t> struct Vec2 {
    t raw[2];
    t& x;
    t& y;
    Vec2<t>() : raw(), x(raw[0]), y(raw[1]) { x = y = t(); }
    Vec2<t>(t _x, t _y) : raw(), x(raw[0]), y(raw[1]) { x=_x; y=_y; }
    Vec2<t>(const Vec2<t> &v) : raw(), x(raw[0]), y(raw[1]) { *this = v; }
    Vec2<t> & operator =(const Vec2<t> &v) {
        if (this != &v) {
            x = v.x;
            y = v.y;
        }
        return *this;
    }
    Vec2<t> operator +(const Vec2<t> &V) const { return Vec2<t>(x+V.x, y+V.y); }
    Vec2<t> operator -(const Vec2<t> &V) const { return Vec2<t>(x-V.x, y-V.y); }
    Vec2<t> operator *(float f)          const { return Vec2<t>(x*f, y*f); }
    t& operator[](const int i) {return raw[i];}
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

template <class t> struct Vec3 {
    t raw[3];
    t& x;
    t& y;
    t& z;
    Vec3<t>() : raw(), x(raw[0]), y(raw[1]), z(raw[2]) { x = y = z = t(); }
    Vec3<t>(t _x, t _y, t _z) : raw(), x(raw[0]), y(raw[1]), z(raw[2]) { x=_x; y=_y; z=_z; }
    Vec3<t>(const Vec3<t> &v) : raw(), x(raw[0]), y(raw[1]), z(raw[2]) { *this = v; }
    Vec3<t> & operator =(const Vec3<t> &v) {
        if (this != &v) {
            x = v.x;
            y = v.y;
            z = v.z;
        }
        return *this;
    }
    Vec3<t> operator ^(const Vec3<t> &v) const { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
    Vec3<t> operator +(const Vec3<t> &v) const { return Vec3<t>(x+v.x, y+v.y, z+v.z); }
    Vec3<t> operator -(const Vec3<t> &v) const { return Vec3<t>(x-v.x, y-v.y, z-v.z); }
    Vec3<t> operator *(float f)          const { return Vec3<t>(x*f, y*f, z*f); }
    t       operator *(const Vec3<t> &v) const { return x*v.x + y*v.y + z*v.z; }
    float norm () const { return std::sqrt(x*x+y*y+z*z); }
    Vec3<t> & normalize(t l=1) { *this = (*this)*(l/norm()); return *this; }
    t& operator[](const int i) {return raw[i];}
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};

typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;

template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
    s << "(" << v.x << ", " << v.y << ")\n";
    return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
    return s;
}

#endif //__GEOMETRY_H__
