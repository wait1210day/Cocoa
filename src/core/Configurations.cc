#include "core/Configurator.h"

namespace cocoa
{

/**
 * COT (Configuration Object Template) is a structured representation
 * of JSON configuration file in C++. It allows any depth of nesting
 * to be defined and will be expanded into JSON path recursively
 * at runtime.
 */

const CfgObjectTemplate CfgCommonObjectTemplate{
    { "version",            kConfigConstraintNumber     },
    { "traceOnException",   kConfigConstraintBoolean    },
    { "standardOutput",     kConfigConstraintCocoaPath | kConfigConstraintFilePath },
    { "level",              kConfigConstraintString     },
    { "consoleColor",       kConfigConstraintBoolean    }
};

const CfgObjectTemplate CfgFeaturesObjectTemplate{
    { "CMAudioSupport",             kConfigConstraintBoolean },
    { "CMScriptAudioProcess",       kConfigConstraintBoolean },
    { "CMScriptAudioControl",       kConfigConstraintBoolean },
    { "CGHardwareAcceleration",     kConfigConstraintBoolean },
    { "CGHardwareComposite",        kConfigConstraintBoolean }
};

const CfgObjectTemplate CfgRootObjectTemplate{
    { "type",       kConfigConstraintString },
    { "common",     kConfigConstraintObject, &CfgCommonObjectTemplate },
    { "features",   kConfigConstraintObject, &CfgFeaturesObjectTemplate },
};

void cfgExpandObjectTemplateToSimpleEntryRecursive(const CfgObjectTemplate *obj,
                                      std::vector<char const *>& traceback,
                                      std::vector<SimpleConfigEntry>& result)
{
    for (const CfgObjectTemplateElement& element : *obj)
    {
        if (element.valueTypes & kConfigConstraintArray
            && element.valueTypes & kConfigConstraintObject)
            continue;

        traceback.push_back(element.key);
        if (element.valueTypes == kConfigConstraintObject)
            cfgExpandObjectTemplateToSimpleEntryRecursive(element.valueTemplate, traceback, result);
        else
        {
            SimpleConfigEntry entry;
            if (element.valueTypes & kConfigConstraintArray)
            {
                entry.constraints = element.valueTypes & ~kConfigConstraintArray;
                entry.array = true;
            }
            else
            {
                entry.array = false;
                entry.constraints = element.valueTypes;
            }
            
            for (char const *ptr : traceback)
                entry.objectPath.push_back(ptr);
            result.push_back(entry);
        }
        
        traceback.pop_back();
    }
}

void cfgExpandObjectTemplateToSimpleEntry(const CfgObjectTemplate *root,
                                          std::vector<SimpleConfigEntry>& result)
{
    std::vector<char const *> traceback;
    cfgExpandObjectTemplateToSimpleEntryRecursive(root, traceback, result);
}

void cfgExpandObjectTemplateToJsonPathRecursive(const CfgObjectTemplate *root,
                                       std::vector<Config::ExpandedJsonPathList>& result,
                                       Config::JsonPathList& traceback)
{
    for (const CfgObjectTemplateElement& element : *root)
    {
        bool popbackSubscript = false;
        Config::JsonPath path;

        path.type = Config::JsonPath::JsonPathType::kPath;
        path.path = element.key;
        traceback.push_back(path);

        if (element.valueTypes & kConfigConstraintArray)
        {
            path.type = Config::JsonPath::JsonPathType::kSubscript;
            traceback.push_back(path);
            popbackSubscript = true;
        }
        
        if (element.valueTypes & kConfigConstraintObject)
            cfgExpandObjectTemplateToJsonPathRecursive(element.valueTemplate, result, traceback);
        else
        {
            Config::ExpandedJsonPathList expanded;
            expanded.constraints = static_cast<ConfigDataTypes>(element.valueTypes);
            expanded.list = traceback;
            result.push_back(expanded);
        }

        if (popbackSubscript)
            traceback.pop_back();
        traceback.pop_back();
    }
}

void cfgExpandObjectTemplateToJsonPath(const CfgObjectTemplate *root,
                                       std::vector<Config::ExpandedJsonPathList>& result)
{
    Config::JsonPathList traceback;
    cfgExpandObjectTemplateToJsonPathRecursive(root, result, traceback);
}

} // namespace cocoa
