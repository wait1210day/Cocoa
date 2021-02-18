#ifndef __JSON_CONFIG_H__
#define __JSON_CONFIG_H__

#include "core/ObjectHolder.h"
#include "core/Exception.h"

#include <list>
#include <string>
#include <memory>

#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

namespace cocoa
{

enum ConfigDataTypes
{
    kConfigConstraintBoolean    = 0x01,
    kConfigConstraintString     = 0x02,
    kConfigConstraintNumber     = 0x04,
    kConfigConstraintFilePath   = 0x08,
    kConfigConstraintCocoaPath  = 0x10,
    kConfigConstraintArray      = 0x20,
    kConfigConstraintObject     = 0x40
};

struct CfgObjectTemplateElement;

typedef std::vector<CfgObjectTemplateElement> CfgObjectTemplate;
struct CfgObjectTemplateElement
{
    char const *key;
    unsigned int valueTypes;
    /* If valueTypes is an object or an array, this field is available */
    const CfgObjectTemplate *valueTemplate = nullptr;
};

struct SimpleConfigEntry
{
    std::vector<char const*> objectPath;
    bool array;
    int constraints;
};

class Config final : public Throwable, public QObject
{
    GOM_DECLARE(Config)

public:
    union DataVariant
    {
        bool boolean;
        char *string;
        double number;
    };

    class JsonPath
    {
    public:
        JsonPath(const std::string& kpath): type(JsonPathType::kPath), path(kpath) {}
        JsonPath(int kSubscript): type(JsonPathType::kSubscript), subscript(kSubscript) {}
        JsonPath(const JsonPath& p): type(p.type), path(p.path), subscript(p.subscript) {}
        JsonPath() {}

        enum class JsonPathType
        {
            kPath,
            kSubscript
        } type;

        std::string path;
        int subscript;
    };
    typedef std::vector<JsonPath> JsonPathList;

    struct ExpandedJsonPathList
    {
        JsonPathList list;
        ConfigDataTypes constraints;
    };

public:
    Config(GOMCollector *collector);

    void parseFromCLI(int argc, char const **argv);
    void parseFromJSON(char const *file);
    inline bool needExit() { return mNeedExit; }

private:
    void printHelp(char const *programName);
    void printVersion();
    void parseCLIObjectSyntax(char const *str);
    char *getCLIObjectSyntaxName(char *p);
    char *getCLIObjectSyntaxTypeIdentifier(char *p);
    void tryMatchEntry(const std::string& name, int constraint, bool isArray);
    void registerConfigEntryToPropertyTree(const std::string& entryName, ConfigDataTypes typeConstraint, bool isArray,
                                           std::unique_ptr<DataVariant>& data, int arraySize = 0);
    DataVariant *getCLIObjectSyntaxValue(std::size_t& arraySize, char *str, ConfigDataTypes typeConstraint, bool isArray);
    DataVariant *parseCLIObjectArrayValue(std::size_t& arraySize, char *str, ConfigDataTypes type);
    DataVariant *parseCLIObjectValue(char *str, ConfigDataTypes type);
    void parseStringAsDataVariant(DataVariant& data, const std::string& str, ConfigDataTypes type);

    void JSONDynamicVarProcessor(Poco::Dynamic::Var& var, JsonPathList& path);
    void JSONArrayRecursiveVisitor(Poco::JSON::Array::Ptr JsonArray, JsonPathList& path);
    void JSONObjectRecursiveVisitor(Poco::JSON::Object::Ptr JsonObject, JsonPathList& path);
    void registerJSONPath(JsonPathList& path, ConfigDataTypes typeConstraint, DataVariant& variant);
    void tryMatchJSONPath(JsonPathList& path, ConfigDataTypes typeConstraint);

private:
    std::vector<SimpleConfigEntry>      mSimpleEntries;
    std::vector<ExpandedJsonPathList>   mExpandedJsonPaths;
    GOMCollector                 *mpCollector;
    bool                          mNeedExit;
};

std::ostream& operator<<(std::ostream&, const Config::JsonPathList&);

extern const CfgObjectTemplate CfgRootObjectTemplate;
void cfgExpandObjectTemplateToSimpleEntry(const CfgObjectTemplate *root,
                                          std::vector<SimpleConfigEntry>& result);
void cfgExpandObjectTemplateToJsonPath(const CfgObjectTemplate *root,
                                       std::vector<Config::ExpandedJsonPathList>& result);
} // namespace cocoa


#endif /* __JSON_CONFIG_H__ */
