#include "Ciallo/DDR/GrBaseCompositor.h"

CIALLO_BEGIN_NS

GrBaseCompositor::GrBaseCompositor(CompositeDevice device,
                                   int32_t width, int32_t height,
                                   GrColorFormat colorFormat,
                                   GrBasePlatform *platform)
    : fWidth(width),
      fHeight(height),
      fColorFormat(colorFormat),
      fPlatform(platform)
{
    fBackendInfo.fDeviceType = device;
}

GrBaseCompositor::~GrBaseCompositor() = default;

void GrBaseCompositor::setDriverSpecDeviceTypeInfo(CompositeDriverSpecDeviceType type)
{
    fBackendInfo.fDriverSpecDeviceType = type;
}

void GrBaseCompositor::setDeviceInfo(uint32_t driverVersion,
                                   uint32_t APIVersion,
                                   uint32_t vendor,
                                   const std::string &device)
{
    fBackendInfo.fDriverVersion = driverVersion;
    fBackendInfo.fAPIVersion = APIVersion;
    fBackendInfo.fDeviceVendor = vendor;
    fBackendInfo.fDeviceName = device;
}

void GrBaseCompositor::appendGpuExtensionsInfo(const std::string& ext)
{
    fBackendInfo.fGpuExtensions.push_back(ext);
}

void GrBaseCompositor::appendNativeExtensionsInfo(const std::string& ext)
{
    fBackendInfo.fNativeExtensions.push_back(ext);
}

std::shared_ptr<GrBaseRenderLayer> GrBaseCompositor::newRenderLayer(int32_t width,
                                                                int32_t height,
                                                                int32_t left,
                                                                int32_t top,
                                                                int zindex)
{
    if (fLayerIDMap.contains(zindex))
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("ZIndex ")
                .append(zindex)
                .append(" is already being used")
                .make<RuntimeException>();
    }

    if (width > fWidth || height > fHeight)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Invalid layer size (")
                .append(width)
                .append("x")
                .append(height)
                .append(")")
                .make<RuntimeException>();
    }

    std::shared_ptr<GrBaseRenderLayer> layer(onCreateRenderLayer(
        width,
        height,
        left,
        top,
        zindex
    ));

    if (layer == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Couldn\'t create a new layer")
                .make<RuntimeException>();
    }
    layer->setCompositor(shared_from_this());

    RenderLayerID layerID;
    LayerBinder layerBinder;
    layerBinder.fHandle = layer.get();

    bool found = false;
    for (size_t i = 0; i < fLayers.size(); i++)
    {
        if (fLayers[i].fHandle == nullptr)
        {
            layerID = i;
            found = true;
        }
    }
    if (!found)
    {
        layerID = fLayers.size();
        fLayers.push_back(layerBinder);
    }
    else
    {
        fLayers[layerID] = layerBinder;
    }
    fLayerIDMap[zindex] = layerID;

    return layer;
}

void GrBaseCompositor::swapRenderLayers(int a, int b)
{
    RUNTIME_EXCEPTION_ASSERT(fLayerIDMap.contains(a));
    RUNTIME_EXCEPTION_ASSERT(fLayerIDMap.contains(b));

    RenderLayerID& ra = fLayerIDMap[a];
    RenderLayerID& rb = fLayerIDMap[b];

    fLayers[ra].fHandle->setZIndex(b);
    fLayers[rb].fHandle->setZIndex(a);

    RenderLayerID tmp = ra;
    ra = rb;
    rb = tmp;
}

GrBaseCompositor::RenderLayerIteator GrBaseCompositor::begin()
{
    return fLayerIDMap.begin();
}

GrBaseCompositor::RenderLayerIteator GrBaseCompositor::end()
{
    return fLayerIDMap.end();
}

void GrBaseCompositor::submit(GrBaseRenderLayer *who, const GrLayerResult& result, const SkIRect& clipRect)
{
    RUNTIME_EXCEPTION_ASSERT(fLayerIDMap.contains(who->zindex()));

    RenderLayerID layerID = fLayerIDMap[who->zindex()];
    LayerBinder& binder = fLayers[layerID];
    RUNTIME_EXCEPTION_ASSERT(binder.fHandle != nullptr);

    if (binder.fSubmittedImage.valid())
        binder.fDroppedFrames++;
    binder.fSubmittedImage = result;
    binder.fSubmittedClip = clipRect;
}

void GrBaseCompositor::present()
{
    GrTargetSurface target = onTargetSurface();

    for (auto& layerIDPair : fLayerIDMap)
    {
        LayerBinder& binder = fLayers[layerIDPair.second];
        if (binder.fHandle == nullptr)
            continue;
        if (!binder.fHandle->visible()
            || !binder.fSubmittedImage.valid())
            continue;

        SkRect srcClip = SkRect::MakeLTRB(binder.fSubmittedClip.left(),
                                          binder.fSubmittedClip.top(),
                                          binder.fSubmittedClip.right(),
                                          binder.fSubmittedClip.bottom());

        SkRect dstClip = SkRect::MakeLTRB(binder.fHandle->left() + srcClip.left(),
                                          binder.fHandle->top() + srcClip.top(),
                                          binder.fHandle->left() + srcClip.right(),
                                          binder.fHandle->top() + srcClip.bottom());

        if (target.kind() == GrTargetSurface::Kind::kSkSurface)
        {
            skComposite(target.asSkSurface(),
                        binder.fSubmittedImage.asImage(),
                        srcClip, dstClip);
        }
#ifdef COCOA_USE_OPENCL
        else if (target.kind() == GrTargetSurface::Kind::kOpenClSurface)
        {
            clComposite(target.asClSurface(),
                        binder.fSubmittedImage.asOpenCLImage(),
                        srcClip, dstClip);
        }
#endif /* COCOA_USE_OPENCL */
        else
        {
            throw RuntimeException::Builder(__FUNCTION__)
                    .append("Invalid kind of target surface")
                    .make<RuntimeException>();
        }
    }
    onPresent();
}

void GrBaseCompositor::skComposite(SkSurface *target, const sk_sp<SkImage> &image,
                                   const SkRect &srcClip, const SkRect &dstClip)
{
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrcOver);
    target->getCanvas()->drawImageRect(image, srcClip, dstClip,
                                       &paint, SkCanvas::kStrict_SrcRectConstraint);
}

#ifdef COCOA_USE_OPENCL
void GrBaseCompositor::clComposite(::cl_mem target, ::cl_mem image,
                                   const SkRect &srcClip, const SkRect &dstClip)
{
    throw RuntimeException::Builder(__FUNCTION__)
            .append("Not implemented yet")
            .make<RuntimeException>();
}
#endif /* COCOA_USE_OPENCL */

void GrBaseCompositor::Dispose()
{
    for (auto& id : fLayerIDMap)
    {
        LayerBinder& binder = fLayers[id.second];
        if (binder.fSubmittedImage.valid())
            binder.fSubmittedImage.disposeImage();
    }
}

SkColorType GrBaseCompositor::ToSkColorType(GrColorFormat colorFormat)
{
    switch (colorFormat)
    {
    case GrColorFormat::kColor_BGRA_8888:
        return SkColorType::kBGRA_8888_SkColorType;
    case GrColorFormat::kColor_RGBA_8888:
        return SkColorType::kRGBA_8888_SkColorType;
    }
}

CIALLO_END_NS
