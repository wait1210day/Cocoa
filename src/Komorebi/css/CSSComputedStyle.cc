#include "Komorebi/css/LibCSSImport.h"
#include "Komorebi/css/CSSComputedStyle.h"
KMR_NAMESPACE_BEGIN

CSSComputedStyle::CSSComputedStyle(css_select_results *pResults)
    : fResults(pResults)
{
}

CSSComputedStyle::~CSSComputedStyle()
{
    if (fResults != nullptr)
        css_select_results_destroy(fResults);
}

CSSComputedStyle::CSSComputedStyle(CSSComputedStyle&& other) noexcept
    : fResults(other.fResults)
{
    other.fResults = nullptr;
}

css_select_results *CSSComputedStyle::nativeHandle() noexcept
{
    return fResults;
}

KMR_NAMESPACE_END
