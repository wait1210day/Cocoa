#include <memory>

#include "Komorebi/css/LibCSSImport.h"
#include "Komorebi/css/CSSSelectionContext.h"
KMR_NAMESPACE_BEGIN

std::shared_ptr<CSSSelectionContext> CSSSelectionContext::Make()
{
    return std::make_shared<CSSSelectionContext>();
}

CSSSelectionContext::CSSSelectionContext()
    : fSelectCtx(nullptr)
{
    css_select_ctx_create(&fSelectCtx);
}

CSSSelectionContext::~CSSSelectionContext()
{
    if (fSelectCtx != nullptr)
        css_select_ctx_destroy(fSelectCtx);
}

void CSSSelectionContext::appendSheet(const std::shared_ptr<CSSStylesheet>& sheet)
{
    css_select_ctx_append_sheet(fSelectCtx, sheet->nativeHandle(),
                                CSS_ORIGIN_AUTHOR, nullptr);

    /* Keep a reference count to avoid Stylesheet being destructed
       before SelectionContext being destructed. */
    fSheets.push_back(sheet);
}

css_select_ctx *CSSSelectionContext::nativeHandle()
{
    return fSelectCtx;
}

KMR_NAMESPACE_END
