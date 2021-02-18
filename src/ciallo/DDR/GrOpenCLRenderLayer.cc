#include <CL/cl.h>
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"

#include "ciallo/GrBase.h"
#include "ciallo/DDR/GrOpenCLRenderLayer.h"
CIALLO_BEGIN_NS

GrOpenCLRenderLayer::GrOpenCLRenderLayer(int32_t x, int32_t y, int32_t z,
                                         int32_t width, int32_t height,
                                         const SkImageInfo& imageInfo,
                                         cl_mem image, cl_command_queue commandQueue)
    : GrBaseRenderLayer("cocoa::ciallo::GrOpenCLRenderLayer",
                        x, y, z, width, height),
      fCommandQueue(commandQueue),
      fDeviceImage(image),
      fBitmapAddr(nullptr),
      fBitmapSize(0),
      fSurface(nullptr),
      fImageInfo(imageInfo)
{
    fBitmapSize = width * height * imageInfo.bytesPerPixel();
}

GrOpenCLRenderLayer::~GrOpenCLRenderLayer()
{
    /* "if" is unnecessary, deleting a null-pointer has no effect */
    delete[] fBitmapAddr;
    if (fDeviceImage)
        ::clReleaseMemObject(fDeviceImage);
}

SkCanvas *GrOpenCLRenderLayer::onCreateCanvas()
{
    if (fSurface != nullptr)
        return fSurface->getCanvas();

    fBitmapAddr = new uint8_t[fBitmapSize];
    fSurface = SkSurface::MakeRasterDirect(fImageInfo, fBitmapAddr,
                                           this->width() * fImageInfo.bytesPerPixel());
    if (fSurface == nullptr)
    {
        artfmt(__FUNCTION__) << "Failed to create SkSurface";
        arthrow<ARTException>(new ARTException);
    }
    return fSurface->getCanvas();
}

GrLayerResult GrOpenCLRenderLayer::onLayerResult()
{
    if (fBitmapAddr == nullptr)
    {
        artfmt(__FUNCTION__) << "Nothing was rendered on this layer";
        arthrow<ARTException>(new ARTException);
    }

    SkIRect boundary = dirtyBoundary();
    size_t origin[3];
    size_t region[3];
    origin[0] = boundary.left();
    origin[1] = boundary.top();
    origin[2] = 0;
    region[0] = boundary.width();
    region[1] = boundary.height();
    region[2] = 1;

    ::cl_int ret = ::clEnqueueWriteImage(fCommandQueue,
                                         fDeviceImage,
                                         CL_TRUE,
                                         origin,
                                         region,
                                         this->width() * fImageInfo.bytesPerPixel(),
                                         0,
                                         fBitmapAddr,
                                         0,
                                         nullptr,
                                         nullptr);
    if (ret != CL_SUCCESS)
    {
        artfmt(__FUNCTION__) << "Failed to invoke clEnqueueWriteImage";
        arthrow<ARTException>(new ARTException);
    }
    return GrLayerResult(fDeviceImage);
}

CIALLO_END_NS
