#ifndef COCOA_GRCPUCOMPOSITOR_H
#define COCOA_GRCPUCOMPOSITOR_H

#include <memory>

#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkBitmap.h"

#include "Ciallo/GrBase.h"
#include "Ciallo/DDR/GrBaseCompositor.h"
CIALLO_BEGIN_NS

#define CIALLO_ROMAN_CPU_DRIVER_VERSION     210123
#define CIALLO_ROMAN_CPU_API_VERSION        210123
#define CIALLO_ROMAN_CPU_VENDOR             0
#define CIALLO_ROMAN_CPU_DEVICE_NAME        "Cocoa/Ciallo Roman CPU Renderer"

class GrCpuCompositor : public GrBaseCompositor
{
public:
    GrCpuCompositor(int32_t width,
                    int32_t height,
                    GrColorFormat colorFormat,
                    GrBasePlatform *platform);
    ~GrCpuCompositor();

    static std::shared_ptr<GrBaseCompositor> MakeDirectCpu(int32_t width,
                                                           int32_t height,
                                                           GrColorFormat colorFormat,
                                                           GrBasePlatform *platform);

private:
    GrTargetSurface onTargetSurface() override;
    void onPresent() override;
    GrBaseRenderLayer *onCreateRenderLayer(int32_t width,
                                           int32_t height,
                                           int32_t left,
                                           int32_t top,
                                           int zindex) override;

    void createSurface();

private:
    sk_sp<SkSurface>             fBitmapSurface;
};

CIALLO_END_NS
#endif //COCOA_GRCPUCOMPOSITOR_H
