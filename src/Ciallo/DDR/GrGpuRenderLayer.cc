#include "Ciallo/DDR/GrGpuRenderLayer.h"

CIALLO_BEGIN_NS

GrGpuRenderLayer::~GrGpuRenderLayer()
{
    /* sk_sp<...> will free all the resources automatically,
       we have nothing special to do. */
}

GrGpuRenderLayer::GrGpuRenderLayer(GrDirectContext *ctx,
                               int32_t x,
                               int32_t y,
                               int32_t z,
                               const SkImageInfo &imageInfo)
    : GrBaseRenderLayer(x, y, z,
                        imageInfo.width(), imageInfo.height()),
      fDirectContext(ctx),
      fImageInfo(imageInfo)
{
}

SkCanvas *GrGpuRenderLayer::onCreateCanvas()
{
    if (fSurface == nullptr)
        createSurface();
    return fSurface->getCanvas();
}

GrLayerResult GrGpuRenderLayer::onLayerResult()
{
    // implicit cast (sk_sp<SkImage> => GrLayerResult)
    return GrLayerResult(fSurface->makeImageSnapshot());
}

void GrGpuRenderLayer::createSurface()
{
    fSurface = SkSurface::MakeRenderTarget(fDirectContext,
                                           SkBudgeted::kNo,
                                           fImageInfo,
                                           4,
                                           GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
                                           nullptr);
    
    RUNTIME_EXCEPTION_ASSERT(fSurface != nullptr);
}

CIALLO_END_NS
