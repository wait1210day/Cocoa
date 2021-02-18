#include <vector>
#include <cstring>
#include <ostream>
#include <iostream>
#include <memory>
#include <fstream>
#include <cxxabi.h>
#include <typeinfo>

#include "Project.h"
#include "core/Configurator.h"

namespace cocoa
{

GOM_DEFINE(Config)
Config::Config(GOMCollector *collector)
    : Throwable("cocoa::Config"), mpCollector(collector),
      mNeedExit(false)
{
    bool gomIsActive = mpCollector->getPropertyBool("/system/GOMActive");
    if (!gomIsActive)
    {
        artfmt(ARTEXCEPT_THROWER_CONSTRUCTOR) << "GOM subsystem isn\'t active";
        arthrow<ARTException>(new ARTException());
    }

    mpCollector->registerDirectory(this, "/config", PRT_ATTR_INHERIT);
    cfgExpandObjectTemplateToSimpleEntry(&CfgRootObjectTemplate, mSimpleEntries);
    cfgExpandObjectTemplateToJsonPath(&CfgRootObjectTemplate, mExpandedJsonPaths);
    GOM_CONSTRUCT_DONE
}

void Config::printHelp(char const *programName)
{
    std::cout << "Cocoa Quick GUI Framework" << std::endl;
    std::cout << "Usage: " << programName << " [options...] [-declare <name>=<type>:<value>]" << std::endl;
    std::cout << "Usage: " << programName << " [options...] [-declare <name>=<@type>:<values...>]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -help                   Print this and then exit." << std::endl;
    std::cout << "  -version                Print information about version and copyright." << std::endl;
    std::cout << "  -config <file>          Specify a JSON file as configuration." << std::endl;
    std::cout << "  -declare <declaration>  Specify a configuration entry." << std::endl;
    std::cout << std::endl;
    std::cout << "Configuration Declaration:" << std::endl;
    std::cout << "  Basic declaration: <name>=<type>:<value>      | Example: test.val=boolean:true" << std::endl;
    std::cout << "  Array declaration: <name>=<@type>:<v1,v2,...> | Example: test.aval=@number:1,2,3" << std::endl;
}

void Config::printVersion()
{
    std::cout << "Cocoa Quick GUI Framework" << std::endl;
    std::cout << "Copyright(C) <masshiro.io@qq.com> 2020" << std::endl;
    std::cout << COCOA_VERSION << std::endl;
    std::cout << "  " << COCOA_FRAMEWORK_VERSION << std::endl;
}

#define if_match(a, b) if (!std::strcmp(a, b))
#define else_if_match(a, b) else if (!std::strcmp(a, b))
void Config::parseCLIObjectSyntax(char const *__str)
{
    /* Make the constant memory writable */
    std::unique_ptr<char> str(new char[std::strlen(__str) + 1]);
    std::strcpy(str.get(), __str);

    char *begin = str.get();
    char *end = getCLIObjectSyntaxName(str.get());
    *(end + 1) = '\0';
    std::string entryName(begin);

    begin = end + 2;
    if (*begin == '\0')
    {
        artfmt(__FUNCTION__) << "Argument \"" << __str << "\": Syntax error";
        arthrow<ARTException>(new ARTException());
    }

    end = getCLIObjectSyntaxTypeIdentifier(begin);
    *(end + 1) = '\0';
    bool isArray = (*begin == '@');
    if (isArray) begin++;

    ConfigDataTypes type = kConfigConstraintNumber;
    if_match(begin, "boolean")
        type = kConfigConstraintBoolean;
    else_if_match(begin, "number")
        type = kConfigConstraintNumber;
    else_if_match(begin, "string")
        type = kConfigConstraintString;
    else_if_match(begin, "file")
        type = kConfigConstraintFilePath;
    else_if_match(begin, "cocoa")
        type = kConfigConstraintCocoaPath;
    else
    {
        artfmt(__FUNCTION__) << "Argument \"" << __str << "\": Invalid type identifier \""
            << (isArray ? (begin - 1) : begin) << '\"';
        arthrow<ARTException>(new ARTException());
    }

    tryMatchEntry(entryName, type, isArray);
    begin = end + 2;
    if (*begin == '\0')
    {
        artfmt(__FUNCTION__) << "Argument \"" << __str << "\": Syntax error";
        arthrow<ARTException>(new ARTException());
    }

    std::size_t arraySize;
    std::unique_ptr<DataVariant> dataVariant(getCLIObjectSyntaxValue(arraySize, begin, type, isArray));
    registerConfigEntryToPropertyTree(entryName, type, isArray, dataVariant, arraySize);

    if (type == kConfigConstraintString || type == kConfigConstraintFilePath
        || type == kConfigConstraintCocoaPath)
    {
        if (isArray)
        {
            for (size_t i = 0; i < arraySize; i++)
            {
                if (dataVariant.get()[i].string)
                    delete dataVariant.get()[i].string;
            }
        }
        else if (dataVariant.get()->string)
            delete dataVariant.get()->string;
    }
}

char *Config::getCLIObjectSyntaxName(char *p)
{
    char *pStart = p;
    while (*p != '=' && *p != '\0')
        p++;
    
    if (*p == '\0')
    {
        artfmt(__FUNCTION__) << "Argument \"" << pStart << "\": Syntax error, expect \"=\"";
        arthrow<ARTException>(new ARTException());
    }
    if (p == pStart)
    {
        artfmt(__FUNCTION__) << "Argument \"" << pStart << "\": Syntax error, expect an identifier before \"=\"";
        arthrow<ARTException>(new ARTException());
    }
    return p - 1;
}

char *Config::getCLIObjectSyntaxTypeIdentifier(char *p)
{
    char *pStart = p;
    while (*p != ':' && *p != '\0')
        p++;

    if (*p == '\0')
    {
        artfmt(__FUNCTION__) << "Argument \"" << pStart << "\": Syntax error, expect \":\"";
        arthrow<ARTException>(new ARTException());
    }
    if (p == pStart)
    {
        artfmt(__FUNCTION__) << "Argument \"" << pStart << "\": Syntax error, expect an identifier before \":\"";
        arthrow<ARTException>(new ARTException());
    }
    return p - 1;
}

void Config::tryMatchEntry(const std::string& name, int constraint, bool isArray)
{
    const SimpleConfigEntry *pEntry = nullptr;
    for (const SimpleConfigEntry& entry : mSimpleEntries)
    {
        std::string loString;
        bool first = true;
        for (char const *str : entry.objectPath)
        {
            if (first)
                first = false;
            else
                loString += ".";
            loString += str;
        }

        if (loString == name)
        {
            pEntry = &entry;
            break;
        }
    }

    if (pEntry == nullptr)
    {
        artfmt(__FUNCTION__) << "No such configuration entry";
        arthrow<ARTException>(new ARTException());
    }

    if (!(pEntry->constraints & constraint))
    {
        artfmt(__FUNCTION__) << "Configuration \"" << name << "\": Type mismatch";
        arthrow<ARTException>(new ARTException());
    }

    if (pEntry->array != isArray)
    {
        artfmt(__FUNCTION__) << "Configuration \"" << name << "\": Array flag mismatch";
        arthrow<ARTException>(new ARTException());
    }
}

Config::DataVariant *Config::getCLIObjectSyntaxValue(std::size_t& arraySize, char *str, ConfigDataTypes typeConstraint, bool isArray)
{
    arraySize = 0;
    if (isArray) {
        return parseCLIObjectArrayValue(arraySize, str, typeConstraint);
    }
    else {
        return parseCLIObjectValue(str, typeConstraint);
    }
}

Config::DataVariant *Config::parseCLIObjectArrayValue(std::size_t& arraySize, char *str, ConfigDataTypes type)
{
    std::vector<DataVariant> data;
    char *p = str;
    bool supress = false;
    char supressCondition;
    std::string supressBuffer;
    std::string buffer;

    do
    {
        if (supress)
        {
            if (*p == supressCondition && *(p - 1) != '\\')
                supress = false;
            else
                supressBuffer.push_back(*p);
        }
        else if (*p == '\'' || *p == '\"')
        {
            supressBuffer.clear();
            supress = true;
            supressCondition = *p;
        }
        else if (*p == ',' || *p == '\0')
        {
            DataVariant variant;
            if (type == kConfigConstraintString || type == kConfigConstraintFilePath
                || type == kConfigConstraintCocoaPath)
                parseStringAsDataVariant(variant, supressBuffer, type);
            else
                parseStringAsDataVariant(variant, buffer, type);

            data.push_back(variant);
            buffer.clear();
        }
        else
        {
            buffer.push_back(*p);
        }
    } while (*p++ != '\0');

    DataVariant *pVariants = new DataVariant[data.size()];
    for (size_t i = 0; i < data.size(); i++)
        pVariants[i] = data[i];
    
    arraySize = data.size();
    return pVariants;
}

Config::DataVariant *Config::parseCLIObjectValue(char *str, ConfigDataTypes type)
{
    DataVariant *pVariant = new DataVariant;
    parseStringAsDataVariant(*pVariant, str, type);

    return pVariant;
}

void Config::parseStringAsDataVariant(Config::DataVariant& data, const std::string& str, ConfigDataTypes type)
{
    data.string = nullptr;
    // std::cout << "parse \"" << str << "\" as type #" << type << std::endl;

    if (str.empty())
    {
        artfmt(__FUNCTION__) << "Empty buffer";
        arthrow<ARTException>(new ARTException());
    }

    auto isNumber = [](const std::string& s) -> bool {
        bool neg = false;
        bool pt = false;
        for (char ch : s)
        {
            if (ch == '-')
            {
                if (neg)
                    return false;
                else
                    neg = true;
            }
            else if (ch == '.')
            {
                if (pt)
                    return false;
                else
                    pt = true;
            }
            else if (!std::isdigit(ch))
                return false;
        }
        return true;
    };

    switch (type)
    {
    case kConfigConstraintBoolean:
        if (str == "True" || str == "true")
            data.boolean = true;
        else if (str == "False" || str == "false")
            data.boolean = false;
        else
        {
            artfmt(__FUNCTION__) << "Invalid boolean value \"" << str << "\"";
            arthrow<ARTException>(new ARTException());
        }
        break;
    case kConfigConstraintNumber:
        if (!isNumber(str))
        {
            artfmt(__FUNCTION__) << "Invalid number value \"" << str << "\"";
            arthrow<ARTException>(new ARTException());
        }
        data.number = std::atof(str.c_str());
        break;
    case kConfigConstraintString:
    case kConfigConstraintFilePath:
    case kConfigConstraintCocoaPath:
        data.string = new char[str.size()];
        std::strncpy(data.string, str.c_str(), str.size());
        break;
    default:
        /* WTF??? */
        artfmt(__FUNCTION__) << "InternalError: Unexpected enumeration value, a bug or broken compiler?";
        arthrow<ARTException>(new ARTException());
        break;
    }
}

void Config::registerConfigEntryToPropertyTree(const std::string& entryName, ConfigDataTypes typeConstraint, bool isArray,
        std::unique_ptr<DataVariant>& data, int arraySize)
{
    std::vector<std::string> prtPath;
    std::string buffer;
    for (char ch : entryName)
    {
        if (ch == '.')
        {
            prtPath.push_back(buffer);
            buffer.clear();
        }
        else
        {
            buffer.push_back(ch);
        }
    }
    if (buffer.empty())
    {
        artfmt(__FUNCTION__) << "InternalError: Empty buffer, maybe bug or broken compiler?";
        arthrow<ARTException>(new ARTException());
    }
    if (isArray)
        prtPath.push_back(buffer);
    
    std::string path("/config");
    for (const std::string& str : prtPath)
    {
        path += std::string("/") + str;
        if (!mpCollector->hasPropertyOrDirectory(path.c_str()))
        {
            mpCollector->registerDirectory(this, path.c_str(), PRTNodeAttribute::PRT_ATTR_INHERIT);
            mpCollector->setNodeUserdata(path.c_str(), kConfigConstraintObject);
        }
    }

    auto addToPropertiesTree = [&typeConstraint](DataVariant& variant, const std::string& destPath, GOMCollector *collector) -> void {
        switch (typeConstraint)
        {
        case kConfigConstraintBoolean:
            collector->setPropertyBool(destPath.c_str(), variant.boolean);
            break;
        case kConfigConstraintNumber:
            collector->setPropertyDouble(destPath.c_str(), variant.number);
            break;
        case kConfigConstraintString:
        case kConfigConstraintFilePath:
        case kConfigConstraintCocoaPath:
            collector->setPropertyString(destPath.c_str(), variant.string);
            break;
        default:
            break;
        }
        collector->setNodeUserdata(destPath.c_str(), typeConstraint);
    };

    if (isArray)
    {
        mpCollector->setNodeUserdata(path.c_str(), kConfigConstraintArray);
        for (int i = 0; i < arraySize; i++)
        {
            std::ostringstream ss;
            ss << path << '/' << i;
            addToPropertiesTree(data.get()[i], ss.str(), mpCollector);
        }
    }
    else
    {
        path += std::string("/") + buffer;
        addToPropertiesTree(*data.get(), path, mpCollector);
    }
}

#undef if_match
#undef else_if_match

#define if_match(pstr) if (!std::strcmp(pstr, str))
#define else_if_match(pstr) else if (!std::strcmp(pstr, str))
void Config::parseFromCLI(int argc, char const **argv)
{
    bool recordNext = false;
    char const *recordNextPath = nullptr;
    for (int i = 1; i < argc; i++)
    {
        char const *str = argv[i];

        if (recordNext)
        {
            if (recordNextPath)
            {
                mpCollector->setPropertyString(recordNextPath, str);
                recordNextPath = nullptr;
                recordNext = false;
                continue;
            }
            parseCLIObjectSyntax(str);

            recordNext = false;
            continue;
        }

        if_match("-help")
        {
            mNeedExit = true;
            printHelp(argv[0]);
            return;
        }
        else_if_match("-version")
        {
            mNeedExit = true;
            printVersion();
            return;
        }
        else_if_match("-config")
        {
            recordNext = true;
            recordNextPath = "/config/JSONConfigFile";
        }
        else_if_match("-declare")
        {
            recordNext = true;
        }
        else
        {
            artfmt(__FUNCTION__) << "Invalid option: " << str;
            arthrow<ARTException>(new ARTException());
        }
    }

    if (recordNext)
    {
        artfmt(__FUNCTION__) << "The last option requires an argument";
        arthrow<ARTException>(new ARTException());
    }
}

#define match_type(typ) if (varType == typeid(typ))
#define else_match_type(typ) else match_type(typ)
void Config::JSONDynamicVarProcessor(Poco::Dynamic::Var& var, JsonPathList& path)
{
    if (var.type() == typeid(Poco::JSON::Object::Ptr))
    {
        auto object = var.extract<Poco::JSON::Object::Ptr>();
        JSONObjectRecursiveVisitor(object, path);
    }
    else if (var.type() == typeid(Poco::JSON::Array::Ptr))
    {
        auto array = var.extract<Poco::JSON::Array::Ptr>();
        JSONArrayRecursiveVisitor(array, path);
    }
    else
    {
        /*
        for (auto& str : path)
            std::cout << '/' << str;

        char *realname = abi::__cxa_demangle(var.type().name(), nullptr, nullptr, nullptr);
        std::cout << ": type " << realname << std::endl;
        std::free(realname);
        */
        ConfigDataTypes type = kConfigConstraintNumber;
        DataVariant variant;
        bool needReleaseString = false;
        auto& varType = var.type();

        match_type(long)
        {
            type = kConfigConstraintNumber;
            variant.number = var.convert<long>();
        }
        else_match_type(double)
        {
            type = kConfigConstraintNumber;
            variant.number = var.convert<double>();
        }
        else_match_type(bool)
        {
            type = kConfigConstraintBoolean;
            variant.boolean = var.convert<bool>();
        }
        else_match_type(std::string)
        {
            std::string str = var.convert<std::string>();
            const char *copyStart = nullptr;

            if (!std::strncmp(str.c_str(), "cocoa://", 8))
            {
                type = kConfigConstraintCocoaPath;
                copyStart = str.c_str() + 8;
            }
            else if (!std::strncmp(str.c_str(), "file://", 7))
            {
                type = kConfigConstraintFilePath;
                copyStart = str.c_str() + 7;
            }
            else
            {
                type = kConfigConstraintString;
                copyStart = str.c_str();
            }

            
            variant.string = new char[str.size() + 1];
            std::strncpy(variant.string, copyStart, str.size() + 1);

            needReleaseString = true;
        }
        else
        {
            char *demangleName = abi::__cxa_demangle(varType.name(), nullptr, nullptr, nullptr);
            artfmt(__FUNCTION__) << "RTTI: Unexpected JSON type: " << demangleName;
            std::free(demangleName);
            arthrow<ARTException>(new ARTException());
        }

        try
        {
            tryMatchJSONPath(path, type);
            registerJSONPath(path, type, variant);
        }
        catch (ARTException *e)
        {
            if (needReleaseString)
                delete[] variant.string;
            throw e;
        }

        if (needReleaseString)
            delete[] variant.string;
    }
}
#undef else_match_type
#undef match_type

std::ostream& operator<<(std::ostream& os, const Config::JsonPathList& p)
{
    bool first = true;
    for (const Config::JsonPath& path : p)
    {   
        switch (path.type)
        {
        case Config::JsonPath::JsonPathType::kPath:
            if (first)
                first = false;
            else
                os << '.';
            os << path.path;
            break;
        case Config::JsonPath::JsonPathType::kSubscript:
            os << '[' << path.subscript << ']';
            break;
        }
    }
    return os;
}

void Config::tryMatchJSONPath(JsonPathList& path, ConfigDataTypes typeConstraint)
{
    for (ExpandedJsonPathList& expanded : mExpandedJsonPaths)
    {
        JsonPathList& jpl = expanded.list;
        if (jpl.size() != path.size() || !(expanded.constraints & typeConstraint))
            continue;
        
        for (size_t i = 0; i < jpl.size(); i++)
        {
            if (jpl[i].type != path[i].type)
                continue;
            
            if (jpl[i].type == JsonPath::JsonPathType::kPath && jpl[i].path != path[i].path)
                continue;
            return;
        }
    }

    artfmt(__FUNCTION__) << "JSON: \"" << path << "\" Invalid configuration entry";
    arthrow<ARTException>(new ARTException());
}

void Config::JSONArrayRecursiveVisitor(Poco::JSON::Array::Ptr JsonArray, JsonPathList& path)
{
    int i = 0;

    for (Poco::Dynamic::Var var : *JsonArray)
    {
        path.push_back(JsonPath(i++));

        JSONDynamicVarProcessor(var, path);
        path.pop_back();
    }
}

void Config::JSONObjectRecursiveVisitor(Poco::JSON::Object::Ptr JsonObject, JsonPathList& path)
{
    for (auto& pair : *JsonObject)
    {
        Poco::Dynamic::Var& var = pair.second;
        
        path.push_back(JsonPath(pair.first));
        JSONDynamicVarProcessor(var, path);
        path.pop_back();
    }
}

void Config::registerJSONPath(JsonPathList& path, ConfigDataTypes typeConstraint, DataVariant& variant)
{
    auto addToPropertiesTree = [typeConstraint, &variant](const std::string& str, GOMCollector *collector) -> void {
        switch (typeConstraint)
        {
        case kConfigConstraintBoolean:
            collector->setPropertyBool(str.c_str(), variant.boolean);
            break;
        case kConfigConstraintNumber:
            collector->setPropertyDouble(str.c_str(), variant.number);
            break;
        case kConfigConstraintString:
        case kConfigConstraintCocoaPath:
        case kConfigConstraintFilePath:
            collector->setPropertyString(str.c_str(), variant.string);
            break;
        default:
            break;
        }

        collector->setNodeUserdata(str.c_str(), typeConstraint);
    };

    std::ostringstream ss;
    std::string str;

    ss << "/config";
    for (size_t i = 0; i < path.size(); i++)
    {
        JsonPath& sp = path[i];

        ss << '/';
        switch (sp.type)
        {
        case JsonPath::JsonPathType::kPath:
            ss << sp.path;
            str = ss.str();
            break;
        case JsonPath::JsonPathType::kSubscript:
            ss << sp.subscript;
            str = ss.str();
            break;
        }

        if (mpCollector->hasPropertyOrDirectory(str.c_str()))
            continue;

        if (i < path.size() - 1)
        {
            /* Not the last one */
            mpCollector->registerDirectory(this, str.c_str());
            if (path[i + 1].type == JsonPath::JsonPathType::kSubscript)
                mpCollector->setNodeUserdata(str.c_str(), kConfigConstraintArray);
            else
                mpCollector->setNodeUserdata(str.c_str(), kConfigConstraintObject);
        }
        else
        {
            /* It's the last one */
            addToPropertiesTree(str.c_str(), mpCollector);
        }
    }
}

void Config::parseFromJSON(char const *file)
{
    std::ifstream fs(file, std::ios_base::binary);
    if (!fs.is_open())
    {
        artfmt(__FUNCTION__) << "Couldn\'t open configuration file: " << std::strerror(errno);
        arthrow<ARTException>(new ARTException());
    }

    try
    {
        Poco::JSON::Parser parser;
        parser.parse(fs);
        Poco::Dynamic::Var parsedJsonResult = parser.result();

        Poco::JSON::Object::Ptr JsonObject = parsedJsonResult.extract<Poco::JSON::Object::Ptr>();
        JsonPathList path;
        JSONObjectRecursiveVisitor(JsonObject, path);
    }
    catch (Poco::JSON::JSONException& except)
    {
        artfmt(__FUNCTION__) << except.displayText();
        arthrow<ARTException>(new ARTException());
    }
    catch (Poco::Exception& except)
    {
        artfmt(__FUNCTION__) << "PocoError: " << except.displayText();
        arthrow<ARTException>(new ARTException());
    }

    if (!GOM->hasPropertyOrDirectory("/config/type"))
    {
        std::cerr << "Configuration file missing a valid signature field." << std::endl;
        std::cerr << "See also: " << PROJ_DOCUMENT_REF_STR(PROJ_DOC_USER, 2, 1.2, 020102) << std::endl;
        throw std::runtime_error("Corrupted configuration");
    }
    const char *type = GOM->getPropertyString("/config/type");
    if (std::strcmp(type, PROJ_CONF_SIGNATURE))
    {
        std::cerr << "Configuration file missing a valid signature field." << std::endl;
        std::cerr << "See also: " << PROJ_DOCUMENT_REF_STR(PROJ_DOC_USER, 2, 1.2, 020102) << std::endl;
        throw std::runtime_error("Corrupted configuration");
    }
}

} // namespace cocoa
