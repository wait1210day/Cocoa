#ifndef COCOA_GRLAYERRESULT_H
#define COCOA_GRLAYERRESULT_H

#ifdef COCOA_USE_OPENCL
#include <CL/cl.h>
#endif /* COCOA_USE_OPENCL */
#include "include/core/SkImage.h"

#include "ciallo/GrBase.h"
CIALLO_BEGIN_NS

class GrLayerResult
{
public:
    enum class Kind
    {
        kGpuImage,
        kCpuImage,
        kOpenClImage,
        kUnknown
    };

    GrLayerResult();
    GrLayerResult(const GrLayerResult& that);
    GrLayerResult(GrLayerResult&& that) noexcept;
    explicit GrLayerResult(const sk_sp<SkImage>& image);
#ifdef COCOA_USE_OPENCL
    explicit GrLayerResult(::cl_mem memory);
#endif
    GrLayerResult& operator=(const GrLayerResult& that);

    ~GrLayerResult() = default;

    [[nodiscard]] inline bool valid() const
    { return fImage != nullptr || fOpenClMemory != nullptr; }

    void reset(sk_sp<SkImage> image);
#ifdef COCOA_USE_OPENCL
    void reset(::cl_mem memory);
#endif

    inline sk_sp<SkImage> asImage() { return fImage; }
#ifdef COCOA_USE_OPENCL
    inline ::cl_mem  asOpenCLImage() { return fOpenClMemory; }
#endif

    inline void disposeImage() { fImage = nullptr; }
    [[nodiscard]] inline Kind kind() const { return fKind; }

private:
    Kind            fKind;
    sk_sp<SkImage>  fImage;
#ifdef COCOA_USE_OPENCL
    ::cl_mem        fOpenClMemory;
#endif
};

CIALLO_END_NS
#endif //COCOA_GRLAYERRESULT_H
