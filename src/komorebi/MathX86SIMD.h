#ifndef COCOA_MATHX86SIMD_H
#define COCOA_MATHX86SIMD_H

#ifndef __x86_64__
#error "Your CPU doesn't support x86 SIMD instructions"
#endif // __x86_64__

#include <x86intrin.h>

#include "komorebi/Komorebi.h"
#include "komorebi/Math.h"
KMR_NAMESPACE_BEGIN

namespace vec_utils
{

template<typename T, uint32_t N>
class Ret
{
public:
    Ret(Vec<T, N>& vref, uint32_t idx)
        : _vref(vref), _idx(idx) {}

    // Implicit conversion
    inline operator T()
    {
        return _vref.load(_idx);
    }

    inline Ret& operator=(const T& val)
    {
        _vref.store(_idx, val);
        return *this;
    }

private:
    Vec<T, N>&   _vref;
    uint32_t     _idx;
};

} // namespace vec_utils

template<uint32_t N>
class Vec<float, N>
{
public:
    Vec() : cache_index(0)
    {
        make_cache(0);
    }

    template<typename Other>
    Vec(const std::initializer_list<Other>& list)
        : cache_index(0)
    { load_from_list(list); }

    template<typename Other>
    Vec(const Vec<Other, N>& other)
        : cache_index(0)
    { copy_from_other(other); }

    Vec(const Vec<float, N>& that)
        : cache_index(0)
    { copy_from_same(that); }

    template<typename Other>
    Vec<float, N>& operator=(const std::initializer_list<Other>& list)
    {
        load_from_list(list);
        return *this;
    }

    template<typename Other>
    Vec<float, N>& operator=(const Vec<Other, N>& other)
    {
        copy_from_other(other);
        return *this;
    }

    Vec<float, N>& operator=(const Vec<float, N>& that)
    {
        copy_from_same(that);
        return *this;
    }

    float load(uint32_t idx) const
    {
        /* cache_index = idx / 8 = idx >> 3 */
        if (cache_index != idx >> 3)
            make_cache(idx >> 3);

        return cache_value[idx & 0x7];
    }

    void store(uint32_t idx, float val)
    {
        /* Load, modify and then store */
        float value_group[8];
        _mm256_storeu_ps(value_group, va[idx >> 3]);
        value_group[idx & 0x7] = val;
        va[idx >> 3] = _mm256_loadu_ps(value_group);

        /* If current value-group is cached, we should update it */
        if (cache_index == idx >> 3)
            make_cache(idx >> 3);
    }

    vec_utils::Ret<float, N> operator[](uint32_t idx)
    {
        return vec_utils::Ret<float, N>(*this, idx);
    }

    template<typename S, uint32_t M>
    friend Vec<float, M> operator*(const Vec<float, M>&, const S&);

    template<typename V, uint32_t M>
    friend Vec<float, M> operator+(const Vec<float, M>& v1, const Vec<V, M>& v2);

    template<uint32_t M>
    friend Vec<float, M> operator+(const Vec<float, M>& v1, const Vec<float, M>& v2);

    template<uint32_t M>
    friend std::ostream& operator<<(std::ostream&, Vec<float, M>&);

private:
    template<typename Other>
    void load_from_list(const std::initializer_list<Other>& list)
    {
        if (list.size() != N)
            throw std::runtime_error("Bad size of initializer list");

        float value_group[8];   /* Each __m512 type can store 16 float values */
        uint8_t i = 0;          /* Index of value group */
        uint32_t p = 0;         /* Index of __m512 array */

        for (float val : list)
        {
            value_group[i++] = val;
            if (i == 8)
            {
                i = 0;
                va[p++] = _mm256_loadu_ps(value_group);
            }
        }
        /* Store remaining elements */
        if (i > 0)
            va[p] = _mm256_loadu_ps(value_group);

        make_cache(0);
    }

    void copy_from_same(const Vec<float, N>& that)
    {
        for (uint32_t i = 0; i < va_size; i++)
            va[i] = that.va[i];
        make_cache(0);
    }

    template<typename Other>
    void copy_from_other(const Vec<Other, N>& other)
    {
        float values[8];
        uint8_t q = 0;
        uint32_t p = 0;

        for (uint32_t i = 0; i < N; i++)
        {
            values[q++] = other[i];
            if (q == 8)
            {
                q = 0;
                va[p++] = _mm256_loadu_ps(values);
            }
        }
        if (q > 0)
            va[p] = _mm256_loadu_ps(values);

        make_cache(0);
    }

    void make_cache(uint32_t idx)
    {
        cache_index = idx;
        _mm256_storeu_ps(cache_value, va[cache_index]);
    }

private:
    static constexpr uint32_t va_size = N / 8 + 1;
    __m256                    va[va_size];
    float                     cache_value[8];
    uint32_t                  cache_index;
};

template<typename S, uint32_t M>
Vec<float, M> operator*(const Vec<float, M>& v, const S& scalar)
{
    auto fs = static_cast<float>(scalar);
    float vectorize[8] = { fs, fs, fs, fs,
                           fs, fs, fs, fs };
    __m256 b = _mm256_loadu_ps(vectorize);

    Vec<float, M> ret;
    for (uint32_t i = 0; i < v.va_size; i++)
        ret.va[i] = _mm256_mul_ps(v.va[i], b);

    ret.make_cache(0);
    return ret;
}

template<typename V, uint32_t M>
Vec<float, M> operator+(const Vec<float, M>& v1, const Vec<V, M>& v2)
{
    Vec<float, M> ret;
    for (uint32_t i = 0; i < (M >> 3); i++)
    {
        float values[8] = { static_cast<float>(v2[i]),
                            static_cast<float>(v2[i + 1]),
                            static_cast<float>(v2[i + 2]),
                            static_cast<float>(v2[i + 3]),
                            static_cast<float>(v2[i + 4]),
                            static_cast<float>(v2[i + 5]),
                            static_cast<float>(v2[i + 6]),
                            static_cast<float>(v2[i + 7]) };
        __m256 value_packed = _mm256_loadu_ps(values);
        ret.va[i] = _mm256_add_ps(v1.va[i], value_packed);
    }

    for (uint32_t i = 0; i < (M & 0x7); i++)
        ret[i] = v1[i] + v2[i];

    /* We don't need to update cache here.
      The copy constructor of Vec<...> will do. */
    return ret;
}

template<uint32_t M>
Vec<float, M> operator+(const Vec<float, M>& v1, const Vec<float, M>& v2)
{
    Vec<float, M> ret;
    for (uint32_t i = 0; i < v1.va_size; i++)
        ret.va[i] = _mm256_add_ps(v1.va[i], v2.va[i]);

    /* We don't need to update cache here.
      The copy constructor of Vec<...> will do. */
    return ret;
}

template<uint32_t M>
std::ostream& operator<<(std::ostream& os, Vec<float, M>& v)
{
    os << "[";
    for (uint32_t p = 0, i = 0; p < v.va_size; p++)
    {
        float values[8];
        _mm256_storeu_ps(values, v.va[p]);

        for (uint32_t q = 0; q < 8 && i < M; ++q, ++i)
        {
            os << values[q];
            if (i < M - 1)
                os << ", ";
        }
    }
    os << ']';
    return os;
}

KMR_NAMESPACE_END
#endif //COCOA_MATHX86SIMD_H
