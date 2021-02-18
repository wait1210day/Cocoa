#ifndef COCOA_COMPOSITENODE_H
#define COCOA_COMPOSITENODE_H

#include <memory>

#include "ciallo/GrBase.h"
#include "ciallo/DDR/GrBaseCompositor.h"
#include "ciallo/DIR/BaseNode.h"
CIALLO_BEGIN_NS

class CompositeNode : public BaseNode
{
public:
    CIALLO_STATIC_NODE_KIND(NodeKind::kCompositeNode)

    explicit CompositeNode(std::shared_ptr<GrBaseCompositor>&& compositor);
    ~CompositeNode() override = default;

    std::shared_ptr<GrBaseCompositor> asCompositor();

    NodeBackendKind backendKind() const override;

private:
    std::shared_ptr<GrBaseCompositor>       fCompositor;
};

CIALLO_END_NS
#endif //COCOA_COMPOSITENODE_H
