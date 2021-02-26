#ifndef COCOA_CSSSELECTIONCONTEXT_H
#define COCOA_CSSSELECTIONCONTEXT_H

#include <memory>

#include "Komorebi/Komorebi.h"
#include "Komorebi/css/CSSStylesheet.h"

struct css_select_ctx;
KMR_NAMESPACE_BEGIN

class CSSSelectionContext
{
public:
    CSSSelectionContext();
    ~CSSSelectionContext();

    static std::shared_ptr<CSSSelectionContext> Make();

    void appendSheet(const std::shared_ptr<CSSStylesheet>& sheet);
    css_select_ctx *nativeHandle();

private:
    css_select_ctx        *fSelectCtx;
    std::vector<std::shared_ptr<CSSStylesheet>>
                           fSheets;
};

KMR_NAMESPACE_END
#endif //COCOA_CSSSELECTIONCONTEXT_H
