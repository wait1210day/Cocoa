#include <cstring>
#include <stdexcept>
#include <iostream>
#include <dlfcn.h>
#include <cxxabi.h>

#include "core/Project.h"
#include "core/ObjectHolder.h"

namespace cocoa
{
GOMCollector *oc = nullptr;

namespace gom_utils
{

void resolve_address_symbol(const void *ptr, std::string& res)
{
	res.clear();
	Dl_info info;
	signed long dist;
	const char *demangled = nullptr;
	std::ostringstream ss;

	if (dladdr(ptr, &info) < 0)
		goto err;
	
	dist = reinterpret_cast<unsigned long>(ptr)
		- reinterpret_cast<unsigned long>(info.dli_saddr);
	demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, nullptr);
	if (!demangled)
		demangled = info.dli_sname;
	
	if (dist >= 0)
		ss << std::hex << ptr << "<+" << dist << "> " << demangled;
	else
		ss << std::hex << ptr << "<" << dist << "> " << demangled;
	
	if (demangled != info.dli_sname)
		std::free(const_cast<char *>(demangled));

	res.append(ss.str());
	return ;

err:
	res.append("Unknown");
}

}; // namespace gom_utils

void OCInitialize()
{
    if (!oc)
        oc = new GOMCollector();
}

void OCFinalize()
{
    if (oc)
    {
        delete oc;
        oc = nullptr;
    }
}

GOMCollector::GOMCollector()
{
    createPRTRoot();
    registerDirectory(nullptr, "/system");
    setPropertyBool("/system/GOMActive", true);
    setPropertyString("/system/GOMVersion", COCOA_GOM_VERSION);
    setPropertyInt("/system/StandardOutput", 1);
    setPropertyInt("/system/StandardInput", 0);
    setPropertyInt("/system/StandardError", 2);
}

GOMCollector::~GOMCollector()
{
    collectAll();
    removePRTNode(mPRTRoot);
}

void GOMCollector::addObject(QObject* obj)
{
    mObjs.push(obj);
}

void GOMCollector::collectAll()
{
    while (!mObjs.empty())
    {
        delete mObjs.top();
        mObjs.pop();
    }
}

void GOMCollector::createPRTRoot()
{
    mPRTRoot = new PRTNode;

    mPRTRoot->parent = nullptr;
    mPRTRoot->owner = nullptr;
    mPRTRoot->ownerStr = "PRT-Root";
    mPRTRoot->nodeType = PRTNodeType::PRT_NODE_ROOT;
    mPRTRoot->nodeAttribute = PRT_ATTR_ACCESS;

    std::memset(mPRTRoot->name, '\0', GOM_PRT_NAME_BUFFER_SIZE);
}

PRTNode *GOMCollector::createPRTNode(InterpretedGOMPath& interpreted)
{
    PRTNode *prt = mPRTRoot;
    bool found = false;
    for (size_t i = 0; i < interpreted.size() - 1; i++)
    {
        found = false;
        for (PRTNode *child : prt->children)
        {
            if (child->nodeType == PRTNodeType::PRT_NODE_DIRECTORY
                && interpreted[i] == child->name
                && child->nodeAttribute & PRT_ATTR_ACCESS)
            {
                prt = child;
                found = true;
                break;
            }
        }

        if (!found)
            return nullptr;
    }

    /* Create a new node */
    PRTNode *newNode = new PRTNode;
    newNode->parent = prt;
    prt->children.push_back(newNode);
    std::memset(&newNode->content, 0, sizeof(newNode->content));

    return newNode;
}

PRTNode *GOMCollector::getPRTNode(InterpretedGOMPath& interpreted)
{
    PRTNode *prt = mPRTRoot;
    bool found = false;
    for (size_t i = 0; i < interpreted.size(); i++)
    {
        found = false;
        for (PRTNode *child : prt->children)
        {
            if (interpreted[i] == child->name
                && child->nodeAttribute & PRT_ATTR_ACCESS)
            {
                prt = child;
                found = true;
                break;
            }
        }

        if (!found)
            return nullptr;
    }

    return prt;
}

static void remove_prt_node_without_parent(PRTNode *node)
{
    for (PRTNode *p : node->children)
        remove_prt_node_without_parent(p);

    if (node->nodeType == PRTNodeType::PRT_NODE_PROPERTY
        && node->dataType == PRTDataType::PRT_DATA_STRING)
        delete[] node->content._str;
    delete node;
}

void GOMCollector::removePRTNode(PRTNode *node)
{
    PRTNode *parent = node->parent;
    if (parent)
    {
        for (PRTNode *p : parent->children)
        {
            if (p == node) {
                parent->children.remove(node);
                break;
            }
        }
    }

    remove_prt_node_without_parent(node);
}

void GOMCollector::interpretPropertyPath(const char *path, InterpretedGOMPath& out)
{
    std::string buf;
    const char *p = path;

    while (*p != '\0')
    {
        if (*p == '/')
        {
            if (buf.size() > 0)
                out.push_back(buf);
            buf.clear();
            goto next;
        }

        buf += *p;

    next:
        p++;
    }

    if (buf.size() > 0)
        out.push_back(buf);
}

void GOMCollector::registerDirectory(QObject *obj, const char *dir, int attri)
{
    InterpretedGOMPath path;
    interpretPropertyPath(dir, path);

    if (getPRTNode(path))
        throw std::runtime_error("Directory exists");

    PRTNode *p = createPRTNode(path);
    if (!p)
        throw std::runtime_error("No such directory");

    p->nodeType = PRTNodeType::PRT_NODE_DIRECTORY;
    if (attri & PRT_ATTR_INHERIT)
        p->nodeAttribute = p->parent->nodeAttribute;
    else
        p->nodeAttribute = attri;
    p->owner = obj;

    std::strncpy(p->name, path.back().c_str(), GOM_PRT_NAME_BUFFER_SIZE);
}

bool GOMCollector::hasPropertyOrDirectory(char const *path)
{
    InterpretedGOMPath ipath;
    interpretPropertyPath(path, ipath);
    return (getPRTNode(ipath) != nullptr);
}

PRTNode *GOMCollector::__generic_set_property(const char *__path, int attri)
{
    InterpretedGOMPath path;
    interpretPropertyPath(__path, path);

    PRTNode *current;
    current = getPRTNode(path);
    if (!current)
    {
        current = createPRTNode(path);
        if (!current)
            throw std::runtime_error("No such property or directory");
        std::strncpy(current->name, path.back().c_str(), GOM_PRT_NAME_BUFFER_SIZE);

        current->nodeType = PRTNodeType::PRT_NODE_PROPERTY;
    }
    else if (current->nodeType != PRTNodeType::PRT_NODE_PROPERTY)
        throw std::runtime_error("Not a property, is a directory");

    if (attri & PRT_ATTR_INHERIT)
        current->nodeAttribute = current->parent->nodeAttribute;
    else
        current->nodeAttribute = attri;
    current->owner = current->parent->owner;
    return current;
}

PRTNode *GOMCollector::__generic_get_property(const char *__path, PRTDataType reqType)
{
    InterpretedGOMPath path;
    interpretPropertyPath(__path, path);

    PRTNode *current;
    current = getPRTNode(path);
    if (!current)
        throw std::runtime_error("No such property");

    if (current->dataType != reqType)
        throw std::runtime_error("Unexpected type identifier for PRT tree");

    return current;
}

void GOMCollector::setPropertyInt(const char *path, int64_t data, int attri)
{
    PRTNode *node = __generic_set_property(path, attri);
    node->dataType = PRTDataType::PRT_DATA_INT;
    node->content._int = data;
}

int GOMCollector::getPropertyInt(const char *path)
{
    return __generic_get_property(path, PRTDataType::PRT_DATA_INT)->content._int;
}

void GOMCollector::setPropertyString(const char *path, const char *data, int attri)
{
    PRTNode *node = __generic_set_property(path, attri);
    node->dataType = PRTDataType::PRT_DATA_STRING;
    if (node->content._str)
        delete[] node->content._str;

    size_t len = std::strlen(data);
    node->content._str = new char[len + 1];
    std::memcpy(node->content._str, data, len + 1);
}

char const *GOMCollector::getPropertyString(const char *path)
{
    return __generic_get_property(path, PRTDataType::PRT_DATA_STRING)->content._str;
}

void GOMCollector::setPropertyBool(const char *path, bool data, int attri)
{
    PRTNode *node = __generic_set_property(path, attri);
    node->dataType = PRTDataType::PRT_DATA_BOOL;
    node->content._bool = data;
}

bool GOMCollector::getPropertyBool(const char *path)
{
    return __generic_get_property(path, PRTDataType::PRT_DATA_BOOL)->content._bool;
}

void GOMCollector::setPropertyDouble(const char *path, double data, int attri)
{
    PRTNode *node = __generic_set_property(path, attri);
    node->dataType = PRTDataType::PRT_DATA_DOUBLE;
    node->content._double = data;
}

double GOMCollector::getPropertyDouble(const char *path)
{
    return __generic_get_property(path, PRTDataType::PRT_DATA_DOUBLE)->content._double;
}

void GOMCollector::removePropertyOrDirectory(const char *path)
{
    InterpretedGOMPath ip;
    interpretPropertyPath(path, ip);
    PRTNode *p = getPRTNode(ip);

    if (!p)
        std::runtime_error("No such directory or property");
    removePRTNode(p);
}

static void __dump_properties_tree_recursive(StringVector& out, PRTNode *current, std::vector<std::string>& prefix)
{
    size_t childrenCount = current->children.size();
    size_t i = 0;
    std::ostringstream os;
    // std::ostream& os = std::cout;

    // char context[gom_prt_dump_context_buffer_size];
    for (PRTNode *p : current->children)
    {
        os.str("");
        for (std::string& str : prefix)
            os << str;

        /*
        std::memset(context, '\0', gom_prt_dump_context_buffer_size);
        if (p->dataType == PRTDataType::PRT_DATA_DOUBLE)
            snprintf(context, gom_prt_dump_context_buffer_size, "(real) %f", p->content._double);
        else if (p->dataType == PRTDataType::PRT_DATA_BOOL)
            std::strncpy(context, p->content._bool ? "(boolean) true" : "(boolean) false",
                gom_prt_dump_context_buffer_size);
        else if (p->dataType == PRTDataType::PRT_DATA_STRING)
            snprintf(context, gom_prt_dump_context_buffer_size, "\"%s\"", p->content._str);
        else if (p->dataType == PRTDataType::PRT_DATA_INT)
            snprintf(context, gom_prt_dump_context_buffer_size, "(i64) %ld", p->content._int);
        */

        if (i == childrenCount - 1)
        {
            /* It's the last one */
            os << "└── ";
            prefix.push_back("    ");
        }
        else
        {
            os << "├── ";
            prefix.push_back("│   ");
        }
        os << p->name;

        out.push_back(os.str());

        if (p->nodeType == PRTNodeType::PRT_NODE_DIRECTORY
            || p->nodeType == PRTNodeType::PRT_NODE_ROOT)
            __dump_properties_tree_recursive(out, p, prefix);
        prefix.pop_back();
        i++;
    }
}

void GOMCollector::dumpPropertiesTree(StringVector& os)
{
    std::vector<std::string> prefix;
    os.push_back("/");
    __dump_properties_tree_recursive(os, mPRTRoot, prefix);
}

} // namespace cocoa
