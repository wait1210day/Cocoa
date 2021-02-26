#ifndef COCOA_STRICTJSONPARSER_H
#define COCOA_STRICTJSONPARSER_H

#include "Core/PropertyTree.h"

namespace cocoa::json {

class AnyTemplate
{
public:
    enum class Kind
    {
        kObjectTemplate,
        kArrayTemplate,
        kFinalValueTemplate
    };

    AnyTemplate(Kind kind, bool optional, const char *name) noexcept;
    virtual ~AnyTemplate() = default;

    [[nodiscard]] inline Kind kind() const
    {
        return fKind;
    }

    [[nodiscard]] inline bool optional() const
    {
        return fOptional;
    }

    [[nodiscard]] inline const char *name() const
    {
        return fName;
    }

    template<typename T>
    const T *cast() const
    {
        return dynamic_cast<const T*>(this);
    }

private:
    Kind        fKind;
    bool        fOptional;
    const char *fName;
};

class ObjectTemplate : public AnyTemplate
{
public:
    ObjectTemplate(bool optional,
                   const char *name,
                   const std::initializer_list<const AnyTemplate*>& members) noexcept;

    ObjectTemplate(bool optional,
                   const std::initializer_list<const AnyTemplate*>& members) noexcept;

    ~ObjectTemplate() override = default;

    [[nodiscard]] inline const std::vector<const AnyTemplate*>& members() const
    {
        return fMembers;
    }

private:

private:
    std::vector<const AnyTemplate*>  fMembers;
};

class ArrayTemplate : public AnyTemplate
{
public:
    ArrayTemplate(bool optional, const char *name, AnyTemplate *elementTemp) noexcept;
    ArrayTemplate(bool optional, AnyTemplate *elementTemp) noexcept;

    ~ArrayTemplate() override = default;

    [[nodiscard]] inline const AnyTemplate *elementTemplate() const
    {
        return fElementTemp;
    }

private:
    AnyTemplate     *fElementTemp;
};

class FinalValueTemplate : public AnyTemplate
{
public:
    enum Type
    {
        kType_Integer = 0,
        kType_Float,
        kType_String,
        kType_Boolean
    };

    FinalValueTemplate(bool optional, char const *name, Type type) noexcept;
    FinalValueTemplate(bool optional, Type type) noexcept;

    ~FinalValueTemplate() override = default;

    [[nodiscard]] inline Type type() const
    {
        return fType;
    }

private:
    Type    fType;
};

void parseFile(const std::string& file, const ObjectTemplate *rootTemplate, PropertyTreeNode *pNode);
void parseString(const std::string& file, const ObjectTemplate *rootTemplate, PropertyTreeNode *pNode);

} // namespace cocoa::json

#endif //COCOA_STRICTJSONPARSER_H
