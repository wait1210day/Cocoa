#ifndef __GR_BASE_PLATFORM__
#define __GR_BASE_PLATFORM__

#include <memory>
#include <vector>
#include <mutex>

#include "core/Exception.h"
#include "core/QObject.h"
#include "ciallo/GrBase.h"
#include "ciallo/DDR/GrBaseCompositor.h"
CIALLO_BEGIN_NS

enum class GrPlatformKind
{
    kXcb
};

struct GrPlatformOptions
{
    bool use_gpu_accel = true;
    bool use_opencl_accel = false;
    /**
     * If @a use_strict_accel is false, the platform is supposed
     * to create a CPU rendering compositor when fail to create
     * a accelerated compositor though you've specified use_*_accel
     * option.
     */
    bool use_strict_accel = false;

    /**
     * If use_gpu_accel is true, following options will be used.
     */
    bool vulkan_debug = false;
    std::vector<std::string> vulkan_required_instance_ext;
    std::vector<std::string> vulkan_required_device_ext;

    /**
     * If use_opencl_accel is true, following options will be used.
     */
     std::string opencl_platform_keyword;
     std::string opencl_device_keyword;
};

class GrBasePlatform : public QObject, public Throwable
{
public:
    class ScopedAcquireBuffer
    {
    public:
        explicit ScopedAcquireBuffer(GrBasePlatform *platform)
            : fPlatform(platform) { fPlatform->acquireBuffer(); }
        ~ScopedAcquireBuffer()
        { fPlatform->releaseBuffer(); }

    private:
        GrBasePlatform  *fPlatform;
    };

    ~GrBasePlatform() override = default;

    /**
     * @brief Gets a compositor, maybe nullptr.
     */
    std::shared_ptr<GrBaseCompositor> compositor();

    inline GrPlatformKind kind() const              { return fKind; }
    inline const GrPlatformOptions& options() const { return fOptions; }

    bool tryAcquireBuffer()
    { return fBufferMutex.try_lock(); }

    void acquireBuffer()
    { fBufferMutex.lock(); }

    void releaseBuffer()
    { fBufferMutex.unlock(); }

    uint8_t *writableBuffer();
    void expose();

protected:
    GrBasePlatform(const std::string& inherit,
                   GrPlatformKind kind,
                   const GrPlatformOptions& opts);

    inline GrPlatformOptions& writableOptions()  { return fOptions; }
    virtual std::shared_ptr<GrBaseCompositor> onCreateCompositor() = 0;

    virtual uint8_t *onWritableBuffer() = 0;
    virtual void onExpose() = 0;

private:
    std::shared_ptr<GrBaseCompositor>   fCompositor;
    GrPlatformKind                      fKind;
    GrPlatformOptions                   fOptions;
    std::mutex                          fBufferMutex;
};

CIALLO_END_NS
#endif /* __GR_BASE_PLATFORM__ */
