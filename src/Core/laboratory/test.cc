#include <vector>
#include <iostream>

#include "Configurator.h"

using namespace cocoa;

void inspect_each_node(PropertyTreeNode *pNode, std::vector<std::string>& rec)
{
    rec.push_back(pNode->name());
    for (const std::string& name : rec)
        std::cout << '/' << name;
    std::cout << std::endl;

    for (auto *pChild : pNode->children())
    {
        inspect_each_node(pChild, rec);
    }

    rec.pop_back();
}

int main(int argc, char const **argv)
{
    PropertyTree::New();

    Configurator c;
    c.parse(argc, argv);

    std::vector<std::string> rec;
    inspect_each_node(PropertyTree::Instance()->asNode("/"), rec);

    PropertyTree::Delete();
    return 0;
}