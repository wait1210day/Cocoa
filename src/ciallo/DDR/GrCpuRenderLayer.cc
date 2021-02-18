#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImageInfo.h"

#include "ciallo/GrBase.h"
#include "ciallo/DDR/GrCpuRenderLayer.h"
CIALLO_BEGIN_NS

GrCpuRenderLayer::GrCpuRenderLayer(int32_t x,
                                   int32_t y,
                                   int32_t z,
                                   int32_t w,
                                   int32_t h,
                                   const SkImageInfo &imageInfo)
    : GrBaseRenderLayer("cocoa::ciallo::GrCpuRenderLayer",
                        x, y, z, w, h),
      fImageInfo(imageInfo),
      fSurface(nullptr)
{
}

GrLayerResult GrCpuRenderLayer::onLayerResult()
{
    THROWABLE_ASSERT(fSurface != nullptr);
    return GrLayerResult(fSurface->makeImageSnapshot());
}

SkCanvas *GrCpuRenderLayer::onCreateCanvas()
{
    if (fSurface != nullptr)
        return fSurface->getCanvas();
    fSurface = SkSurface::MakeRaster(fImageInfo, nullptr);
    if (fSurface == nullptr)
    {
        artfmt(__FUNCTION__) << "Failed to create SkSurface to render to";
        arthrow<ARTException>(new ARTException);
    }

    return fSurface->getCanvas();
}

CIALLO_END_NS
