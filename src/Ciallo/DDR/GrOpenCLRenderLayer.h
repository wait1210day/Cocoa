#ifndef COCOA_GROPENCLRENDERLAYER_H
#define COCOA_GROPENCLRENDERLAYER_H

#include "CL/cl.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"

#include "Ciallo/GrBase.h"
#include "Ciallo/DDR/GrBaseRenderLayer.h"
CIALLO_BEGIN_NS

class GrOpenCLRenderLayer : public GrBaseRenderLayer
{
public:
    GrOpenCLRenderLayer(int32_t x, int32_t y, int32_t z,
                        int32_t width, int32_t height,
                        const SkImageInfo& imageInfo,
                        cl_mem image,
                        cl_command_queue commandQueue);
    ~GrOpenCLRenderLayer() override;

private:
    GrLayerResult onLayerResult() override;
    SkCanvas *onCreateCanvas() override;

private:
    ::cl_command_queue      fCommandQueue;
    ::cl_mem                fDeviceImage;
    uint8_t                *fBitmapAddr;
    size_t                  fBitmapSize;
    sk_sp<SkSurface>        fSurface;
    SkImageInfo             fImageInfo;
};

CIALLO_END_NS
#endif //COCOA_GROPENCLRENDERLAYER_H
