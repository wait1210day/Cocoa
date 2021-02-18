#include <memory>

#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"

#include "core/Exception.h"
#include "ciallo/GrBase.h"
#include "ciallo/DDR/GrBaseCompositor.h"
#include "ciallo/DDR/GrCpuCompositor.h"
#include "ciallo/DDR/GrCpuRenderLayer.h"
#include "ciallo/DDR/GrBasePlatform.h"

CIALLO_BEGIN_NS

#define NULL_XID    0

std::shared_ptr<GrBaseCompositor> GrCpuCompositor::MakeDirectCpu(int32_t width,
                                                                 int32_t height,
                                                                 GrColorFormat colorFormat,
                                                                 GrBasePlatform *platform)
{
    auto ret = std::make_shared<GrCpuCompositor>(width,
                                                 height,
                                                 colorFormat,
                                                 platform);
    ret->createSurface();
    return ret;
}

GrCpuCompositor::GrCpuCompositor(int32_t width,
                                 int32_t height,
                                 GrColorFormat colorFormat,
                                 GrBasePlatform *platform)
    : GrBaseCompositor("cocoa::ciallo::GrCpuCompositor",
                       CompositeDevice::kCpuDevice,
                       width,
                       height,
                       colorFormat,
                       platform),
      fBitmapSurface(nullptr)
{
    setDriverSpecDeviceTypeInfo(CompositeDriverSpecDeviceType::kDirectCpu);
    setDeviceInfo(CIALLO_ROMAN_CPU_DRIVER_VERSION,
                  CIALLO_ROMAN_CPU_API_VERSION,
                  CIALLO_ROMAN_CPU_VENDOR,
                  CIALLO_ROMAN_CPU_DEVICE_NAME);
}

GrCpuCompositor::~GrCpuCompositor()
{
    this->Dispose();
}

void GrCpuCompositor::createSurface()
{
    SkISize size = SkISize::Make(this->width(), this->height());
    SkImageInfo imageInfo = SkImageInfo::Make(size,
                                              ToSkColorType(this->colorFormat()),
                                              SkAlphaType::kPremul_SkAlphaType);
    fBitmapSurface = SkSurface::MakeRaster(imageInfo, imageInfo.minRowBytes(), nullptr);
    if (fBitmapSurface == nullptr)
    {
        artfmt(__FUNCTION__) << "Failed to create SkSurface to render to";
        arthrow<ARTException>(new ARTException);
    }
}

GrTargetSurface GrCpuCompositor::onTargetSurface()
{
    THROWABLE_ASSERT(fBitmapSurface != nullptr);
    return GrTargetSurface(fBitmapSurface.get());
}

void GrCpuCompositor::onPresent()
{
    GrBasePlatform::ScopedAcquireBuffer scopedAcquireBuffer(getPlatform());
    SkImageInfo imageInfo = SkImageInfo::Make(SkISize::Make(this->width(), this->height()),
                                              ToSkColorType(this->colorFormat()),
                                              SkAlphaType::kPremul_SkAlphaType);
    fBitmapSurface->readPixels(imageInfo, getPlatform()->writableBuffer(),
                               imageInfo.minRowBytes(), 0, 0);
}

GrBaseRenderLayer *GrCpuCompositor::onCreateRenderLayer(int32_t width, int32_t height,
                                                        int32_t left, int32_t top,
                                                        int zindex)
{
    SkISize size = SkISize::Make(width, height);
    SkImageInfo imageInfo = SkImageInfo::Make(size,
                                              ToSkColorType(this->colorFormat()),
                                              SkAlphaType::kPremul_SkAlphaType);
    return new GrCpuRenderLayer(left, top, zindex, width, height, imageInfo);
}

CIALLO_END_NS
