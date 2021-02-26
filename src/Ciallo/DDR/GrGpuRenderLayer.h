#ifndef __GPU_RENDERLAYER_H__
#define __GPU_RENDERLAYER_H__

#include <memory>

#include "include/gpu/GrDirectContext.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImage.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"

#include "Ciallo/DDR/GrBaseRenderLayer.h"

CIALLO_BEGIN_NS

class GrGpuRenderLayer : public GrBaseRenderLayer
{
public:
    ~GrGpuRenderLayer() override;
    GrGpuRenderLayer(GrDirectContext *ctx,
                   int32_t x,
                   int32_t y,
                   int32_t z,
                   const SkImageInfo &imageInfo);

private:
    SkCanvas *onCreateCanvas() override;
    GrLayerResult onLayerResult() override;

    void createSurface();

private:
    GrDirectContext     *fDirectContext;
    SkImageInfo          fImageInfo;
    sk_sp<SkSurface>     fSurface;
};

CIALLO_END_NS

#endif // __GPU_RENDERLAYER_H__
