#include "include/core/SkRect.h"
#include "include/core/SkCanvas.h"

#include "Komorebi/Komorebi.h"
#include "Komorebi/KEvent.h"
#include "Komorebi/KWidget.h"
KMR_NAMESPACE_BEGIN

KWidget::KWidget(KWidget *parent, bool propagate, const LayoutInfo& layoutInfo)
    : ciallo::PaintNode(parent, layoutInfo.width, layoutInfo.height,
                        layoutInfo.left, layoutInfo.top),
      fPropagate(propagate)
{
}

void KWidget::setLayoutInfo(const LayoutInfo& layoutInfo)
{
    this->resize(layoutInfo.width, layoutInfo.height);
    this->moveTo(layoutInfo.left, layoutInfo.top);
}

void KWidget::setPropagate(bool propagate)
{
    fPropagate = propagate;
}

void KWidget::dispatch(KEvent *event)
{
    switch (event->kind())
    {
    case KEvent::Kind::kRepaintEvent:
        {
            ciallo::PaintNode::ScopedPaint scopedPaint(this);
            onRepaintEvent(this->asCanvas(), event->cast<KRepaintEvent>());
        }
        break;

    default:
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Unknown event type")
                .make<RuntimeException>();
    }

    if (!fPropagate || this->parent() == nullptr)
        return;

    if (event->kind() == KEvent::Kind::kRepaintEvent)
    {
        /* Update the coordinate because the child widgets the its parent widget
         * always use different coordinate system */
        auto *repaintEvent = event->cast<KRepaintEvent>();
        SkRect clip = repaintEvent->clip();

        repaintEvent->setClip(SkRect::MakeXYWH(clip.left() + this->left(),
                                               clip.top() + this->top(),
                                               clip.width(),
                                               clip.height()));
    }
    dynamic_cast<KWidget*>(this->parent())->dispatch(event);
}

void KWidget::onRepaintEvent(SkCanvas *canvas, const KRepaintEvent *event)
{
    /* We have nothing special to do.
     * This function should be implemented in inherited class. */
}

KMR_NAMESPACE_END
