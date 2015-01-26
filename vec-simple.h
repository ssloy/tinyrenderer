#ifndef VECSIMPLE_H
#define VECSIMPLE_H
#ifndef VEC_H
#define VEC_H
#include <ostream>
#include <iomanip>
#include "macroses.h"
template<size_t DimRows,size_t DimCols,typename Number> class mat;
template <size_t Dim,typename Number> class vec
{    

public:
    Number items[Dim];
    typedef Number NumberT;
    static const size_t DimN=Dim;
///////////////////////////////нормировать вектора
    vec<Dim,Number> normalize() const
    {
        return((*this)/norm());
    }
///////////////////////////////найти норму вектора
    Number norm() const
    {
        return(sqrt((*this)*(*this)));
    }
///////////////////////////////найти номер максимального элемента
    size_t maxElementPos() const
    {
        size_t ret=0;
        for(size_t i=Dim;--i;)
        {
            if(items[i]>items[ret])
            {
                ret=i;
            }
        }
        return(ret);
    }
///////////////////////////////построить вектор, заполненный константой
    static vec<Dim,Number > fill(const Number& val=0)
    {
        vec<Dim, Number> ret;
        for(size_t i=Dim;i--;)
        {
            ret[i]=val;
        }
        return(ret);
    }

    bool operator!=(const vec<Dim,Number>&v)
    {
        for(size_t i=Dim;i--;)
        {
            if(v[i]!=items[i])
            {
                return(true);
            }
        }
        return(false);
    }

    Number& operator [](size_t index)
    {
        ASSERT(index<Dim);
        return(items[index]);
    }

    const Number& operator [](size_t index) const
    {
        ASSERT(index<Dim);
        return(items[index]);
    }
};
/////////////////////////////сумма векторов
template<size_t Dim,typename Number>vec<Dim,Number > operator+(vec<Dim,Number > lhs, const vec<Dim,Number >& rhs)
{
    for(size_t i=Dim;i--;)
    {
        lhs[i]+=rhs[i];
    }
    return(lhs);

}
/////////////////////////////скалярное умножение векторов
template<size_t Dim,typename Number> vec<Dim,Number> operator^(const vec<Dim,Number >&lhs, const vec<Dim,Number >& rhs)
{
    vec<Dim,Number> ret;
    for(size_t i=Dim;i--;)
    {
        mat<Dim,3,Number> temp;
        temp[0]=ret.fill(0);
        temp[0][i]=1;
        temp[1]=lhs;
        temp[2]=rhs;
        ret[i]=temp.det();
    }
    return(ret);
}
/////////////////////////////скалярное умножение векторов
template<size_t Dim,typename Number> Number operator*(const vec<Dim,Number >&lhs, const vec<Dim,Number >& rhs)
{
    Number ret=0;
    for(size_t i=Dim;i--;)
    {
        ret+=lhs[i]*rhs[i];
    }
    return(ret);
}
/////////////////////////////разность векторов
template<size_t Dim,typename Number>vec<Dim,Number > operator-(vec<Dim,Number > lhs, const vec<Dim,Number >& rhs)
{
    for(size_t i=Dim;i--;)
    {
        lhs[i]-=rhs[i];
    }
    return(lhs);
}
/////////////////////////////умножение вектора на скаляр
template<size_t Dim,typename Number>vec<Dim,Number > operator*(vec<Dim,Number > lhs, const Number& rhs)
{
    for(size_t i=Dim;i--;)
    {
        lhs[i]*=rhs;
    }
    return(lhs);
}
/////////////////////////////деление вектора на скаляр
template<size_t Dim,typename Number>vec<Dim,Number > operator/(vec<Dim,Number > lhs, const Number& rhs)
{    
    for(size_t i=Dim;i--;)
    {
        lhs[i]/=rhs;
    }
    return(lhs);
}
/////////////////////////////погружение вектора
template<size_t len,size_t Dim, typename Number> vec<len,Number > dive(const vec<Dim,Number> &v,const Number& fill=1)
{
    vec<len,Number> ret;
    for(size_t i=Dim;i<len;i++)
    {
        ret[i]=fill;
    }
    for(size_t i=Dim;i--;)
    {
        ret[i]=v[i];
    }
    return(ret);
}
/////////////////////////////проекция вектора
template<size_t len,size_t Dim, typename Number> vec<len,Number > proj(const vec<Dim,Number> &v)
{    
    vec<len,Number> ret;
    for(size_t i=len;i--;)
    {
        ret[i]=v[i];
    }
    return(ret);
}
/////////////////////////////вывод вектора в ostream
template<size_t Dim,typename Number> std::ostream& operator<<(std::ostream& out,const vec<Dim,Number >& v)
{
    out<<"{ ";
    for(size_t i=0;i<Dim;i++)
    {
        out<<std::setw(6)<<v[i]<<" ";
    }
    out<<"} ";
    return(out);
}

template<size_t DimCols,size_t DimRows,typename Number> struct dt
{
    static Number det(const mat<DimRows,DimCols,Number>& src)
    {
        Number ret=0;
        for(size_t i=DimCols;i--;)
        {
            ret+=src[0][i] * src.algAdd(0,i);
        }
        return(ret);
    }
};

template<typename Number> struct dt<1,1,Number>
{
    static Number det(const mat<1,1,Number>& src)
    {
        return(src[0][0]);
    }
};

template<size_t DimRows,size_t DimCols,typename Number> class mat
{
    vec<DimCols,Number> rows[DimRows];


public:
    typedef Number NumberT;
    static size_t shift(size_t in,const size_t& val)
    {
        return(in<val ? in : ++in);
    }

    mat()
    {
    }
    mat(const mat<DimRows,DimCols,Number >& src)
    {
        for(size_t i=DimCols;i--;)
        {
            for(size_t j=DimRows;j--;)
            {
                const Number t=src[i][j];
                rows[i][j]=t;
            }
        }
    }

    std::ostream& print(std::ostream& out) const
    {
        for(size_t i=0;i<DimRows;i++)
        {
            out<<rows[i]<<"\n";
        }
        return(out);
    }
    vec<DimCols,Number > minimums()
    {
        vec<DimCols,Number > ret=rows[0];
        for(size_t i=DimRows;--i;)
        {
            for(size_t j=DimCols;j--;)
            {
                ret[j]=std::min(ret[j],rows[i][j]);
            }
        }
        return(ret);
    }

    vec<DimCols,Number > maximums()
    {
        vec<DimCols,Number > ret=rows[0];
        for(size_t i=DimRows;--i;)
        {
            for(size_t j=DimCols;j--;)
            {
                ret[j]=std::max(ret[j],rows[i][j]);
            }
        }
        return(ret);
    }
    vec<DimCols,Number>& operator[] (size_t index)
    {
        return(rows[index]);
    }

    const vec<DimCols,Number>& operator[] (size_t index) const
    {
        return(rows[index]);
    }
    static mat<DimCols,DimRows,Number> ones()
    {
        mat<DimCols,DimRows,Number> ret;
        for(size_t i=DimRows;i--;)
        {
            for(size_t j=DimCols;j--;)
            {
                ret[i][j]=(i==j);
            }

        }
        return(ret);
    }

    Number det() const
    {
        return(dt<DimCols,DimRows,Number>::det(*this));
    }

    mat<DimRows-1,DimCols-1,Number> minor(size_t row,size_t col) const
    {
        mat<DimRows-1,DimCols-1,Number> ret;
        for(size_t i=DimRows-1;i--;)
        {
            for(size_t j=DimCols-1;j--;)
            {
                ret[i][j]=rows[ret.shift(i,row)][ret.shift(j,col)];
            }
        }
        return(ret);
    }


    Number algAdd(size_t row,size_t col) const
    {
        return(minor(row,col).det()*( (row+col)%2 ? -1 : 1));
    }

    mat<DimRows,DimCols,Number> Adjacent()const
    {
        mat<DimRows,DimCols,Number> ret;
        for(size_t i=DimRows;i--;)
        {
            for(size_t j=DimCols;j--;)
            {
                ret[i][j]=algAdd(i,j);
            }
        }
        return(ret);
    }

    mat<DimRows,DimCols,Number> invertT()const
    {
        mat<DimRows,DimCols,Number> ret=Adjacent();
        return(ret/(ret[0]*rows[0]));
    }

    void setCol(const Number& val,size_t col)
    {
        for(size_t i=DimRows;i--;)
        {
            rows[i][col]=val;
        }
    }

};

template<size_t Dim,typename Number>vec<Dim,Number > operator*(const mat<Dim,Dim,Number >& lhs, const vec<Dim,Number>& rhs)
{
    vec<Dim,Number> ret;
    for(size_t i=Dim;i--;)
    {
        ret[i]=lhs[i]*rhs;
    }
    return(ret);
}

template<size_t DimCols,size_t DimRows,typename Number>mat<DimCols,DimRows,Number > operator/(mat<DimCols,DimRows,Number> lhs, const Number& rhs)
{
    for(size_t i=DimRows;i--;)
    {
        lhs[i]=lhs[i]/rhs;
    }
    return(lhs);
}

template<size_t DimRows,size_t DimCols,typename Number> std::ostream& operator<<(std::ostream& os,const mat<DimRows,DimCols,Number>& v)
{
    return(v.print(os));
}

typedef mat<2,2,float > tempMat;

#endif // VEC_H


#endif // VECSIMPLE_H

