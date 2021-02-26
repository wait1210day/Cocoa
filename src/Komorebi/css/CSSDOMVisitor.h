#ifndef COCOA_CSSDOMVISITOR_H
#define COCOA_CSSDOMVISITOR_H

#include <string>
#include <utility>
#include <vector>

#include "Komorebi/Komorebi.h"
#include "Komorebi/css/CSSComputedStyle.h"
KMR_NAMESPACE_BEGIN

class CSSDOMVisitor
{
public:
    struct QualifiedName
    {
        QualifiedName(std::string  ns, std::string  name)
                : fNamespace(std::move(ns)), fName(std::move(name)) {}
        bool operator==(const QualifiedName& that) const
        {
            if (fNamespace == "*" || that.fNamespace == "*")
                return fName == that.fName;
            return fNamespace == that.fNamespace &&
                   fName == that.fName;
        }
        std::string fNamespace;
        std::string fName;
    };

    using String = std::string;
    using StringList = std::vector<std::string>;
    using VisitorList = std::vector<CSSDOMVisitor*>;

    virtual QualifiedName nodeName() = 0;
    virtual StringList nodeClasses() = 0;
    virtual String nodeId() = 0;
    virtual CSSDOMVisitor *parentNode() = 0;
    virtual VisitorList childrenNodes() = 0;
    virtual bool hasName(const QualifiedName& qname) = 0;
    virtual bool hasClass(String _class) = 0;
    virtual bool hasId(String id) = 0;
    virtual bool hasAttribute(const QualifiedName& attribute) = 0;
    virtual String getAttribute(const QualifiedName& attribute) = 0;
    virtual bool isRoot() = 0;
    virtual int siblingCount(bool sameName, bool after) = 0;
    virtual bool isEmpty() = 0;
    virtual bool isLink() = 0;
    virtual bool isVisited() = 0;
    virtual bool isHover() = 0;
    virtual bool isActive() = 0;
    virtual bool isFocus() = 0;
    virtual bool isEnabled() = 0;
    virtual bool isDisabled() = 0;
    virtual bool isChecked() = 0;
    virtual bool isTarget() = 0;
    virtual bool isLang(const String& lang) = 0;

    void *getCSSNodeData();
    void setCSSNodeData(void *ptr);

    CSSComputedStyle *getStyleRules();
    void setStyleRules(std::unique_ptr<CSSComputedStyle> styles);

private:
    void    *fLibCSSNodeData;
    std::unique_ptr<CSSComputedStyle> fStyles;
};

KMR_NAMESPACE_END
#endif //COCOA_CSSDOMVISITOR_H
