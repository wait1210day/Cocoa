#ifndef COCOA_KWIDGET_H
#define COCOA_KWIDGET_H

#include <string>

#include "include/core/SkCanvas.h"

#include "ciallo/DIR/PaintNode.h"
#include "komorebi/Komorebi.h"
#include "komorebi/KEvent.h"
KMR_NAMESPACE_BEGIN

class KWidget : public ciallo::PaintNode
{
public:
    struct LayoutInfo
    {
        int32_t width;
        int32_t height;
        int32_t left;
        int32_t top;
    };

    KWidget(const std::string& inherit, KWidget *parent, bool propagate, const LayoutInfo& layoutInfo);
    ~KWidget() override = default;

    void setLayoutInfo(const LayoutInfo& layoutInfo);
    void setPropagate(bool propagate);

    void dispatch(KEvent *event);

protected:
    virtual void onRepaintEvent(SkCanvas *canvas, const KRepaintEvent *event);

private:
    bool        fPropagate;
};

KMR_NAMESPACE_END
#endif //COCOA_WIDGET_H
