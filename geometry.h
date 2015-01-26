#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>

template<size_t DimCols,size_t DimRows,typename T> class mat;

template <size_t DIM, typename T> struct vec {
    vec() { for (size_t i=DIM; i--; data_[i] = T(0)); }

          T& operator[](const size_t i)       { assert(i<DIM); return data_[i]; }
    const T& operator[](const size_t i) const { assert(i<DIM); return data_[i]; }

    vec<DIM,T> operator+(const vec<DIM,T>& v) {
        vec<DIM, T> ret(*this);
        for (size_t i=DIM; i--; ret[i]+=v[i]);
        return ret;
    }

    vec<DIM,T> operator-(const vec<DIM,T>& v) {
        vec<DIM, T> ret(*this);
        for (size_t i=DIM; i--; ret[i]-=v[i]);
        return ret;
    }

    T operator*(const vec<DIM,T>& v) const {
        T ret = T();
        for (size_t i=DIM; i--; ret += (*this)[i]*v[i]);
        return ret;
    }

    vec<DIM,T> operator *(T u) {
        vec<DIM, T> ret(*this);
        for (size_t i=DIM; i--; ret[i]*=u);
        return ret;
    }

/*
    template <typename U> vec<DIM,T> operator *(U u) {
        vec<DIM, T> ret(*this);
        for (size_t i=DIM; i--; ret[i]*=u);
        return ret;
    }
*/
private:
    T data_[DIM];
};

/////////////////////////////////////////////////////////////////////////////////

template <typename T> struct vec<2,T> {
    vec() : x(0), y(0) {}
    vec(T X, T Y) : x(X), y(Y) {}
    template <class U> vec<2,T>(const vec<2,U> &v);

          T& operator[](const size_t i)       { assert(i<2); return i<=0 ? x : y; }
    const T& operator[](const size_t i) const { assert(i<2); return i<=0 ? x : y; }
    vec<2,T> operator +(const vec<2,T> &v) { return vec<2,T>(x+v.x, y+v.y); }
    vec<2,T> operator -(const vec<2,T> &v) { return vec<2,T>(x-v.x, y-v.y); }
    T        operator *(const vec<2,T> &v) { return x*v.x + y*v.y; }
    vec<2,T> operator *(T u) { return vec<2,T>(x*u, y*u); }
//    template <typename U> vec<2,T> operator *(U u) { return vec<2,T>(x*u, y*u); }

    T x,y;
};

/////////////////////////////////////////////////////////////////////////////////

template <typename T> struct vec<3,T> {
    vec() : x(0), y(0), z(0) {}
    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
    template <class U> vec<3,T>(const vec<3,U> &v);

          T& operator[](const size_t i)       { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
    const T& operator[](const size_t i) const { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
    vec<3,T> operator +(const vec<3,T> &v) { return vec<3,T>(x+v.x, y+v.y, z+v.z); }
    vec<3,T> operator -(const vec<3,T> &v) { return vec<3,T>(x-v.x, y-v.y, z-v.z); }
    T        operator *(const vec<3,T> &v) { return x*v.x + y*v.y + z*v.z; }
    vec<3,T> operator *(T u) { return vec<3,T>(x*u, y*u, z*u); }
//    template <typename U> vec<3,T> operator *(U u) { return vec<3,T>(x*u, y*u, z*u); }

    float norm() { return std::sqrt(x*x+y*y+z*z); }
    vec<3,T> & normalize(T l=1) { *this = (*this)*(l/norm()); return *this; }

    T x,y,z;
};

/////////////////////////////////////////////////////////////////////////////////

template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, vec<DIM,T>& v) {
    for(unsigned int i=0; i<DIM; i++) {
        out << v[i] << " " ;
    }
    return out ;
}

template <class T> std::ostream& operator<<(std::ostream& out, vec<3,T>& v) {
    out << v.x << " " << v.y << " " << v.z;
    return out ;
}

template <class T> std::ostream& operator<<(std::ostream& out, vec<2,T>& v) {
    out << v.x << " " << v.y;
    return out ;
}

template<size_t LEN,size_t DIM,typename T> vec<LEN,T> embed(const vec<DIM,T> &v, T fill=1) { // погружение вектора
    vec<LEN,T> ret;
    for (size_t i=LEN; i--; ret[i]=(i<DIM?v[i]:fill));
    return ret;
}

template<size_t LEN,size_t DIM, typename T> vec<LEN,T> proj(const vec<DIM,T> &v) { // погружение вектора
    vec<LEN,T> ret;
    for (size_t i=LEN; i--; ret[i]=v[i]);
    return ret;
}

template <typename T> vec<3,T> cross(vec<3,T> v1, vec<3,T> v2) {
    return vec<3,T>(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

/////////////////////////////////////////////////////////////////////////////////

template<size_t DIM,typename T> struct dt {
    static T det(const mat<DIM,DIM,T>& src) {
        T ret=0;
        for (size_t i=DIM; i--; ret += src[0][i]*src.algAdd(0,i));
        return ret;
    }
};

template<typename T> struct dt<1,T> {
    static T det(const mat<1,1,T>& src) {
        return src[0][0];
    }
};

/////////////////////////////////////////////////////////////////////////////////

template<size_t DimRows,size_t DimCols,typename T> class mat {
    vec<DimCols,T> rows[DimRows];
public:
    mat() {}

    vec<DimCols,T>& operator[] (const size_t idx) {
        assert(idx<DimRows);
        return rows[idx];
    }

    const vec<DimCols,T>& operator[] (const size_t idx) const {
        assert(idx<DimRows);
        return rows[idx];
    }

    vec<DimRows,T> col(const size_t idx) {
        assert(idx<DimCols);
        vec<DimRows,T> ret;
        for (size_t i=DimRows; i--; ret[i]=rows[i][idx]);
        return ret;
    }

    static mat<DimRows,DimCols,T> identity() {
        mat<DimRows,DimCols,T> ret;
        for (size_t i=DimRows; i--; )
            for (size_t j=DimCols;j--; ret[i][j]=(i==j));
        return ret;
    }

    T det() const {
        return dt<DimCols,T>::det(*this);
    }

    mat<DimRows-1,DimCols-1,T> minor(size_t row, size_t col) const {
        mat<DimRows-1,DimCols-1,T> ret;
        for (size_t i=DimRows-1; i--; )
            for (size_t j=DimCols-1;j--; ret[i][j]=rows[i<row?i:i+1][j<col?j:j+1]);
        return ret;
    }

    T algAdd(size_t row, size_t col) const {
        return minor(row,col).det()*((row+col)%2 ? -1 : 1);
    }

    mat<DimRows,DimCols,T> adjugate() const {
        mat<DimRows,DimCols,T> ret;
        for (size_t i=DimRows; i--; )
            for (size_t j=DimCols; j--; ret[i][j]=algAdd(i,j));
        return ret;
    }

    mat<DimRows,DimCols,T> invert_transpose() {
        mat<DimRows,DimCols,T> ret = adjugate();
        T tmp = ret[0]*rows[0];
        return ret/tmp;
    }
};

/////////////////////////////////////////////////////////////////////////////////

template<size_t DimRows,size_t DimCols,typename T> vec<DimRows,T> operator*(const mat<DimRows,DimCols,T>& lhs, const vec<DimCols,T>& rhs) {
    vec<DimRows,T> ret;
    for (size_t i=DimRows; i--; ret[i]=lhs[i]*rhs);
    return ret;
}


template<size_t DimRows,size_t DimCols,typename T>mat<DimCols,DimRows,T> operator/(mat<DimRows,DimCols,T> lhs, const T& rhs) {
    for (size_t i=DimRows; i--; lhs[i]=lhs[i]*(1./rhs));
    return lhs;
}


template <size_t DimRows,size_t DimCols,class T> std::ostream& operator<<(std::ostream& out, mat<DimRows,DimCols,T>& m) {
    for (size_t i=DimRows; i--; ) out << m[i] << std::endl;
    return out;
}

/////////////////////////////////////////////////////////////////////////////////

typedef vec<2,  float> Vec2f;
typedef vec<2,  int>   Vec2i;
typedef vec<3,  float> Vec3f;
typedef vec<3,  int>   Vec3i;
typedef mat<4,4,float> Matrix;

#endif //__GEOMETRY_H__

