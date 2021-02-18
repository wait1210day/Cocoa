#include <list>

#include "ciallo/GrBase.h"
#include "ciallo/DIR/BaseNode.h"
CIALLO_BEGIN_NS

BaseNode::ChildrenIterable::ChildrenIterable(std::list<BaseNode *>& node)
    : fBegin(node.begin()),
      fEnd(node.end())
{
}

BaseNode::ChildrenIterable::ChildrenIterable(const ChildrenIterable& that)
    : fBegin(that.fBegin),
      fEnd(that.fEnd)
{
}

std::list<BaseNode *>::iterator BaseNode::ChildrenIterable::begin()
{
    return fBegin;
}

std::list<BaseNode *>::iterator BaseNode::ChildrenIterable::end()
{
    return fEnd;
}

// -----------------------------------------------------------------------------

BaseNode::BaseNode(const std::string& inherit, NodeKind kind, BaseNode *parent)
    : Throwable(inherit.c_str()),
      fKind(kind),
      fBackendKind(NodeBackendKind::kUndefined),
      fParent(parent)
{
    if (parent != nullptr)
        parent->appendChild(this);
}

BaseNode::~BaseNode()
{
    if (fParent)
    {
        fParent = nullptr;
        fParent->removeChild(this);
    }
    for (BaseNode *pChild : fChildrenList)
    {
        pChild->parentDispose();
        delete pChild;
    }
}

void BaseNode::appendChild(const BaseNode *child)
{
    fChildrenList.push_back(const_cast<BaseNode*>(child));
}

void BaseNode::removeChild(const BaseNode *child)
{
    fChildrenList.remove(const_cast<BaseNode*>(child));
}

void BaseNode::parentDispose()
{
    fParent = nullptr;
}

void BaseNode::setParent(BaseNode *parent)
{
    if (fParent)
        fParent->removeChild(this);

    fParent = parent;
    if (fParent)
        fParent->appendChild(this);
}

BaseNode::ChildrenIterable BaseNode::children()
{
    return ChildrenIterable(fChildrenList);
}

void BaseNode::badNodeCast()
{
    artfmt(__FUNCTION__) << "Bad cast: Node kind mismatched";
    arthrow<GrNodeBadCastException>(new GrNodeBadCastException);
}

BaseNode::NodeBackendKind BaseNode::backendKind() const
{
    if (fBackendKind != NodeBackendKind::kUndefined)
        return fBackendKind;

    if (fParent == nullptr)
        return NodeBackendKind::kUnknown;
    return fParent->backendKind();
}

CIALLO_END_NS
