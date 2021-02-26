#include <iostream>
#include "Core/PropertyTree.h"

using namespace cocoa;

int main()
{
    PropertyTree::New();

    PropertyTreeNode::NewDataNode(PropertyTree::Instance()->asNode("/"), "testdata", 2233);
    PropertyTreeNode::NewDirNode(PropertyTree::Instance()->asNode("/"), "testdir");
    PropertyTreeNode::NewDataNode(PropertyTree::Instance()->asNode("/testdir"), "fuck", 666);

    std::cout << PropertyTree::Instance()->asNode("/testdir/fuck")->cast<PropertyTreeDataNode>()->value().extract<int>()
             << std::endl;

    PropertyTree::Delete();
    return 0;
}
