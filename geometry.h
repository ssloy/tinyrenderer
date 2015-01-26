#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>
#include <iomanip>

template<size_t size,typename number_t> class mat;

template <size_t Dim,typename number_t> struct vec {
    number_t items[Dim];

    vec<Dim,number_t> normalize() const { return (*this)/norm(); }
    number_t norm() const { return std::sqrt((*this)*(*this)); }

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

    static vec<Dim,number_t> fill(const number_t& val=0) { // построить вектор, заполненный константой TODO переделать в конструктор
        vec<Dim, number_t> ret;
        for (size_t i=Dim; i--; ret[i]=val);
        return ret;
    }

    /*
    bool operator!=(const vec<Dim,number_t>&v) { // TODO переделать в >=, пригодится для растеризатора
        for (size_t i=Dim; i--; ) {
            if(v[i]!=items[i])
                return true;
        }
        return false;
    }
    */

    number_t& operator [](size_t index) {
        assert(index<Dim);
        return items[index];
    }

    const number_t& operator [](size_t index) const {
        assert(index<Dim);
        return items[index];
    }
};

template<size_t Dim,typename number_t>vec<Dim,number_t> operator+(vec<Dim,number_t> lhs, const vec<Dim,number_t>& rhs) {
    for (size_t i=Dim; i--; lhs[i]+=rhs[i]);
    return lhs;
}


template<size_t Dim,typename number_t> number_t operator*(const vec<Dim,number_t>&lhs, const vec<Dim,number_t>& rhs) {
    number_t ret=0;
    for (size_t i=Dim; i--; ret+=lhs[i]*rhs[i]);
    return ret;
}

template<size_t Dim,typename number_t>vec<Dim,number_t> operator-(vec<Dim,number_t> lhs, const vec<Dim,number_t>& rhs) {
    for (size_t i=Dim; i--; lhs[i]-=rhs[i]);
    return lhs;
}

template<size_t Dim,typename number_t>vec<Dim,number_t> operator*(vec<Dim,number_t> lhs, const number_t& rhs) {
    for (size_t i=Dim; i--; lhs[i]*=rhs);
    return lhs;
}

template<size_t Dim,typename number_t>vec<Dim,number_t> operator/(vec<Dim,number_t> lhs, const number_t& rhs) {
    for (size_t i=Dim; i--; lhs[i]/=rhs);
    return lhs;
}

template<size_t len,size_t Dim, typename number_t> vec<len,number_t> embed(const vec<Dim,number_t> &v,const number_t& fill=1) { // погружение вектора
    vec<len,number_t> ret = vec<len,number_t>::fill(fill);
    for (size_t i=Dim; i--; ret[i]=v[i]);
    return ret;
}

template<size_t len,size_t Dim, typename number_t> vec<len,number_t> proj(const vec<Dim,number_t> &v) { //проекция вектора
    vec<len,number_t> ret;
    for (size_t i=len; i--; ret[i]=v[i]);
    return ret;
}

template<size_t Dim,typename number_t> std::ostream& operator<<(std::ostream& out,const vec<Dim,number_t>& v) {
    out<<"{ ";
    for (size_t i=0; i<Dim; i++) {
        out<<std::setw(6)<<v[i]<<" ";
    }
    out<<"} ";
    return out;
}

/////////////////////////////////////////////////////////////////////////////////

template<size_t size,typename number_t> struct dt {
    static number_t det(const mat<size,number_t>& src) {
        number_t ret=0;
        for (size_t i=size; i--; ret += src[0][i]*src.algAdd(0,i));
        return ret;
    }
};

template<typename number_t> struct dt<1,number_t> {
    static number_t det(const mat<1,number_t>& src) {
        return src[0][0];
    }
};

/////////////////////////////////////////////////////////////////////////////////

template<size_t size,typename number_t> class mat {
    vec<size,number_t> rows[size];
public:
    mat() {}

    mat(const mat<size,number_t>& src) {
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
    vec<size,number_t> minimums() {
        vec<size,number_t> ret=rows[0];
        for (size_t i=size;--i;) {
            for (size_t j=size;j--;) {
                ret[j]=std::min(ret[j],rows[i][j]);
            }
        }
        return ret;
    }

    vec<size,number_t> maximums() {
        vec<size,number_t> ret=rows[0];
        for (size_t i=size;--i;) {
            for (size_t j=size;j--;) {
                ret[j]=std::max(ret[j],rows[i][j]);
            }
        }
        return ret;
    }
    */

    vec<size,number_t> col(const size_t& idx) const {
        assert(idx<size);
        vec<size,number_t> ret;
        for(size_t i=size; i--; ret[i]=rows[i][idx]);
        return ret;
    }

    vec<size,number_t>& operator[] (size_t index) {
        return rows[index];
    }

    const vec<size,number_t>& operator[] (size_t index) const {
        return rows[index];
    }

    static mat<size,number_t> identity() {
        mat<size,number_t> ret;
        for (size_t i=size; i--; )
            for (size_t j=size;j--; ret[i][j]=(i==j));
        return ret;
    }

    number_t det() const {
        return dt<size,number_t>::det(*this);
    }

    mat<size-1,number_t> minor(size_t row,size_t col) const {
        mat<size-1,number_t> ret;
        for (size_t i=size-1; i--; )
            for (size_t j=size-1;j--; ret[i][j]=rows[i<row?i:i+1][j<col?j:j+1]);
        return ret;
    }

    number_t algAdd(size_t row, size_t col) const {
        return minor(row,col).det()*((row+col)%2 ? -1 : 1);
    }

    mat<size,number_t> adjugate() const {
        mat<size,number_t> ret;
        for (size_t i=size; i--; )
            for (size_t j=size; j--; ret[i][j]=algAdd(i,j));
        return ret;
    }

    mat<size,number_t> invert_transpose() const {
        mat<size,number_t> ret = adjugate();
        return ret/(ret[0]*rows[0]);
    }

    /*
    void setCol(const number_t& val,size_t col) {
        for (size_t i=size; i--; rows[i][col]=val);
    }
    */

};

template<size_t size,typename number_t>vec<size,number_t> operator*(const mat<size,number_t>& lhs, const vec<size,number_t>& rhs) {
    vec<size,number_t> ret;
    for (size_t i=size; i--; ret[i]=lhs[i]*rhs);
    return ret;
}

template<size_t size,typename number_t>mat<size,number_t> operator*(const mat<size,number_t>& lhs, const mat<size,number_t>& rhs) { // TODO уфф
    mat<size,number_t> result;
    for (size_t i=size; i--; )
        for (size_t j=size; j--; result[i][j]=lhs[i]*rhs.col(j));
    return result;
}

/*
template<size_t size,typename number_t>mat<size,number_t> operator/(mat<size,number_t> lhs, const number_t& rhs) {
    for (size_t i=size; i--; lhs[i]=lhs[i]/rhs);
    return lhs;
}
*/

template<size_t size,typename number_t> std::ostream& operator<<(std::ostream& os,const mat<size,number_t>& v) {
    return v.print(os);
}



/////////////////////////////////////////////////////////////////////////////////

typedef vec<2,float> Vec2f;
typedef vec<2,int>   Vec2i;
typedef vec<3,float> Vec3f;
typedef vec<3,int>   Vec3i;
typedef mat<4,float> Matrix;

template<typename number_t> vec<3,number_t> cross(vec<3,number_t> lhs, vec<3,number_t> rhs) {
    vec<3,number_t> ret;
    for (size_t i=3; i--; ) {
        mat<3,number_t> temp;
        temp[0]=vec<3,number_t>::fill(0);
        temp[0][i]=1;
        temp[1]=lhs;
        temp[2]=rhs;
        ret[i]=temp.det();
    }
    return ret;
}

#endif //__GEOMETRY_H__

