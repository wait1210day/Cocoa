#include <iostream>

#include "Komorebi/Komorebi.h"
#include "Komorebi/css/CSSStylesheetEnvironment.h"
KMR_NAMESPACE_BEGIN

void CSSStylesheetEnvironment::setInlineStyle(bool value)
{
    fInlineStyle = value;
}

bool CSSStylesheetEnvironment::isInlineStyle() const
{
    return fInlineStyle;
}

// ----------------------------------------------------------------

/* TODO: For a actual implement CSSStylesheetDefaultEnvironment */

void CSSStylesheetDefaultEnvironment::importNotifier(const std::string& url)
{
    std::cout << "Import " << url << std::endl;
}

CSSARGBValue CSSStylesheetDefaultEnvironment::resolveColor(const std::string& name)
{
    std::cout << "Color " << name << std::endl;
    return 0;
}

void CSSStylesheetDefaultEnvironment::resolveFont(const std::string& name, CSSFontDescriptor& font)
{
    std::cout << "Font " << name << std::endl;

    font.family = "family";
    font.style = CSSFontDescriptor::Style::kNormal;
    font.variant = CSSFontDescriptor::Variant::kNormal;
    font.weight = CSSFontDescriptor::Weight::kNormal;
    font.size.size = 0;
    font.size.unit = CSSUnit::kPx;
    font.lineHeight.size = 0;
    font.lineHeight.unit = CSSUnit::kPx;
}

KMR_NAMESPACE_END
