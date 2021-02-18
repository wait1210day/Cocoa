/**
 * Ciallo is a part of the Cocoa project,
 * and Ciallo is a dom/CSS rendering engine based on Skia 2D graphics library.
*/

#ifndef __GR_BASE_H__
#define __GR_BASE_H__

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <string>
#include <stdint.h>

#include "core/Exception.h"

#define CIALLO_BEGIN_NS \
namespace cocoa { \
namespace ciallo {

#define CIALLO_END_NS \
}} /* namespace cocoa::ciallo */

CIALLO_BEGIN_NS

class GrException : public ARTException
{
public:
    GrException() = default;
    ~GrException() = default;
};

/* Basic types and decalarations */
using GrScalar = double;
template<typename _Tp>
using GrVector = std::vector<_Tp>;
template<typename _Tp>
using GrList = std::list<_Tp>;
template<typename _T1, typename _T2>
using GrPair = std::pair<_T1, _T2>;

template<typename _Tk, typename _Tv>
using GrMap = std::map<_Tk, _Tv>;
template<typename _Tk, typename _Tv>
using GrUnorderedMap = std::unordered_map<_Tk, _Tv>;

struct GrSize
{
    std::size_t width;
    std::size_t height;
    inline bool operator==(const GrSize& siz) const
    { return this->width == siz.width && this->height == siz.height; }

    inline bool operator!=(const GrSize& siz) const
    { return this->width != siz.width || this->height != siz.height; }
};

/* Ciallo extensions for C++ language */
class cxx_abi
{
public:
    static std::string resolve_name(const std::string& name);
    template<typename _Tp>
    static std::string resolve_typename()
    { return resolve_name(typeid(_Tp).name()); }
};

class GrColorSpecifier
{
public:
    GrColorSpecifier(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha = 0xff) {
        fValue = (alpha << 24) | (r << 16) | (g << 8) | b;
        fR = r;
        fG = g;
        fB = b;
        fA = alpha;
    }

    inline uint32_t value_dword() const { return fValue; }
    inline uint8_t value_byte(int i) const {
        const uint8_t val = reinterpret_cast<const uint8_t*>(&fValue)[i];
        return val;
    };
    inline GrScalar value_rf() const { return static_cast<GrScalar>(fR) / 255.0; }
    inline GrScalar value_gf() const { return static_cast<GrScalar>(fG) / 255.0; }
    inline GrScalar value_bf() const { return static_cast<GrScalar>(fB) / 255.0; }
    inline GrScalar value_af() const { return static_cast<GrScalar>(fA) / 255.0; }

private:
    uint32_t    fValue;
    uint8_t     fR;
    uint8_t     fG;
    uint8_t     fB;
    uint8_t     fA;
};

#define GR_SLOTS
#define GR_SIGNALS

CIALLO_END_NS

#endif /* __GR_BASE_H__ */
