#ifndef __GOM_H__
#define __GOM_H__

#include <list>
#include <vector>
#include <string>
#include <sstream>
#include <stack>
#include <cstring>

#include "core/QObject.h"

#define GOM_DECLARE(className) \
public: \
    static className *instance(); \
private: \
    static className *__mLLObject;

#define GOM_DEFINE(className) \
className *className::__mLLObject = nullptr; \
className *className::instance() \
{ \
    return __mLLObject; \
}

#define GOM_CONSTRUCT_DONE \
    __mLLObject = this;

/* GOM means the Global Objects Manager */
#define GOM     cocoa::oc

#define gom_prt_dump_context_buffer_size    128UL

namespace cocoa
{
typedef std::vector<std::string> StringVector;

template<typename _Tp>
class type_checker
{
public:
    using same_type = struct { char __field[1]; };
    using not_same_type = struct { char __field[2]; };
    static same_type check(_Tp);
    static not_same_type check(...);
};
#define type_is(type, ident)       ((bool)(sizeof(type_checker<type>::check(ident)) == 1))

namespace gom_utils
{
void resolve_address_symbol(const void *ptr, std::string& res);
};

/* PRT: Properties Representation Tree */
enum class PRTDataType
{
    PRT_DATA_INT,
    PRT_DATA_STRING,
    PRT_DATA_BOOL,
    PRT_DATA_DOUBLE
};

enum class PRTNodeType
{
    PRT_NODE_ROOT,
    PRT_NODE_DIRECTORY,
    PRT_NODE_PROPERTY
};

enum PRTNodeAttribute
{
    PRT_ATTR_ACCESS     = 0x01,
    PRT_ATTR_HIDDEN     = 0x02,
    PRT_ATTR_INHERIT    = 0x08
};

#define GOM_PRT_NAME_BUFFER_SIZE    32

using prt_type_int = int64_t;
using prt_type_double = double;
using prt_type_string = char *;
using prt_type_bool = bool;

struct PRTNode
{
    PRTNode *parent;
    std::list<PRTNode *> children;

    QObject   *owner;
    /* Only available when this node is root */
    char const *ownerStr;
    PRTNodeType nodeType;
    PRTDataType dataType;
    int         nodeAttribute;
    char        name[GOM_PRT_NAME_BUFFER_SIZE];

    union
    {
        prt_type_int  _int;
        prt_type_bool _bool;
        prt_type_string _str;
        prt_type_double _double;
    } content;

    std::uint32_t userdata;
};

class GOMCollector
{
public:
    typedef std::vector<std::string> InterpretedGOMPath;
public:
    GOMCollector();
    ~GOMCollector();

    void addObject(QObject *obj);
    void collectAll();

    /* Processing cocoa objects and properties path */
    void registerDirectory(QObject *obj, char const *dir, int attri = PRT_ATTR_INHERIT);
    bool hasPropertyOrDirectory(char const *path);

    void setPropertyInt(char const *path, int64_t data, int attri = PRT_ATTR_INHERIT);
    void setPropertyString(char const *path, char const *data, int attri = PRT_ATTR_INHERIT);
    void setPropertyBool(char const *path, bool data, int attri = PRT_ATTR_INHERIT);
    void setPropertyDouble(char const *path, double data, int attri = PRT_ATTR_INHERIT);

    void setNodeUserdata(const char *path, const std::uint32_t data)
    {
        InterpretedGOMPath ipath;
        interpretPropertyPath(path, ipath);

        PRTNode *node = getPRTNode(ipath);
        if (!node)
            throw std::runtime_error("No such property");
        
        node->userdata = data;
    }

    int getPropertyInt(char const *path);
    char const *getPropertyString(char const *path);
    bool getPropertyBool(char const *path);
    double getPropertyDouble(char const *path);

    std::uint32_t getNodeUserdata(const char *path)
    {
        InterpretedGOMPath ipath;
        interpretPropertyPath(path, ipath);

        PRTNode *node = getPRTNode(ipath);
        if (!node)
            throw std::runtime_error("No such property");

        return node->userdata;
    }

    void removePropertyOrDirectory(char const *path);
    void dumpPropertiesTree(StringVector& out);

private:
    void        createPRTRoot();
    PRTNode    *getPRTNode(InterpretedGOMPath& path);
    PRTNode    *createPRTNode(InterpretedGOMPath& path);
    void        removePRTNode(PRTNode *node);
    void        interpretPropertyPath(char const *path, InterpretedGOMPath& out);
    PRTNode    *__generic_set_property(const char *path, int attri);
    PRTNode    *__generic_get_property(const char *path, PRTDataType reqType);

private:
    std::stack<QObject *> mObjs;
    PRTNode  *mPRTRoot;
};

extern GOMCollector *oc;
void OCInitialize();
void OCFinalize();

#define reinterpret_value_as(type, ptr)     (*reinterpret_cast<type*>(ptr))
template<typename _Ts>
class StructurePRTAdder
{
public:
    StructurePRTAdder(GOMCollector *holder, const _Ts *st, const std::string& path)
        : fHolder(holder), fStructure(st), fPath(path) {}

    template<typename _Tf>
    void field(const std::string& name, off_t offset)
    {
        const _Tf *ptr = reinterpret_cast<const _Tf*>(reinterpret_cast<const uint8_t*>(fStructure) + offset);
        const std::type_info& field_type = typeid(_Tf);
        std::string path(fPath + '/' + name);
        const char *path_str = path.c_str();

        if (field_type == typeid(prt_type_int))
            fHolder->setPropertyInt(path_str, reinterpret_value_as(const prt_type_int, ptr));
        else if (field_type == typeid(prt_type_double))
            fHolder->setPropertyDouble(path_str, reinterpret_value_as(const prt_type_double, ptr));
        else if (field_type == typeid(prt_type_bool))
            fHolder->setPropertyBool(path_str, reinterpret_value_as(const prt_type_bool, ptr));
        else if (field_type == typeid(prt_type_string))
            fHolder->setPropertyString(path_str, reinterpret_value_as(const prt_type_string, ptr));
        else
            throw std::runtime_error("Type mismatched!");
    }

private:
    GOMCollector   *fHolder;
    const _Ts      *fStructure;
    std::string     fPath;
};
#define StructurePRTAdderField(adder, sttype, type, _field) \
    adder.field<type>(#_field, offsetof(sttype, _field))

} // namespace cocoa

#ifndef GOM_TYPE_HELPERS

#undef reinterpret_value_as

#endif // GOM_TYPE_HELPERS
#endif // __GOM_H__
