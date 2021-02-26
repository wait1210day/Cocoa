#ifndef COCOA_UTILS_H
#define COCOA_UTILS_H

#include <functional>

#include "Core/PropertyTree.h"
#include "Core/Exception.h"
namespace cocoa::utils {

using Printer = std::function<void(const std::string&)>;

void DumpPropertyTree(PropertyTreeNode *pRoot, Printer printer);
void DumpRuntimeException(const RuntimeException& except, bool color, Printer printer);

void ChangeWorkDirectory(const std::string& dir);

} // namespace cocoa

#endif //COCOA_UTILS_H
