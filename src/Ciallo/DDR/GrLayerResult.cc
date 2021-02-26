#ifdef COCOA_USE_OPENCL
#include <CL/cl.h>
#endif
#include "include/core/SkImage.h"

#include "Ciallo/GrBase.h"
#include "Ciallo/DDR/GrLayerResult.h"
CIALLO_BEGIN_NS

GrLayerResult::GrLayerResult(const sk_sp<SkImage>& image)
    : fImage(image)
#ifdef COCOA_USE_OPENCL
    , fOpenClMemory(nullptr)
#endif
{
    if (image != nullptr)
    {
        if (image->isTextureBacked())
            fKind = Kind::kGpuImage;
        else
            fKind = Kind::kCpuImage;
    }
}

GrLayerResult::GrLayerResult(const GrLayerResult &that)
    : fKind(that.fKind)
    , fImage(that.fImage)
#ifdef COCOA_USE_OPENCL
    , fOpenClMemory(that.fOpenClMemory)
#endif
{
}

GrLayerResult::GrLayerResult()
    : fKind(Kind::kUnknown)
    , fImage(nullptr)
#ifdef COCOA_USE_OPENCL
    , fOpenClMemory(nullptr)
#endif
{
}

GrLayerResult::GrLayerResult(GrLayerResult &&that) noexcept
    : fKind(that.fKind)
    , fImage(std::move(that.fImage))
#ifdef COCOA_USE_OPENCL
    , fOpenClMemory(that.fOpenClMemory)
#endif
{
#ifdef COCOA_USE_OPENCL
    that.fOpenClMemory = nullptr;
#endif
    that.fImage = nullptr;
}

void GrLayerResult::reset(sk_sp<SkImage> image)
{
    fImage = image;
#ifdef COCOA_USE_OPENCL
    fOpenClMemory = nullptr;
#endif
    if (image != nullptr)
    {
        if (image->isTextureBacked())
            fKind = Kind::kGpuImage;
        else
            fKind = Kind::kCpuImage;
    }
}

GrLayerResult& GrLayerResult::operator=(const GrLayerResult &that)
{
    fKind = that.fKind;
    fImage = that.fImage;
#ifdef COCOA_USE_OPENCL
    fOpenClMemory = that.fOpenClMemory;
#endif
    return *this;
}

#ifdef COCOA_USE_OPENCL

GrLayerResult::GrLayerResult(::cl_mem memory)
    : fKind(Kind::kOpenClImage),
      fImage(nullptr),
      fOpenClMemory(memory)
{
}

void GrLayerResult::reset(::cl_mem memory)
{
    fKind = Kind::kOpenClImage;
    fImage = nullptr;
    fOpenClMemory = memory;
}

#endif /* COCOA_USE_OPENCL */
CIALLO_END_NS
