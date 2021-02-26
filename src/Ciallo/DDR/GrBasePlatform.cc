#include <memory>

#include "Ciallo/GrBase.h"
#include "Ciallo/DDR/GrBasePlatform.h"
#include "Ciallo/DDR/GrBaseCompositor.h"

CIALLO_BEGIN_NS

GrBasePlatform::GrBasePlatform(GrPlatformKind kind,
                               const GrPlatformOptions& opts)
    : fCompositor(nullptr),
      fKind(kind),
      fOptions(opts)
{
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
