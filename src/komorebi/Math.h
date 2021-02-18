#ifndef COCOA_MATH_H
#define COCOA_MATH_H

#include <ostream>

#include "komorebi/Komorebi.h"
KMR_NAMESPACE_BEGIN

template<typename T, uint32_t N>
class Vec
{
public:
    Vec() = default;

    Vec(const std::initializer_list<T>& list)
    { initializer_list(list); }

    template<typename Other>
    explicit Vec(const Vec<Other, N>& that)
    { copy<Other>(that); }

    Vec<T, N>& operator=(const std::initializer_list<T>& list)
    {
        initializer_list(list);
        return *this;
    }

    template<typename Other>
    Vec<T, N>& operator=(const Vec<Other, N>& that)
    {
        copy<Other>(that);
        return *this;
    }

    T& operator[](uint32_t idx)
    {
        if (idx >= N)
            throw std::runtime_error("Invalid index value for Vec");
        return fComp[idx];
    }

    template<typename U, typename V, uint32_t M>
    friend U dot(const Vec<U, M>&, const Vec<V, M>&);

    template<typename U, typename S, uint32_t M>
    friend Vec<U, M> operator*(const Vec<U, M>&, const S&);

    template<typename U, typename V, uint32_t M>
    friend Vec<U, M> operator+(const Vec<U, M>& v1, const Vec<V, M>& v2);

    template<typename U, uint32_t M>
    friend std::ostream& operator<<(std::ostream&, const Vec<U, M>&);

private:
    void initializer_list(const std::initializer_list<T>& list)
    {
        uint32_t i = 0;
        for (const T& e : list)
            fComp[i++] = std::move(e);
    }

    template<typename Other>
    void copy(const Vec<Other, N>& other)
    {
        uint32_t i = 0;
        for (const Other& e : other.fComp)
            fComp[i++] = e;
    }

private:
    T       fComp[N];
};

template<typename T, typename V, uint32_t N>
static T dot(const Vec<T, N>& v1, const Vec<V, N>& v2)
{
    T sum = v1.fComp[0] * v2.fComp[0];
    for (uint32_t i = 1; i < N; i++)
        sum += v1.fComp[i] * v2.fComp[i];
    return sum;
}

template<typename T, typename S, uint32_t N>
Vec<T, N> operator*(const Vec<T, N>& v, const S& scalar)
{
    Vec<T, N> ret;
    for (uint32_t i = 0; i < N; i++)
        ret.fComp[i] = v.fComp[i] * scalar;
    return ret;
}

template<typename T, typename V, uint32_t N>
Vec<T, N> operator+(const Vec<T, N>& v1, const Vec<V, N>& v2)
{
    Vec<T, N> ret;
    for (uint32_t i = 0; i < N; i++)
        ret.fComp[i] = v1.fComp[i] + v2.fComp[i];
    return ret;
}

template<typename T, uint32_t N>
std::ostream& operator<<(std::ostream& os, const Vec<T, N>& v)
{
    os << '[' << v.fComp[0];
    for (uint32_t i = 1; i < N; i++)
        os << ", " << v.fComp[i];
    os << ']';
    return os;
}

using Vec2b = Vec<uint8_t, 2>;
using Vec3b = Vec<uint8_t, 3>;
using Vec4b = Vec<uint8_t, 4>;
using Vec2w = Vec<int16_t, 2>;
using Vec3w = Vec<int16_t, 3>;
using Vec4w = Vec<int16_t, 4>;
using Vec2uw = Vec<uint16_t, 2>;
using Vec3uw = Vec<uint16_t, 3>;
using Vec4uw = Vec<uint16_t, 4>;
using Vec2i = Vec<int32_t, 2>;
using Vec3i = Vec<int32_t, 3>;
using Vec4i = Vec<int32_t, 4>;
using Vec2ui = Vec<uint32_t, 2>;
using Vec3ui = Vec<uint32_t, 3>;
using Vec4ui = Vec<uint32_t, 4>;
using Vec2l = Vec<int64_t, 2>;
using Vec3l = Vec<int64_t, 3>;
using Vec4l = Vec<int64_t, 4>;
using Vec2ul = Vec<uint64_t, 2>;
using Vec3ul = Vec<uint64_t, 3>;
using Vec4ul = Vec<uint64_t, 4>;
using Vec2f = Vec<float, 2>;
using Vec3f = Vec<float, 3>;
using Vec4f = Vec<float, 4>;
using Vec2d = Vec<double, 2>;
using Vec3d = Vec<double, 3>;
using Vec4d = Vec<double, 4>;

KMR_NAMESPACE_END
#endif //COCOA_MATH_H
