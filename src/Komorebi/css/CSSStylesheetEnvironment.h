#ifndef COCOA_CSSSTYLESHEETENVIRONMENT_H
#define COCOA_CSSSTYLESHEETENVIRONMENT_H

#include "Komorebi/Komorebi.h"
KMR_NAMESPACE_BEGIN

enum class CSSUnit
{
    kPx,
    kEx,
    kEm,
    kIn,
    kCm,
    kMm,
    kPt,
    kPc,
    kCap,
    kCh,
    kIc,
    kRem,
    kLh,
    kRlh,
    kVh,
    kVw,
    kVi,
    kVb,
    kVmin,
    kVmax,
    kQ,
    kPercentage,
    kDeg,
    kGrad,
    kRad,
    kMs,
    kS,
    kHz,
    kKHz
};
using CSSFixed = int32_t;
using CSSARGBValue = uint32_t;
struct CSSFontDescriptor
{
    enum class Style
    {
        kInherit,
        kNormal,
        kItalic,
        kOblique
    };

    enum class Variant
    {
        kInherit,
        kNormal,
        kSmallCaps
    };

    enum class Weight
    {
        kInherit,
        kNormal,
        kBold,
        kBolder,
        kLighter,
        kWeight100,
        kWeight200,
        kWeight300,
        kWeight400,
        kWeight500,
        kWeight600,
        kWeight700,
        kWeight800,
        kWeight900,
    };

    struct Size
    {
        CSSFixed    size;
        CSSUnit     unit;
    };

    Style       style;
    Variant     variant;
    Weight      weight;
    Size        size;
    Size        lineHeight;
    std::string family;
};

class CSSStylesheetEnvironment
{
public:
    virtual ~CSSStylesheetEnvironment() = default;

    void setInlineStyle(bool value = true);
    bool isInlineStyle() const;

    virtual void importNotifier(const std::string& url) = 0;
    /* Resolve the color name to ARGB value.
       Throw an exception (stdexcept) if @a name is invalid. */
    virtual CSSARGBValue resolveColor(const std::string& name) = 0;
    virtual void resolveFont(const std::string& name, CSSFontDescriptor& font) = 0;

private:
    bool        fInlineStyle = false;
};

class CSSStylesheetDefaultEnvironment : public CSSStylesheetEnvironment
{
public:
    ~CSSStylesheetDefaultEnvironment() override = default;

    void importNotifier(const std::string& url) override;
    CSSARGBValue resolveColor(const std::string& name) override;
    void resolveFont(const std::string& name, CSSFontDescriptor& font) override;
};

KMR_NAMESPACE_END
#endif //COCOA_CSSSTYLESHEETENVIRONMENT_H
