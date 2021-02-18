#include "komorebi/Komorebi.h"
#include "komorebi/KEvent.h"
KMR_NAMESPACE_BEGIN

KEvent::KEvent(Kind kind)
    : fKind(kind)
{
}

KEvent::Kind KEvent::kind()
{
    return fKind;
}

// -----------------------------------------------------------------------

KRepaintEvent::KRepaintEvent(const SkRect& clip)
    : KEvent(EVENT_KIND),
      fClip(clip)
{
}

SkRect KRepaintEvent::clip() const
{
    return fClip;
}

void KRepaintEvent::setClip(const SkRect& clip)
{
    fClip = clip;
}

KMR_NAMESPACE_END
