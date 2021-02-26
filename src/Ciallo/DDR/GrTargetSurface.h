#ifndef COCOA_GRTARGETSURFACE_H
#define COCOA_GRTARGETSURFACE_H

#ifdef COCOA_USE_OPENCL
#include <CL/cl.h>
#endif
#include "include/core/SkSurface.h"

#include "Ciallo/GrBase.h"
CIALLO_BEGIN_NS

class GrTargetSurface
{
public:
    enum class Kind
    {
        kSkSurface,
        kOpenClSurface,
        kUnknown
    };

    GrTargetSurface();
    GrTargetSurface(const GrTargetSurface& surface);
    GrTargetSurface(SkSurface *surface);
#ifdef COCOA_USE_OPENCL
    GrTargetSurface(::cl_mem mem);
#endif
    GrTargetSurface& operator=(const GrTargetSurface& that);

    inline Kind kind() const { return fKind; }
    inline SkSurface *asSkSurface() { return fSkSurface; }

#ifdef COCOA_USE_OPENCL
    inline ::cl_mem asClSurface() { return fClSurface; }
#endif

private:
    Kind            fKind;
    SkSurface      *fSkSurface;
#ifdef COCOA_USE_OPENCL
    ::cl_mem        fClSurface;
#endif
};

CIALLO_END_NS
#endif //COCOA_GRTARGETSURFACE_H
