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

#include "Core/Exception.h"

#define CIALLO_BEGIN_NS \
namespace cocoa { \
namespace ciallo {

#define CIALLO_END_NS \
}} /* namespace cocoa::ciallo */

CIALLO_BEGIN_NS


/* Basic types and decalarations */
using GrScalar = double;

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
