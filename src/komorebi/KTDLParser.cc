#include <stdexcept>
#include <cstring>
#include <list>
#include <iostream>

#include "komorebi/KTDLParser.h"

KMR_NAMESPACE_BEGIN

namespace {

void scan_token_in_bracket(const char *pBegin, const char *pEnd, KTDLParser::Token& token)
{
    if (*pBegin != '[' || *pEnd != ']')
        throw std::runtime_error("Invalid calling for scan_token_in_bracket");

    pBegin++;
    pEnd--;
    bool filterTailSpaces;

    if (*pEnd == '!')
    {
        pEnd--;
        token.kind = KTDLParser::TokenKind::kEscape;
        filterTailSpaces = true;
    }
    else if (*pBegin == '!')
    {
        pBegin++;
        token.kind = KTDLParser::TokenKind::kEndAttribute;
        filterTailSpaces = true;
    }
    else
    {
        token.kind = KTDLParser::TokenKind::kAttribute;
        filterTailSpaces = false;
    }

    // Filter head and tail spaces
    while (*pBegin == ' ')
        pBegin++;
    if (filterTailSpaces)
    {
        while (*pEnd == ' ')
            pEnd--;
    }

    if (pBegin > pEnd)
        throw std::runtime_error("Expecting content in \"[]\" declaration");
    token.pStart = pBegin;
    token.pEnd = pEnd;
}

KTDLParser::Token scan_next_token(const char *& ptr)
{
    KTDLParser::Token token{};
    const char *pBegin = ptr;

    if (*ptr == '\0')
        throw std::runtime_error("Nothing need to be scanned");
    else if (*ptr == '[')
    {
        while (*ptr != ']' && *ptr != '\0')
            ptr++;
        if (*ptr == '\0')
            throw std::runtime_error("Expecting a \"[\" before ending");
        scan_token_in_bracket(pBegin, ptr, token);
        ptr++;
    }
    else
    {
        token.pStart = ptr;
        while (*ptr != '[' && *ptr != '\0')
            ptr++;
        token.pEnd = ptr - 1;
        token.kind = KTDLParser::TokenKind::kText;
    }
    return token;
}

void stream_string_fragment(std::ostream& os, const char *pBegin, const char *pEnd)
{
    while (pBegin <= pEnd)
    {
        os << *pBegin;
        pBegin++;
    }
}

std::ostream& operator<<(std::ostream& os, const KTDLParser::Token& token)
{
    switch (token.kind)
    {
    case KTDLParser::TokenKind::kText:
        os << "Text[";
        break;
    case KTDLParser::TokenKind::kAttribute:
        os << "Attribute[";
        break;
    case KTDLParser::TokenKind::kEndAttribute:
        os << "EndAttribute[";
        break;
    case KTDLParser::TokenKind::kEscape:
        os << "Escape[";
        break;
    }
    stream_string_fragment(os, token.pStart, token.pEnd);
    os << "]";
    return os;
}

} // namespace anonymous

KTDLParser::KTDLParser(const std::string& data)
    : fpBuffer(nullptr),
      fError(false),
      fNextFragment(nullptr)
{
    char *pBuffer = static_cast<char*>(std::malloc(data.size() + 1));
    if (pBuffer == nullptr)
        throw std::runtime_error("Failed to allocate buffer");

    std::memcpy(pBuffer, data.c_str(), data.size() + 1);
    fpBuffer = pBuffer;
    scanBuffer();

    for (const auto& token : fTokens)
    {
        std::cout << token << std::endl;
    }
}

KTDLParser::~KTDLParser()
{
    if (fpBuffer != nullptr)
        std::free(const_cast<char*>(fpBuffer));
}

bool KTDLParser::hasError() const
{
    return fError;
}

std::string KTDLParser::getErrorDesc()
{
    return fErrorDesc;
}

std::unique_ptr<KTDLParser::Fragment> KTDLParser::nextFragment()
{
    return std::move(fNextFragment);
}

bool KTDLParser::hasNextFragment()
{
    bool ret = false;
    try
    {
        ret = parseNext();
    }
    catch (const std::runtime_error& e)
    {
        fError = true;
        fErrorDesc = e.what();
    }
    return ret;
}

void KTDLParser::scanBuffer()
{
    const char *ptr = fpBuffer;
    while (*ptr != '\0')
        fTokens.emplace_back(scan_next_token(ptr));
}

bool KTDLParser::parseNext()
{
}

KMR_NAMESPACE_END
