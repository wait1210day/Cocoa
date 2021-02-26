#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImageInfo.h"

#include "Ciallo/GrBase.h"
#include "Ciallo/DDR/GrCpuRenderLayer.h"
CIALLO_BEGIN_NS

GrCpuRenderLayer::GrCpuRenderLayer(int32_t x,
                                   int32_t y,
                                   int32_t z,
                                   int32_t w,
                                   int32_t h,
                                   const SkImageInfo &imageInfo)
    : GrBaseRenderLayer(x, y, z, w, h),
      fImageInfo(imageInfo),
      fSurface(nullptr)
{
}

GrLayerResult GrCpuRenderLayer::onLayerResult()
{
    RUNTIME_EXCEPTION_ASSERT(fSurface != nullptr);
    return GrLayerResult(fSurface->makeImageSnapshot());
}

SkCanvas *GrCpuRenderLayer::onCreateCanvas()
{
    if (fSurface != nullptr)
        return fSurface->getCanvas();
    fSurface = SkSurface::MakeRaster(fImageInfo, nullptr);
    if (fSurface == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to create SkSurface to render to")
                .make<RuntimeException>();
    }

    return fSurface->getCanvas();
}

CIALLO_END_NS
