#include "ciallo/DDR/GrTargetSurface.h"
CIALLO_BEGIN_NS

GrTargetSurface::GrTargetSurface()
    : fKind(Kind::kUnknown)
    , fSkSurface(nullptr)
#ifdef COCOA_USE_OPENCL
    , fClSurface(nullptr)
#endif
{}

GrTargetSurface::GrTargetSurface(const GrTargetSurface &surface)
    : fKind(surface.fKind)
    , fSkSurface(surface.fSkSurface)
#ifdef COCOA_USE_OPENCL
    , fClSurface(surface.fClSurface)
#endif
{}

GrTargetSurface::GrTargetSurface(SkSurface *surface)
    : fKind(Kind::kSkSurface)
    , fSkSurface(surface)
#ifdef COCOA_USE_OPENCL
    , fClSurface(nullptr)
#endif
{}

GrTargetSurface& GrTargetSurface::operator=(const GrTargetSurface &that)
{
    fKind = that.fKind;
    fSkSurface = that.fSkSurface;
#ifdef COCOA_USE_OPENCL
    fClSurface = that.fClSurface;
#endif
    return *this;
}

#ifdef COCOA_USE_OPENCL

GrTargetSurface::GrTargetSurface(::cl_mem mem)
    : fKind(Kind::kOpenClSurface),
      fSkSurface(nullptr),
      fClSurface(mem)
{}

#endif /* COCOA_USE_OPENCL */
CIALLO_END_NS
