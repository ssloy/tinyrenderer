#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>
#include <iomanip>

template<size_t size,typename numbertype> class mat;

template <size_t Dim,typename numbertype> struct vec {
    numbertype items[Dim];

    vec<Dim,numbertype> normalize() const { return (*this)/norm(); }
    numbertype norm() const { return std::sqrt((*this)*(*this)); }

    /*
    size_t maxElementPos() const { //найти номер максимального элемента
        size_t ret=0;
        for (size_t i=Dim;--i;) {
            if (items[i]>items[ret]) {
                ret=i;
            }
        }
        return ret;
    }
    */

    static vec<Dim,numbertype> fill(const numbertype& val=0) { // построить вектор, заполненный константой TODO переделать в конструктор
        vec<Dim, numbertype> ret;
        for (size_t i=Dim; i--; ret[i]=val);
        return ret;
    }

    /*
    bool operator!=(const vec<Dim,numbertype>&v) { // TODO переделать в >=, пригодится для растеризатора
        for (size_t i=Dim; i--; ) {
            if(v[i]!=items[i])
                return true;
        }
        return false;
    }
    */

    numbertype& operator [](size_t index) {
        assert(index<Dim);
        return items[index];
    }

    const numbertype& operator [](size_t index) const {
        assert(index<Dim);
        return items[index];
    }
};

template<size_t Dim,typename numbertype>vec<Dim,numbertype> operator+(vec<Dim,numbertype> lhs, const vec<Dim,numbertype>& rhs) {
    for (size_t i=Dim; i--; lhs[i]+=rhs[i]);
    return lhs;
}

template<size_t Dim,typename numbertype> vec<Dim,numbertype> operator^(const vec<Dim,numbertype>&lhs, const vec<Dim,numbertype>& rhs) { // векторное умножение TODO переделать в cross()
    vec<Dim,numbertype> ret;
    for (size_t i=Dim; i--; ) {
        mat<3,numbertype> temp;
        temp[0]=vec<Dim,numbertype>::fill(0);
        temp[0][i]=1;
        temp[1]=lhs;
        temp[2]=rhs;
        ret[i]=temp.det();
    }
    return ret;
}

template<size_t Dim,typename numbertype> numbertype operator*(const vec<Dim,numbertype>&lhs, const vec<Dim,numbertype>& rhs) {
    numbertype ret=0;
    for (size_t i=Dim; i--; ret+=lhs[i]*rhs[i]);
    return ret;
}

template<size_t Dim,typename numbertype>vec<Dim,numbertype> operator-(vec<Dim,numbertype> lhs, const vec<Dim,numbertype>& rhs) {
    for (size_t i=Dim; i--; lhs[i]-=rhs[i]);
    return lhs;
}

template<size_t Dim,typename numbertype>vec<Dim,numbertype> operator*(vec<Dim,numbertype> lhs, const numbertype& rhs) {
    for (size_t i=Dim; i--; lhs[i]*=rhs);
    return lhs;
}

template<size_t Dim,typename numbertype>vec<Dim,numbertype> operator/(vec<Dim,numbertype> lhs, const numbertype& rhs) {
    for (size_t i=Dim; i--; lhs[i]/=rhs);
    return lhs;
}

template<size_t len,size_t Dim, typename numbertype> vec<len,numbertype> embed(const vec<Dim,numbertype> &v,const numbertype& fill=1) { // погружение вектора
    vec<len,numbertype> ret = vec<len,numbertype>::fill(fill);
    for (size_t i=Dim; i--; ret[i]=v[i]);
    return ret;
}

template<size_t len,size_t Dim, typename numbertype> vec<len,numbertype> proj(const vec<Dim,numbertype> &v) { //проекция вектора
    vec<len,numbertype> ret;
    for (size_t i=len; i--; ret[i]=v[i]);
    return ret;
}

template<size_t Dim,typename numbertype> std::ostream& operator<<(std::ostream& out,const vec<Dim,numbertype>& v) {
    out<<"{ ";
    for (size_t i=0; i<Dim; i++) {
        out<<std::setw(6)<<v[i]<<" ";
    }
    out<<"} ";
    return out;
}

/////////////////////////////////////////////////////////////////////////////////

template<size_t size,typename numbertype> struct dt {
    static numbertype det(const mat<size,numbertype>& src) {
        numbertype ret=0;
        for (size_t i=size; i--; ret += src[0][i]*src.algAdd(0,i));
        return ret;
    }
};

template<typename numbertype> struct dt<1,numbertype> {
    static numbertype det(const mat<1,numbertype>& src) {
        return src[0][0];
    }
};

/////////////////////////////////////////////////////////////////////////////////

template<size_t size,typename numbertype> class mat {
    vec<size,numbertype> rows[size];
public:
    mat() {}

    mat(const mat<size,numbertype>& src) {
        for (size_t i=size; i--; )
            for (size_t j=size; j--; rows[i][j]=src[i][j]);
    }

    std::ostream& print(std::ostream& out) const {
        for (size_t i=0;i<size;i++) {
            out<<rows[i]<<"\n";
        }
        return out;
    }

    /*
    vec<size,numbertype> minimums() {
        vec<size,numbertype> ret=rows[0];
        for (size_t i=size;--i;) {
            for (size_t j=size;j--;) {
                ret[j]=std::min(ret[j],rows[i][j]);
            }
        }
        return ret;
    }

    vec<size,numbertype> maximums() {
        vec<size,numbertype> ret=rows[0];
        for (size_t i=size;--i;) {
            for (size_t j=size;j--;) {
                ret[j]=std::max(ret[j],rows[i][j]);
            }
        }
        return ret;
    }
    */


    vec<size,numbertype>& operator[] (size_t index) {
        return rows[index];
    }

    const vec<size,numbertype>& operator[] (size_t index) const {
        return rows[index];
    }

    static mat<size,numbertype> identity() {
        mat<size,numbertype> ret;
        for (size_t i=size; i--; )
            for (size_t j=size;j--; ret[i][j]=(i==j));
        return ret;
    }

    numbertype det() const {
        return dt<size,numbertype>::det(*this);
    }

    mat<size-1,numbertype> minor(size_t row,size_t col) const {
        mat<size-1,numbertype> ret;
        for (size_t i=size-1; i--; )
            for (size_t j=size-1;j--; ret[i][j]=rows[i<row?i:i+1][j<col?j:j+1]);
        return ret;
    }

    numbertype algAdd(size_t row, size_t col) const {
        return minor(row,col).det()*((row+col)%2 ? -1 : 1);
    }

    mat<size,numbertype> adjugate() const {
        mat<size,numbertype> ret;
        for (size_t i=size; i--; )
            for (size_t j=size; j--; ret[i][j]=algAdd(i,j));
        return ret;
    }

    mat<size,numbertype> invert_transpose() const {
        mat<size,numbertype> ret = adjugate();
        return ret/(ret[0]*rows[0]);
    }

    /*
    void setCol(const numbertype& val,size_t col) {
        for (size_t i=size; i--; rows[i][col]=val);
    }
    */

};

template<size_t Dim,typename numbertype>vec<Dim,numbertype> operator*(const mat<Dim,numbertype>& lhs, const vec<Dim,numbertype>& rhs) {
    vec<Dim,numbertype> ret;
    for (size_t i=Dim; i--; ret[i]=lhs[i]*rhs);
    return ret;
}

template<size_t Dim,typename numbertype>mat<Dim,numbertype> operator*(const mat<Dim,numbertype>& lhs, const mat<Dim,numbertype>& rhs) { // TODO уфф
    mat<Dim,numbertype> result;
    for (int i=0; i<Dim; i++) {
        for (int j=0; j<Dim; j++) {
            result[i][j] = numbertype();
            for (int k=0; k<Dim; k++) {
                result[i][j] += lhs[i][k]*rhs[k][j];
            }
        }
    }
    return result;
}

/*
template<size_t size,typename numbertype>mat<size,numbertype> operator/(mat<size,numbertype> lhs, const numbertype& rhs) {
    for (size_t i=size; i--; lhs[i]=lhs[i]/rhs);
    return lhs;
}
*/

template<size_t size,typename numbertype> std::ostream& operator<<(std::ostream& os,const mat<size,numbertype>& v) {
    return v.print(os);
}



/////////////////////////////////////////////////////////////////////////////////

typedef vec<2,float> Vec2f;
typedef vec<2,int>   Vec2i;
typedef vec<3,float> Vec3f;
typedef vec<3,int>   Vec3i;
typedef mat<4,float> Matrix;

#endif //__GEOMETRY_H__

