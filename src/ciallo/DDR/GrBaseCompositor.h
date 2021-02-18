#ifndef __BASE_COMPOSITOR_H__
#define __BASE_COMPOSITOR_H__

#include <string>
#include <memory>
#include <map>
#include <mutex>

#ifdef COCOA_USE_OPENCL
#include <CL/cl.h>
#endif
#include "include/core/SkSurface.h"
#include "include/core/SkImage.h"
#include "include/core/SkRect.h"
#include "include/gpu/GrDirectContext.h"

#include "ciallo/GrBase.h"
#include "ciallo/DDR/GrTargetSurface.h"
#include "ciallo/DDR/GrBaseRenderLayer.h"
#include "ciallo/DDR/GrLayerResult.h"

CIALLO_BEGIN_NS

class GrBasePlatform;

enum class CompositeDevice
{
    kCpuDevice,
    kGpuVulkan,
    kOpenCL
};

enum class CompositeDriverSpecDeviceType
{
    kDirectCpu,

    kOpenCL_Cpu,
    kOpenCL_Gpu,
    kOpenCL_Accelerator,
    kOpenCL_Default,
    kOpenCL_Custom,
    kOpenCL_All,
    kOpenCL_Other,

    kVulkan_Other,
    kVulkan_IntergratedGpu,
    kVulkan_DiscreteGpu,
    kVulkan_VirtualGpu,
    kVulkan_Cpu
};

enum class GrColorFormat
{
    kColor_RGBA_8888,
    kColor_BGRA_8888
};

struct CompositeBackendInfo
{
    CompositeDevice                 fDeviceType;
    CompositeDriverSpecDeviceType   fDriverSpecDeviceType;
    uint32_t                        fDriverVersion;
    uint32_t                        fAPIVersion;
    uint32_t                        fDeviceVendor;
    std::string                     fDeviceName;
    std::vector<std::string>        fGpuExtensions;
    std::vector<std::string>        fNativeExtensions;
};

struct LayerBinder
{
    GrBaseRenderLayer *fHandle = nullptr;
    SkIRect          fSubmittedClip;
    GrLayerResult    fSubmittedImage;
    int64_t          fDroppedFrames = 0;
};

class GrBaseCompositor : public std::enable_shared_from_this<GrBaseCompositor>,
                         public Throwable,
                         public QObject
{
public:
    using RenderLayerID = uint32_t;
    using RenderLayerIteator = std::map<int, RenderLayerID>::iterator;

    ~GrBaseCompositor() override;

    inline CompositeDevice getDeviceType() const
                                            { return fBackendInfo.fDeviceType; }
    inline CompositeDriverSpecDeviceType getDetailedDeviceType() const
                                            { return fBackendInfo.fDriverSpecDeviceType; }

    inline uint32_t getDriverVersion() const       { return fBackendInfo.fDriverVersion; }
    inline uint32_t getAPIVersion() const          { return fBackendInfo.fAPIVersion; }
    inline uint32_t getDeviceVendor() const        { return fBackendInfo.fDeviceVendor; }
    inline std::string getDeviceName() const       { return fBackendInfo.fDeviceName; }

    inline const std::vector<std::string>& getGpuExtensions() const
                                            { return fBackendInfo.fGpuExtensions; }
    inline const std::vector<std::string>& getNativeExtensions() const
                                            { return fBackendInfo.fNativeExtensions; }

    inline int32_t width()  const { return fWidth; }
    inline int32_t height() const { return fHeight; }

    inline GrColorFormat colorFormat() const       { return fColorFormat; }

    void submit(GrBaseRenderLayer *who, const GrLayerResult& result, const SkIRect& clipRect);

    /**
     * @brief Presents current frame.
     * 
     * Rasterizer submits the rendered texture to its own compositor,
     * which generates a new frame. But the new frame is not actually
     * displayed on the screen. Call GrBaseCompositor::present()
     * to display it on the screen, then compositor moves to next frame
     * after calling.
     * Note that even if the rasterizer submits textures multiple times,
     * they will not be displayed until present is called. The content of
     * the final frame is the last texture submitted.
     */
    void present();

    /**
     * @brief Creates a new render layer with a rasterizer.
     * @param width: Width of new layer.
     * @param height: Height of new layer.
     * @param left: The coordinate of the upper-left corner of the rectangle
     *            where the layer is located.
     * @param top: See above.
     * @param zindex: The stacking order of layer. The lower layer has
     *                lower value.
    */
    std::shared_ptr<GrBaseRenderLayer> newRenderLayer(int32_t width,
                                                      int32_t height,
                                                      int32_t left,
                                                      int32_t top,
                                                      int zindex);

    /* Swaps two layers by their Z-index value */
    void swapRenderLayers(int a, int b);

    RenderLayerIteator begin();
    RenderLayerIteator end();

protected:
    GrBaseCompositor(const std::string& inherit,
                     CompositeDevice device,
                     int32_t width,
                     int32_t height,
                     GrColorFormat colorFormat,
                     GrBasePlatform *platform);

    void setDriverSpecDeviceTypeInfo(CompositeDriverSpecDeviceType type);
    void setDeviceInfo(uint32_t driverVersion,
                       uint32_t APIVersion,
                       uint32_t vendor,
                       const std::string& device);
    void appendGpuExtensionsInfo(const std::string& ext);
    void appendNativeExtensionsInfo(const std::string& ext);

    static void skComposite(SkSurface *target, const sk_sp<SkImage>& image,
                            const SkRect& srcClip, const SkRect& dstClip);
#ifdef COCOA_USE_OPENCL
    virtual void clComposite(::cl_mem target, ::cl_mem image,
                             const SkRect& srcClip, const SkRect& dstClip);
#endif

    virtual GrTargetSurface onTargetSurface() = 0;
    virtual void onPresent() = 0;
    virtual GrBaseRenderLayer *onCreateRenderLayer(int32_t width,
                                                   int32_t height,
                                                   int32_t left,
                                                   int32_t top,
                                                   int zindex) = 0;

    inline GrBasePlatform *getPlatform() { return fPlatform; }

    /* Call this before inherit class destructing */
    void Dispose();

    static SkColorType ToSkColorType(GrColorFormat colorFormat);

private:
    int32_t                         fWidth;
    int32_t                         fHeight;
    GrColorFormat                   fColorFormat;
    CompositeBackendInfo            fBackendInfo;
    std::map<int, RenderLayerID>    fLayerIDMap;
    std::vector<LayerBinder>        fLayers;
    GrBasePlatform                 *fPlatform;
};

CIALLO_END_NS

#endif
