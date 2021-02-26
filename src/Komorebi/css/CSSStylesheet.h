#ifndef COCOA_CSSSTYLESHEET_H
#define COCOA_CSSSTYLESHEET_H

#include <string>
#include <memory>

#include "Core/Exception.h"
#include "Komorebi/Komorebi.h"
#include "Komorebi/css/CSSStylesheetEnvironment.h"

struct css_stylesheet;
KMR_NAMESPACE_BEGIN

class CSSStylesheet
{
public:
    enum class Level
    {
        kCSSLevel_1,
        kCSSLevel_2,
        kCSSLevel_21,
        kCSSLevel_3,
        kCSSLevel_Default = kCSSLevel_3
    };

    enum class Charset
    {
        kUtf8,
        kAutoDetect
    };

    CSSStylesheet(Level level, Charset charset, const std::string& url, const std::string& title,
                  std::unique_ptr<CSSStylesheetEnvironment> environment);
    ~CSSStylesheet();

    static std::shared_ptr<CSSStylesheet> Make(Level level, Charset charset, const std::string& url,
                                               const std::string& title,
                                               std::unique_ptr<CSSStylesheetEnvironment> environment);

    void appendData(const std::string& data);
    void parse();

    bool isParsed() const;
    CSSStylesheetEnvironment *environment();
    css_stylesheet *nativeHandle();

private:
    void toRetChecked(int ret, const char *func, const char *method);

private:
    css_stylesheet       *fStylesheet;
    bool                  fParsed;
    std::unique_ptr<CSSStylesheetEnvironment>   fEnvironment;
};

KMR_NAMESPACE_END
#endif //COCOA_CSSSTYLESHEET_H
