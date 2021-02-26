#ifndef COCOA_GRCPURENDERLAYER_H
#define COCOA_GRCPURENDERLAYER_H

#include <memory>

#include "include/core/SkImageInfo.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"

#include "Ciallo/GrBase.h"
#include "Ciallo/DDR/GrBaseRenderLayer.h"
#include "Ciallo/DDR/GrLayerResult.h"
CIALLO_BEGIN_NS

class GrCpuRenderLayer : public GrBaseRenderLayer
{
public:
    GrCpuRenderLayer(int32_t x,
                     int32_t y,
                     int32_t z,
                     int32_t w,
                     int32_t h,
                     const SkImageInfo& imageInfo);
    ~GrCpuRenderLayer() override = default;

private:
    GrLayerResult onLayerResult() override;
    SkCanvas *onCreateCanvas() override;

private:
    SkImageInfo                 fImageInfo;
    sk_sp<SkSurface>            fSurface;
};

CIALLO_END_NS
#endif //COCOA_GRCPURENDERLAYER_H
