#include <memory>

#include "ciallo/GrBase.h"
#include "ciallo/GrGpe.h"
#include "ciallo/DDR/GrBasePlatform.h"
#include "ciallo/DDR/GrBaseCompositor.h"

CIALLO_BEGIN_NS

GrBasePlatform::GrBasePlatform(const std::string& inherit,
                               GrPlatformKind kind,
                               const GrPlatformOptions& opts)
    : Throwable(inherit.c_str()),
      fCompositor(nullptr),
      fKind(kind),
      fOptions(opts)
{
    QObject::signal<void*, void*>(this, GPE_QS_DDR_BEFORE_SUBMIT);
    QObject::signal<void*, void*>(this, GPE_QS_DDR_AFTER_SUBMIT);
    QObject::signal<>(this, GPE_QS_DDR_BEFORE_PRESENT);
    QObject::signal<>(this, GPE_QS_DDR_AFTER_PRESENT);
    QObject::signal<int32_t, int32_t, int32_t, int32_t, int>(this, GPE_QS_DDR_BEFORE_NEW_LAYER);
    QObject::signal<int32_t, int32_t, int32_t, int32_t, int>(this, GPE_QS_DDR_AFTER_NEW_LAYER);
    QObject::signal<>(this, GPE_QS_DDR_BEFORE_ELEVATE);
    QObject::signal<>(this, GPE_QS_DDR_AFTER_ELEVATE);
    QObject::signal<>(this, GPE_QS_DDR_BEFORE_DEPRESS);
    QObject::signal<>(this, GPE_QS_DDR_AFTER_DEPRESS);
    QObject::signal<int32_t, int32_t>(this, GPE_QS_DDR_BEFORE_MOVETO);
    QObject::signal<int32_t, int32_t>(this, GPE_QS_DDR_AFTER_MOVETO);
    QObject::signal<>(this, GPE_QS_DDR_BEFORE_UPDATE);
    QObject::signal<>(this, GPE_QS_DDR_AFTER_UPDATE);
    QObject::signal<void*, int32_t, int32_t, void*>(this, GPE_QS_DDR_BEFORE_PAINT);
    QObject::signal<void*, int32_t, int32_t, void*>(this, GPE_QS_DDR_AFTER_PAINT);
}

std::shared_ptr<GrBaseCompositor> GrBasePlatform::compositor()
{
    if (fCompositor == nullptr)
        fCompositor = onCreateCompositor();
    return fCompositor;
}

void GrBasePlatform::expose()
{
    return this->onExpose();
}

uint8_t *GrBasePlatform::writableBuffer()
{
    return this->onWritableBuffer();
}

CIALLO_END_NS
