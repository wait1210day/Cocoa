#include <memory>
#include <ostream>

#include "core/Exception.h"
#include "core/QObject.h"
#include "core/Journal.h"
#include "ciallo/GrGpe.h"

#include "include/core/SkImage.h"
#include "include/core/SkRect.h"

CIALLO_BEGIN_NS

StreamHolder& operator<<(StreamHolder& os, GrGpeKind kind)
{
    switch (kind)
    {
    case GrGpeKind::kCompositor:
        os << "GrGpeKind::kCompositor";
        break;
    case GrGpeKind::kLayoutEngine:
        os << "GrGpeKind::kLayoutEngine";
        break;
    case GrGpeKind::kRenderLayer:
        os << "GrGpeKind::kRenderLayer";
        break;
    case GrGpeKind::kRenderTree:
        os << "GrGpeKind::kRenderTree";
        break;
    }
    return os;
}

GrGpe::GrGpe(const std::string &path, const std::string &name, GrGpeKind kind)
    : fPath(path),
      fName(name),
      fKind(kind),
      fAttachment(nullptr)
{
    logOut(LOG_INFO) << "Using pipeline extension \"" << fName
                     << "\" [" << fKind << "]" << logEndl;
}

void GrGpe::attach(const std::shared_ptr<QObject>& object)
{
    fAttachment = object;
}

void GrGpe::detach()
{
    fAttachment = nullptr;
}

CIALLO_END_NS
