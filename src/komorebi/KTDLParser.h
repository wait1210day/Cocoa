#ifndef COCOA_KTDLPARSER_H
#define COCOA_KTDLPARSER_H

#include <string>
#include <memory>
#include <list>

#include <Poco/Dynamic/Var.h>

#include "komorebi/Komorebi.h"
KMR_NAMESPACE_BEGIN

/* Kaleidoscopic Text Description Language Parser */
class KTDLParser
{
public:
    enum class AttributeKind
    {
        kNewLine,
        kTab,
        kSetTab,
        kFontSize,
        kFontStyle,
        kFontWeight,
        kFontName,
        kForeground,
        kBackground,
        kLink,
        kTranslate,
        kBlackMask,
        kUnderline,
        kStrikeout
    };

    enum class ValueType
    {
        kNull,
        kInteger,
        kString,
        kHexColor
    };

    struct Attribute
    {
        AttributeKind       attrKind;
        ValueType           valueType;
        Poco::Dynamic::Var  value;
    };

    struct Fragment
    {
        std::vector<Attribute>  attrs;
        const char             *pBegin;
        const char             *pEnd;
    };

    enum class TokenKind
    {
        kText,
        kAttribute,
        kEndAttribute,
        kEscape
    };
    struct Token
    {
        TokenKind       kind;
        const char     *pStart;
        const char     *pEnd;
    };

    using ValueStringT = std::string;
    using ValueIntegerT = int32_t;
    using ValueHexColorT = uint32_t;

    explicit KTDLParser(const std::string& data);
    ~KTDLParser();

    bool hasNextFragment();
    bool hasError() const;
    std::string getErrorDesc();
    std::unique_ptr<Fragment> nextFragment();

private:
    bool parseNext();
    void scanBuffer();

private:
    const char                 *fpBuffer;
    std::vector<Token>          fTokens;
    bool                        fError;
    std::string                 fErrorDesc;
    std::unique_ptr<Fragment>   fNextFragment;
    std::list<Attribute>        fAttributeStack;
};

KMR_NAMESPACE_END
#endif //COCOA_KTDLPARSER_H
