#include <functional>
#include <cstring>

#include "komorebi/Komorebi.h"
#include "komorebi/css/LibCSSImport.h"
#include "komorebi/css/CSSCalculator.h"
KMR_NAMESPACE_BEGIN

#define LIBCSS_CALLBACK
#define TO_CHECKED(action, ...) \
    this->toRetChecked(action(__VA_ARGS__), __FUNCTION__, #action)

#define VTR(node)    static_cast<CSSDOMVisitor*>(node)

namespace {

CSSDOMVisitor::QualifiedName toVisitorQualifiedName(const css_qname *qname)
{
    return CSSDOMVisitor::QualifiedName(qname->ns ? lwc_string_data(qname->ns) : "",
                                       lwc_string_data(qname->name));
}

void toCSSQualifiedName(CSSDOMVisitor::QualifiedName& qualifiedName, css_qname *qname)
{
    if (qualifiedName.fNamespace.empty())
        qname->ns = nullptr;
    else
    {
        lwc_intern_string(qualifiedName.fNamespace.c_str(),
                          qualifiedName.fNamespace.size(), &qname->ns);
    }
    lwc_intern_string(qualifiedName.fName.c_str(),
                      qualifiedName.fName.size(), &qname->name);
}

LIBCSS_CALLBACK css_error handlerNodeName([[maybe_unused]] void *pUserdata,
                                          void *node,
                                          css_qname *qualifiedName)
{
    CSSDOMVisitor::QualifiedName qname = VTR(node)->nodeName();
    toCSSQualifiedName(qname, qualifiedName);

    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeClasses([[maybe_unused]] void *pUserdata,
                                             void *node,
                                             lwc_string ***pClasses,
                                             uint32_t *pnClasses)
{
    CSSDOMVisitor::StringList classesList = VTR(node)->nodeClasses();
    std::size_t numClasses = classesList.size();

    *pnClasses = numClasses;
    *pClasses = static_cast<lwc_string**>(std::malloc(numClasses * sizeof(lwc_string**)));
    for (std::size_t i = 0; i < numClasses; i++)
        lwc_intern_string(classesList[i].c_str(), classesList[i].size(), pClasses[i]);

    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeId([[maybe_unused]] void *pUserdata,
                                        void *node,
                                        lwc_string **pId)
{
    CSSDOMVisitor::String id = VTR(node)->nodeId();
    lwc_intern_string(id.c_str(), id.size(), pId);

    return CSS_OK;
}

CSSDOMVisitor *matchAncestorVisitor(CSSDOMVisitor *visitor,
                                    const std::function<bool(CSSDOMVisitor*)>& matcher)
{
    if (visitor == nullptr)
        return nullptr;

    if (matcher(visitor))
        return visitor;

    return matchAncestorVisitor(visitor->parentNode(), matcher);
}

LIBCSS_CALLBACK css_error handlerNamedAncestorNode([[maybe_unused]] void *pUserdata,
                                                   void *node,
                                                   const css_qname *qname,
                                                   void **ancestor)
{
    CSSDOMVisitor::QualifiedName visitorQualifiedName = toVisitorQualifiedName(qname);
    auto matcher = [&](CSSDOMVisitor *visitor) -> bool {
        CSSDOMVisitor::QualifiedName currentQName = visitor->nodeName();
        if (currentQName == visitorQualifiedName)
            return true;
        return false;
    };

    *ancestor = matchAncestorVisitor(VTR(node), matcher);
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNamedParentNode([[maybe_unused]] void *pUserdata,
                                                 void *node,
                                                 const css_qname *qname,
                                                 void **parent)
{
    *parent = nullptr;
    CSSDOMVisitor::QualifiedName qualifiedName = toVisitorQualifiedName(qname);
    CSSDOMVisitor *parentVisitor = VTR(node)->parentNode();

    if (parentVisitor == nullptr)
        return CSS_OK;

    if (parentVisitor->nodeName() == qualifiedName)
        *parent = parentVisitor;
    return CSS_OK;
}

/**
 * @concept In libcss, a "sibling node" means the previous node of
 *          current node. And a "generic sibling node" means all
 *          the sibling nodes of current node (excluding itself).
 */

LIBCSS_CALLBACK css_error handlerNamedSiblingNode([[maybe_unused]] void *pUserdata,
                                                  void *node,
                                                  const css_qname *qname,
                                                  void **sibling)
{
    CSSDOMVisitor::QualifiedName qualifiedName = toVisitorQualifiedName(qname);
    auto *parent = VTR(node)->parentNode();

    *sibling = nullptr;
    if (parent == nullptr)
        return CSS_OK;
    CSSDOMVisitor *prevVisitor = nullptr;

    for (auto *siblingVisitor : parent->childrenNodes())
    {
        if (siblingVisitor == VTR(node))
            break;
        prevVisitor = siblingVisitor;
    }

    if (prevVisitor == nullptr)
        return CSS_OK;
    if (prevVisitor->nodeName() == qualifiedName)
        *sibling = prevVisitor;
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNamedGenericSiblingNode([[maybe_unused]] void *pUserdata,
                                                         void *node,
                                                         const css_qname *qname,
                                                         void **sibling)
{
    *sibling = nullptr;
    auto *parent = VTR(node)->parentNode();
    if (parent == nullptr)
        return CSS_OK;

    CSSDOMVisitor::QualifiedName qualifiedName = toVisitorQualifiedName(qname);

    for (auto *siblingVisitor : parent->childrenNodes())
    {
        if (siblingVisitor == VTR(node))
            continue;
        if (siblingVisitor->nodeName() == qualifiedName)
        {
            *sibling = siblingVisitor;
            return CSS_OK;
        }
    }

    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerParentNode([[maybe_unused]] void *pUserdata,
                                            void *node,
                                            void **parent)
{
    *parent = VTR(node)->parentNode();
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerSiblingNode([[maybe_unused]] void *pUserdata,
                                             void *node,
                                             void **sibling)
{
    auto *parent = VTR(node)->parentNode();

    *sibling = nullptr;
    for (auto *siblingVisitor : parent->childrenNodes())
    {
        if (siblingVisitor == VTR(node))
            break;
        *sibling = siblingVisitor;
    }

    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeHasName([[maybe_unused]] void *pUserdata,
                                             void *node,
                                             const css_qname *qname,
                                             bool *match)
{
    CSSDOMVisitor::QualifiedName qualifiedName = toVisitorQualifiedName(qname);
    *match = VTR(node)->hasName(qualifiedName);
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeHasClass([[maybe_unused]] void *pUserdata,
                                              void *node,
                                              lwc_string *name,
                                              bool *match)
{
    *match = false;
    for (const std::string& cl : VTR(node)->nodeClasses())
    {
        if (cl == lwc_string_data(name))
            *match = true;
    }
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeHasId([[maybe_unused]] void *pUserdata,
                                           void *node,
                                           lwc_string *name,
                                           bool *match)
{
    std::string id = VTR(node)->nodeId();
    if (id == lwc_string_data(name))
        *match = true;
    else
        *match = false;
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeHasAttribute([[maybe_unused]] void *pUserdata,
                                                  void *node,
                                                  const css_qname *qname,
                                                  bool *match)
{
    CSSDOMVisitor::QualifiedName qualifiedName = toVisitorQualifiedName(qname);
    *match = VTR(node)->hasAttribute(qualifiedName);
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeHasAttributeEqual([[maybe_unused]] void *pUserdata,
                                                       void *node,
                                                       const css_qname *qname,
                                                       lwc_string *value,
                                                       bool *match)
{
    CSSDOMVisitor::QualifiedName qualifiedName = toVisitorQualifiedName(qname);
    *match = false;

    if (!VTR(node)->hasAttribute(qualifiedName))
        return CSS_OK;

    if (VTR(node)->getAttribute(qualifiedName) == lwc_string_data(value))
        *match = true;
    return CSS_OK;
}

bool matchStringFragment(char const *pSrc, char const *pMatch, std::size_t srcSize,
                         std::size_t matchSize, char separator)
{
    char const *pStart = pSrc;
    char const *pEnd = pSrc + srcSize;
    for (char const *pCurrent = pStart; pCurrent < pEnd; pCurrent++)
    {
        if (*pCurrent == separator)
        {
            if (static_cast<std::size_t>(pCurrent - pStart) == matchSize &&
                !std::strncmp(pStart, pMatch, matchSize))
            {
                return true;
            }
            pStart = pCurrent + 1;
        }
    }

    return false;
}

/**
 * @concept A attribute value can be separated into several parts
 *          by spaces. When a given string fully equal to one of
 *          these parts, we say the attribute value "includes"
 *          this string.
 */

LIBCSS_CALLBACK css_error handlerNodeHasAttributeInclude([[maybe_unused]] void *pUserdata,
                                                         void *node,
                                                         const css_qname *qname,
                                                         lwc_string *value,
                                                         bool *match)
{
    CSSDOMVisitor::QualifiedName qualifiedName = toVisitorQualifiedName(qname);
    *match = false;

    if (!VTR(node)->hasAttribute(qualifiedName))
        return CSS_OK;

    const char *pValue = lwc_string_data(value);
    std::size_t vlen = lwc_string_length(value);

    std::string actualValue = VTR(node)->getAttribute(qualifiedName);
    *match = matchStringFragment(actualValue.c_str(), pValue, actualValue.size(), vlen, ' ');
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeHasAttributeDashMatch([[maybe_unused]] void *pUserdata,
                                                          void *node,
                                                          const css_qname *qname,
                                                          lwc_string *value,
                                                          bool *match)
{
    CSSDOMVisitor::QualifiedName qualifiedName = toVisitorQualifiedName(qname);
    *match = false;

    if (!VTR(node)->hasAttribute(qualifiedName))
        return CSS_OK;

    const char *pValue = lwc_string_data(value);
    std::size_t vlen = lwc_string_length(value);

    std::string actualValue = VTR(node)->getAttribute(qualifiedName);
    *match = matchStringFragment(actualValue.c_str(), pValue, actualValue.size(), vlen, '-');
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeHasAttributePrefix([[maybe_unused]] void *pUserdata,
                                                        void *node,
                                                        const css_qname *qname,
                                                        lwc_string *value,
                                                        bool *match)
{
    *match = false;
    CSSDOMVisitor::QualifiedName qualifiedName = toVisitorQualifiedName(qname);

    if (!VTR(node)->hasAttribute(qualifiedName))
        return CSS_OK;

    char const *pMatch = lwc_string_data(value);
    std::string actualValue = VTR(node)->getAttribute(qualifiedName);

    *match = actualValue.find(pMatch, 0) == 0;
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeHasAttributeSuffix([[maybe_unused]] void *pUserdata,
                                                        void *node,
                                                        const css_qname *qname,
                                                        lwc_string *value,
                                                        bool *match)
{
    *match = false;
    CSSDOMVisitor::QualifiedName qualifiedName = toVisitorQualifiedName(qname);

    if (!VTR(node)->hasAttribute(qualifiedName))
        return CSS_OK;

    char const *pMatch = lwc_string_data(value);
    std::size_t matchSize = lwc_string_length(value);
    std::string actualValue = VTR(node)->getAttribute(qualifiedName);

    *match = actualValue.rfind(pMatch) == (actualValue.size() - matchSize);
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeHasAttributeSubstring([[maybe_unused]] void *pUserdata,
                                                           void *node,
                                                           const css_qname *qname,
                                                           lwc_string *value,
                                                           bool *match)
{
    *match = false;
    CSSDOMVisitor::QualifiedName qualifiedName = toVisitorQualifiedName(qname);

    if (!VTR(node)->hasAttribute(qualifiedName))
        return CSS_OK;

    char const *pMatch = lwc_string_data(value);
    std::string actualValue = VTR(node)->getAttribute(qualifiedName);

    *match = actualValue.find(pMatch) != std::string::npos;
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeIsRoot([[maybe_unused]] void *pUserdata,
                                            void *node,
                                            bool *match)
{
    *match = VTR(node)->parentNode() == nullptr;
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeCountSiblings([[maybe_unused]] void *pUserdata,
                                                   void *node,
                                                   bool sameName,
                                                   bool after,
                                                   int32_t *pCount)
{
    *pCount = 0;
    if (VTR(node)->parentNode() == nullptr)
        return CSS_OK;
    auto name = VTR(node)->nodeName();

    if (after)
    {
        bool locked = true;
        for (auto *sibling : VTR(node)->parentNode()->childrenNodes())
        {
            if (locked)
            {
                if (sibling == VTR(node))
                    locked = false;
                continue;
            }
            if (sameName)
            {
                if (sibling->nodeName() == name)
                    (*pCount)++;
            }
            else
                (*pCount)++;
        }
    }
    else
    {
        for (auto *sibling : VTR(node)->parentNode()->childrenNodes())
        {
            if (sibling == VTR(node))
                break;
            if (sameName)
            {
                if (sibling->nodeName() == name)
                    (*pCount)++;
            }
            else
                (*pCount)++;
        }
    }

    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeIsEmpty([[maybe_unused]] void *pUserdata,
                                             void *node,
                                             bool *match)
{
    *match = VTR(node)->isEmpty();
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeIsLink([[maybe_unused]] void *pUserdata,
                                            void *node,
                                            bool *match)
{
    *match = VTR(node)->isLink();
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeIsVisited([[maybe_unused]] void *pUserdata,
                                               void *node,
                                               bool *match)
{
    *match = VTR(node)->isVisited();
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeIsHover([[maybe_unused]] void *pUserdata,
                                             void *node,
                                             bool *match)
{
    *match = VTR(node)->isHover();
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeIsActive([[maybe_unused]] void *pUserdata,
                                              void *node,
                                              bool *match)
{
    *match = VTR(node)->isActive();
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeIsFocus([[maybe_unused]] void *pUserdata,
                                             void *node,
                                             bool *match)
{
    *match = VTR(node)->isFocus();
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeIsEnabled([[maybe_unused]] void *pUserdata,
                                               void *node,
                                               bool *match)
{
    *match = VTR(node)->isEnabled();
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeIsDisabled([[maybe_unused]] void *pUserdata,
                                                void *node,
                                                bool *match)
{
    *match = VTR(node)->isDisabled();
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeIsChecked([[maybe_unused]] void *pUserdata,
                                               void *node,
                                               bool *match)
{
    *match = VTR(node)->isChecked();
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeIsTarget([[maybe_unused]] void *pUserdata,
                                              void *node,
                                              bool *match)
{
    *match = VTR(node)->isTarget();
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodeIsLang([[maybe_unused]] void *pUserdata,
                                            void *node,
                                            lwc_string *lang,
                                            bool *match)
{
    std::string strLang(lwc_string_data(lang));
    *match = VTR(node)->isLang(strLang);
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerNodePresentationalHint([[maybe_unused]] void *pUserdata,
                                                        [[maybe_unused]] void *node,
                                                        uint32_t *numHints,
                                                        css_hint **hints)
{
    /* TODO: I really don't known what's this...
     *       but it should work fine without "presentational hint" */
    *numHints = 0;
    *hints = nullptr;
    return CSS_OK;
}

LIBCSS_CALLBACK css_error handlerUADefaultProperty([[maybe_unused]] void *pUserdata,
                                                   uint32_t property,
                                                   css_hint *hint)
{
    switch (property)
    {
    case CSS_PROP_COLOR:
        hint->data.color = 0xff0000;
        hint->status = CSS_COLOR_COLOR;
        break;

    case CSS_PROP_FONT_FAMILY:
        hint->data.strings = nullptr;
        hint->status = CSS_FONT_FAMILY_SANS_SERIF;
        break;

    case CSS_PROP_QUOTES:
        hint->data.strings = nullptr;
        hint->status = CSS_QUOTES_NONE;
        break;

    case CSS_PROP_VOICE_FAMILY:
        /* CSS voice features are disabled in Komorebi */
        hint->data.strings = nullptr;
        hint->status = 0;
        break;

    default:
        return CSS_INVALID;
    }

    return CSS_OK;
}

css_error handlerComputeFontSize([[maybe_unused]] void *pUserdata,
                                 const css_hint *parent,
                                 css_hint *size)
{
    static css_hint_length sizes[] = {
            { FLTTOFIX(6.75), CSS_UNIT_PT },
            { FLTTOFIX(7.50), CSS_UNIT_PT },
            { FLTTOFIX(9.75), CSS_UNIT_PT },
            { FLTTOFIX(12.0), CSS_UNIT_PT },
            { FLTTOFIX(13.5), CSS_UNIT_PT },
            { FLTTOFIX(18.0), CSS_UNIT_PT },
            { FLTTOFIX(24.0), CSS_UNIT_PT }
    };
    const css_hint_length *parentSize;

    if (parent == nullptr)
        parentSize = &sizes[CSS_FONT_SIZE_MEDIUM - 1];
    else
    {
        if (parent->status != CSS_FONT_SIZE_DIMENSION ||
            parent->data.length.unit == CSS_UNIT_EX ||
            parent->data.length.unit == CSS_UNIT_EM)
        {
            return CSS_INVALID;
        }
        parentSize = &parent->data.length;
    }

    if (size->status == CSS_FONT_SIZE_INHERIT)
        return CSS_INVALID;

    if (size->status < CSS_FONT_SIZE_LARGER)
    {
        size->data.length = sizes[size->status - 1];
    }
    else if (size->status == CSS_FONT_SIZE_LARGER)
    {
        size->data.length.value = FMUL(parentSize->value, FLTTOFIX(1.2));
        size->data.length.unit = parentSize->unit;
    }
    else if (size->status == CSS_FONT_SIZE_SMALLER)
    {
        size->data.length.value = FDIV(parentSize->value, FLTTOFIX(1.2));
        size->data.length.unit = parentSize->unit;
    }
    else if (size->data.length.unit == CSS_UNIT_EM ||
             size->data.length.unit == CSS_UNIT_EX)
    {
        size->data.length.value = FMUL(size->data.length.value, parentSize->value);
        if (size->data.length.unit == CSS_UNIT_EX)
            size->data.length.value = FMUL(size->data.length.value,
                                           FLTTOFIX(0.6));
    }
    else if (size->data.length.unit == CSS_UNIT_PCT)
    {
        size->data.length.value = FDIV(FMUL(size->data.length.value, parentSize->value),
                                       FLTTOFIX(100));
        size->data.length.unit = parentSize->unit;
    }
    size->status = CSS_FONT_SIZE_DIMENSION;
    return CSS_OK;
}

css_error handlerSetNodeData(void *pw,
                             void *node,
                             void *data)
{
    VTR(node)->setCSSNodeData(data);
    return CSS_OK;
}

css_error handlerGetNodeData(void *pw,
                             void *node,
                             void **data)
{
    *data = VTR(node)->getCSSNodeData();
    return CSS_OK;
}

} // anonymous namespace

void CSSCalculator::elevate(std::shared_ptr<CSSSelectionContext> context, CSSDOMVisitor *visitor)
{
    css_media media{
        .type = CSS_MEDIA_SCREEN
    };
    css_select_handler handler{
        .handler_version = CSS_SELECT_HANDLER_VERSION_1,

        .node_name = handlerNodeName,
        .node_classes = handlerNodeClasses,
        .node_id = handlerNodeId,
        .named_ancestor_node = handlerNamedAncestorNode,
        .named_parent_node = handlerNamedParentNode,
        .named_sibling_node = handlerNamedSiblingNode,
        .named_generic_sibling_node = handlerNamedGenericSiblingNode,
        .parent_node = handlerParentNode,
        .sibling_node = handlerSiblingNode,
        .node_has_name = handlerNodeHasName,
        .node_has_class = handlerNodeHasClass,
        .node_has_id = handlerNodeHasId,
        .node_has_attribute = handlerNodeHasAttribute,
        .node_has_attribute_equal = handlerNodeHasAttributeEqual,
        .node_has_attribute_dashmatch = handlerNodeHasAttributeDashMatch,
        .node_has_attribute_includes = handlerNodeHasAttributeInclude,
        .node_has_attribute_prefix = handlerNodeHasAttributePrefix,
        .node_has_attribute_suffix = handlerNodeHasAttributeSuffix,
        .node_has_attribute_substring = handlerNodeHasAttributeSubstring,
        .node_is_root = handlerNodeIsRoot,
        .node_count_siblings = handlerNodeCountSiblings,
        .node_is_empty = handlerNodeIsEmpty,
        .node_is_link = handlerNodeIsLink,
        .node_is_visited = handlerNodeIsVisited,
        .node_is_hover = handlerNodeIsHover,
        .node_is_active = handlerNodeIsActive,
        .node_is_focus = handlerNodeIsFocus,
        .node_is_enabled = handlerNodeIsEnabled,
        .node_is_disabled = handlerNodeIsDisabled,
        .node_is_checked = handlerNodeIsChecked,
        .node_is_target = handlerNodeIsTarget,
        .node_is_lang = handlerNodeIsLang,
        .node_presentational_hint = handlerNodePresentationalHint,
        .ua_default_for_property = handlerUADefaultProperty,
        .compute_font_size = handlerComputeFontSize,
        .set_libcss_node_data = handlerSetNodeData,
        .get_libcss_node_data = handlerGetNodeData
    };

    css_select_results *selectResults;
    css_select_ctx *selectCtx = context->nativeHandle();

    TO_CHECKED(css_select_style,
               selectCtx,
               visitor,
               &media,
               nullptr,
               &handler,
               this,
               &selectResults);

    if (!visitor->isRoot())
    {
        css_computed_style *composed;
        TO_CHECKED(css_computed_style_compose,
                   visitor->parentNode()->getStyleRules()->nativeHandle()->styles[CSS_PSEUDO_ELEMENT_NONE],
                   selectResults->styles[CSS_PSEUDO_ELEMENT_NONE],
                   handlerComputeFontSize,
                   nullptr,
                   &composed);
        css_computed_style_destroy(selectResults->styles[CSS_PSEUDO_ELEMENT_NONE]);
        selectResults->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
    }

    visitor->setStyleRules(std::make_unique<CSSComputedStyle>(selectResults));
}

void CSSCalculator::toRetChecked(int ret, const char *func, const char *action)
{
    if (ret != CSS_OK)
    {
        artfmt(func) << action << " failed and returned " << ret;
        arthrow<KmrException>(new KmrException);
    }
}

KMR_NAMESPACE_END
