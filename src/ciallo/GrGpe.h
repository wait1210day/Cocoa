#ifndef __GR_PIPELINE_EXTENSION__
#define __GR_PIPELINE_EXTENSION__

#include <string>
#include <memory>
#include <ostream>

#include "include/core/SkImage.h"
#include "include/core/SkRect.h"

#include "core/QObject.h"
#include "core/Exception.h"
#include "core/Journal.h"
#include "ciallo/GrBase.h"
#include "ciallo/DDR/GrBaseCompositor.h"
#include "ciallo/DDR/GrBaseRenderLayer.h"
CIALLO_BEGIN_NS

#define GPE_CURRENT_API_VERSION     210117

enum class GrGpeKind
{
    kLayoutEngine,
    kRenderTree,
    kCompositor,
    kRenderLayer
};
StreamHolder& operator<<(StreamHolder& os, GrGpeKind kind);

#define GPE_QS_DDR_BEFORE_SUBMIT       "#DDR-ext-before-submit"
#define GPE_QS_DDR_AFTER_SUBMIT        "#DDR-ext-after-submit"
#define GPE_QS_DDR_BEFORE_PRESENT      "#DDR-ext-before-present"
#define GPE_QS_DDR_AFTER_PRESENT       "#DDR-ext-after-present"
#define GPE_QS_DDR_BEFORE_NEW_LAYER    "#DDR-ext-before-new-layer"
#define GPE_QS_DDR_AFTER_NEW_LAYER     "#DDR-ext-after-new-layer"
#define GPE_QS_DDR_BEFORE_ELEVATE      "#DDR-ext-before-elevate"
#define GPE_QS_DDR_AFTER_ELEVATE       "#DDR-ext-after-elevate"
#define GPE_QS_DDR_BEFORE_DEPRESS      "#DDR-ext-before-depress"
#define GPE_QS_DDR_AFTER_DEPRESS       "#DDR-ext-after-depress"
#define GPE_QS_DDR_BEFORE_MOVETO       "#DDR-ext-before-moveto"
#define GPE_QS_DDR_AFTER_MOVETO        "#DDR-ext-after-moveto"
#define GPE_QS_DDR_BEFORE_UPDATE       "#DDR-ext-before-update"
#define GPE_QS_DDR_AFTER_UPDATE        "#DDR-ext-after-update"
#define GPE_QS_DDR_BEFORE_PAINT        "#DDR-ext-before-paint"
#define GPE_QS_DDR_AFTER_PAINT         "#DDR-ext-after-paint"

struct GrGpeLoaderInfo
{
    std::string     fName;
    std::string     fDescription;
    uint32_t        fApiVersion;
};

/**
 * @class GrGpe
 * 
 * Graphics Pipeline Extension is an optional extension to the
 * Ciallo engine. Specific extensions can be inserted into a
 * process in the 2D rendering pipeline, and the corresponding
 * code will be invoked at the appropriate time.
 */
class GrGpe : public QObject,
              public Throwable
{
public:
    GrGpe(const std::string &path, const std::string &name, GrGpeKind kind);
    virtual ~GrGpe();

    inline std::string name() const     { return fName; }
    inline std::string filePath() const { return fPath; }
    inline GrGpeKind kind() const       { return fKind; }
    
    void attach(const std::shared_ptr<QObject>& object);
    void detach();

    template<IsQObjectChild T>
    inline std::shared_ptr<T> attachment_cast() {
        return std::shared_ptr<T>(fAttachment);
    }

protected:
    /* Compositor extension interfaces */
    virtual void slotBeforeSubmit(GrBaseRenderLayer *layer,
                                  const SkIRect *clip) qobj_slot;

    virtual void slotAfterSubmit(GrBaseRenderLayer *layer,
                                 const SkIRect *clip) qobj_slot;
    
    virtual void slotBeforePresent() qobj_slot;
    virtual void slotAfterPresent() qobj_slot;

    virtual void slotBeforeNewLayer(int32_t width,
                                    int32_t height,
                                    int32_t left,
                                    int32_t top,
                                    int zindex) qobj_slot;

    virtual void slotAfterNewLayer(int32_t width,
                                   int32_t height,
                                   int32_t left,
                                   int32_t top,
                                   int zindex) qobj_slot;

    /* RenderLayer extension interfaces */
    virtual void slotBeforeElevate() qobj_slot;
    virtual void slotAfterElevate() qobj_slot;

    virtual void slotBeforeDepress() qobj_slot;
    virtual void slotAfterDepress() qobj_slot;

    virtual void slotBeforeMoveTo(int32_t px, int32_t py) qobj_slot;
    virtual void slotAfterMoveTo(int32_t px, int32_t py) qobj_slot;

    virtual void slotBeforeUpdate() qobj_slot;
    virtual void slotAfterUpdate() qobj_slot;

    virtual void slotBeforePaint(const SkPicture *picture,
                                 int32_t left,
                                 int32_t top,
                                 const SkPaint *paint) qobj_slot;
    virtual void slotAfterPaint(const SkPicture *picture,
                                int32_t left,
                                int32_t top,
                                const SkPaint *paint) qobj_slot;

private:
    std::string                 fPath;
    std::string                 fName;
    GrGpeKind                   fKind;
    std::shared_ptr<QObject>    fAttachment;
};

CIALLO_END_NS
#endif /* __GR_PIPELINE_EXTENSION__ */
