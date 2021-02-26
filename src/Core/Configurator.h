#ifndef COCOA_CONFIGURATOR_H
#define COCOA_CONFIGURATOR_H

#include <string>
#include <vector>

#include "Core/PropertyTree.h"

namespace cocoa {

class Configurator
{
public:
    Configurator();

    enum class State
    {
        kSuccessful,
        kShouldExitNormally,
        kError
    };

    State parse(int argc, char const **argv);

private:
    std::vector<std::string>    fCmdOverrides;
    std::string                 fJSONFile;
    PropertyTreeDirNode        *fpNode;
};

}

#endif //COCOA_CONFIGURATOR_H
