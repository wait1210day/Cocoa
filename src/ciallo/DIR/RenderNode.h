#ifndef COCOA_RENDERNODE_H
#define COCOA_RENDERNODE_H

#include <memory>

#include "ciallo/GrBase.h"
#include "ciallo/GrDefinitions.h"
#include "ciallo/DIR/BaseNode.h"
#include "ciallo/DDR/GrBaseRenderLayer.h"
CIALLO_BEGIN_NS

class RenderNode : public BaseNode
{
public:
    CIALLO_STATIC_NODE_KIND(NodeKind::kRenderNode)

    static RenderNode *MakeFromParent(BaseNode *parent,
                                      const std::string& id,
                                      int32_t width, int32_t height,
                                      int32_t x, int32_t y,
                                      int zindex);

    ~RenderNode() override = default;

    std::string nodeID() const;
    std::shared_ptr<GrBaseRenderLayer> asRenderLayer();
    void update();

private:
    RenderNode(BaseNode *parent, std::string id,
               std::shared_ptr<GrBaseRenderLayer>&& layer);

private:
    std::string                         fNodeId;
    std::shared_ptr<GrBaseRenderLayer>  fRenderLayer;
};

CIALLO_END_NS
#endif //COCOA_RENDERNODE_H
