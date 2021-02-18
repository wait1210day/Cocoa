#include <iostream>
#include <string>
#include <memory>
#include <utility>

#include "komorebi/css/LibCSSImport.h"
#include "komorebi/css/CSSStylesheet.h"
KMR_NAMESPACE_BEGIN

#define LIBCSS_CALLBACK
#define TO_CHECKED(method, ...)     this->toRetChecked(method(__VA_ARGS__), __FUNCTION__, #method)

namespace {

LIBCSS_CALLBACK css_error urlResolver(void *userdata,
                                      const char *base,
                                      lwc_string *url,
                                      lwc_string **result)
{
    *result = lwc_string_ref(url);
    return CSS_OK;
}

LIBCSS_CALLBACK css_error importNotifier(void *userdata,
                                         css_stylesheet *parent,
                                         lwc_string *url)
{
    auto env = static_cast<CSSStylesheet*>(userdata)->environment();
    env->importNotifier(lwc_string_data(url));

    return CSS_OK;
}

LIBCSS_CALLBACK css_error colorResolver(void *userdata,
                                        lwc_string *name,
                                        css_color *color)
{
    try
    {
        auto env = static_cast<CSSStylesheet*>(userdata)->environment();
        CSSARGBValue value = env->resolveColor(lwc_string_data(name));
        *color = value;
    }
    catch (const std::exception& e)
    {
        return CSS_INVALID;
    }

    return CSS_OK;
}

css_unit toLibcssUnit(CSSUnit unit)
{
    switch (unit)
    {
    case CSSUnit::kPx:
        return CSS_UNIT_PX;
    case CSSUnit::kEx:
        return CSS_UNIT_EX;
    case CSSUnit::kEm:
        return CSS_UNIT_EM;
    case CSSUnit::kIn:
        return CSS_UNIT_IN;
    case CSSUnit::kCm:
        return CSS_UNIT_CM;
    case CSSUnit::kMm:
        return CSS_UNIT_MM;
    case CSSUnit::kPt:
        return CSS_UNIT_PT;
    case CSSUnit::kPc:
        return CSS_UNIT_PC;
    case CSSUnit::kCap:
        return CSS_UNIT_CAP;
    case CSSUnit::kCh:
        return CSS_UNIT_CH;
    case CSSUnit::kIc:
        return CSS_UNIT_IC;
    case CSSUnit::kRem:
        return CSS_UNIT_REM;
    case CSSUnit::kLh:
        return CSS_UNIT_LH;
    case CSSUnit::kRlh:
        return CSS_UNIT_RLH;
    case CSSUnit::kVh:
        return CSS_UNIT_VH;
    case CSSUnit::kVw:
        return CSS_UNIT_VW;
    case CSSUnit::kVi:
        return CSS_UNIT_VI;
    case CSSUnit::kVb:
        return CSS_UNIT_VB;
    case CSSUnit::kVmin:
        return CSS_UNIT_VMIN;
    case CSSUnit::kVmax:
        return CSS_UNIT_VMAX;
    case CSSUnit::kQ:
        return CSS_UNIT_Q;
    case CSSUnit::kPercentage:
        return CSS_UNIT_PCT;
    case CSSUnit::kDeg:
        return CSS_UNIT_DEG;
    case CSSUnit::kGrad:
        return CSS_UNIT_GRAD;
    case CSSUnit::kRad:
        return CSS_UNIT_RAD;
    case CSSUnit::kMs:
        return CSS_UNIT_MS;
    case CSSUnit::kS:
        return CSS_UNIT_S;
    case CSSUnit::kHz:
        return CSS_UNIT_HZ;
    case CSSUnit::kKHz:
        return CSS_UNIT_KHZ;
    }
}

css_font_style_e toLibcssFontStyle(CSSFontDescriptor::Style style)
{
    switch (style)
    {
    case CSSFontDescriptor::Style::kInherit:
        return CSS_FONT_STYLE_INHERIT;
    case CSSFontDescriptor::Style::kNormal:
        return CSS_FONT_STYLE_NORMAL;
    case CSSFontDescriptor::Style::kItalic:
        return CSS_FONT_STYLE_ITALIC;
    case CSSFontDescriptor::Style::kOblique:
        return CSS_FONT_STYLE_OBLIQUE;
    }
}

css_font_variant_e toLibcssFontVariant(CSSFontDescriptor::Variant variant)
{
    switch (variant)
    {
    case CSSFontDescriptor::Variant::kInherit:
        return CSS_FONT_VARIANT_INHERIT;
    case CSSFontDescriptor::Variant::kNormal:
        return CSS_FONT_VARIANT_NORMAL;
    case CSSFontDescriptor::Variant::kSmallCaps:
        return CSS_FONT_VARIANT_SMALL_CAPS;
    }
}

css_font_weight_e toLibcssFontWeight(CSSFontDescriptor::Weight weight)
{
    switch (weight)
    {
    case CSSFontDescriptor::Weight::kInherit:
        return CSS_FONT_WEIGHT_INHERIT;
    case CSSFontDescriptor::Weight::kNormal:
        return CSS_FONT_WEIGHT_NORMAL;
    case CSSFontDescriptor::Weight::kBold:
        return CSS_FONT_WEIGHT_BOLD;
    case CSSFontDescriptor::Weight::kBolder:
        return CSS_FONT_WEIGHT_BOLDER;
    case CSSFontDescriptor::Weight::kLighter:
        return CSS_FONT_WEIGHT_LIGHTER;
    case CSSFontDescriptor::Weight::kWeight100:
        return CSS_FONT_WEIGHT_100;
    case CSSFontDescriptor::Weight::kWeight200:
        return CSS_FONT_WEIGHT_200;
    case CSSFontDescriptor::Weight::kWeight300:
        return CSS_FONT_WEIGHT_300;
    case CSSFontDescriptor::Weight::kWeight400:
        return CSS_FONT_WEIGHT_400;
    case CSSFontDescriptor::Weight::kWeight500:
        return CSS_FONT_WEIGHT_500;
    case CSSFontDescriptor::Weight::kWeight600:
        return CSS_FONT_WEIGHT_600;
    case CSSFontDescriptor::Weight::kWeight700:
        return CSS_FONT_WEIGHT_700;
    case CSSFontDescriptor::Weight::kWeight800:
        return CSS_FONT_WEIGHT_800;
    case CSSFontDescriptor::Weight::kWeight900:
        return CSS_FONT_WEIGHT_900;
    }
}

void fontDescriptorToCssFont(const CSSFontDescriptor& descriptor, css_system_font *cssFont)
{
    cssFont->style = toLibcssFontStyle(descriptor.style);
    cssFont->variant = toLibcssFontVariant(descriptor.variant);
    cssFont->weight = toLibcssFontWeight(descriptor.weight);

    cssFont->size.unit = toLibcssUnit(descriptor.size.unit);
    cssFont->size.size = descriptor.size.size;

    cssFont->line_height.unit = toLibcssUnit(descriptor.lineHeight.unit);
    cssFont->line_height.size = descriptor.lineHeight.size;
    lwc_intern_string(descriptor.family.c_str(), descriptor.family.length(), &cssFont->family);
}

LIBCSS_CALLBACK css_error fontResolver(void *userdata,
                                       lwc_string *name,
                                       css_system_font *result)
{
    CSSFontDescriptor font;
    try
    {
        auto env = static_cast<CSSStylesheet*>(userdata)->environment();
        env->resolveFont(lwc_string_data(name), font);
    }
    catch (const std::exception& e)
    {
        return CSS_INVALID;
    }

    fontDescriptorToCssFont(font, result);
    return CSS_OK;
}

} // anonymous namespace

std::shared_ptr<CSSStylesheet> CSSStylesheet::Make(Level level, Charset charset, const std::string& url,
                                                   const std::string& title,
                                                   std::unique_ptr<CSSStylesheetEnvironment> environment)
{
    return std::make_shared<CSSStylesheet>(level, charset, url, title, std::move(environment));
}

CSSStylesheet::CSSStylesheet(Level level, Charset charset,
                             const std::string& url, const std::string& title,
                             std::unique_ptr<CSSStylesheetEnvironment> environment)
    : Throwable("cocoa::komorebi::CSSStylesheet"),
      fStylesheet(nullptr),
      fParsed(false),
      fEnvironment(std::move(environment))
{
    css_stylesheet_params params;
    params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
    params.url = url.c_str();
    params.title = title.c_str();
    params.allow_quirks = false;
    params.inline_style = fEnvironment->isInlineStyle();
    params.resolve = urlResolver;
    params.resolve_pw = this;
    params.import = importNotifier;
    params.import_pw = this;
    params.color = colorResolver;
    params.color_pw = this;
    params.font = fontResolver;
    params.font_pw = this;

    switch (level)
    {
    case Level::kCSSLevel_1:
        params.level = CSS_LEVEL_1;
        break;
    case Level::kCSSLevel_2:
        params.level = CSS_LEVEL_2;
        break;
    case Level::kCSSLevel_21:
        params.level = CSS_LEVEL_21;
        break;
    case Level::kCSSLevel_3:
        params.level = CSS_LEVEL_3;
        break;
    }

    switch (charset)
    {
    case Charset::kUtf8:
        params.charset = "UTF-8";
        break;
    case Charset::kAutoDetect:
        params.charset = nullptr;
        break;
    }

    TO_CHECKED(css_stylesheet_create, &params, &fStylesheet);
}

CSSStylesheet::~CSSStylesheet()
{
    if (fStylesheet)
    {
        /* We don't check the return value anymore in
         destructor. */
        css_stylesheet_destroy(fStylesheet);
    }
}

void CSSStylesheet::appendData(const std::string& data)
{
    TO_CHECKED(css_stylesheet_append_data, fStylesheet,
               reinterpret_cast<const uint8_t*>(data.c_str()), data.size() + 1);
}

void CSSStylesheet::parse()
{
    TO_CHECKED(css_stylesheet_data_done, fStylesheet);
    fParsed = true;
}

bool CSSStylesheet::isParsed() const
{
    return fParsed;
}

void CSSStylesheet::toRetChecked(int ret, const char *func, const char *method)
{
    if (ret == CSS_OK || ret == CSS_NEEDDATA)
        return;

    char const *err = css_error_to_string((css_error)ret);
    artfmt(func) << method << ": " << err;
    arthrow<KmrException>(new KmrException);
}

CSSStylesheetEnvironment *CSSStylesheet::environment()
{
    return fEnvironment.get();
}

css_stylesheet *CSSStylesheet::nativeHandle()
{
    return fStylesheet;
}

KMR_NAMESPACE_END
