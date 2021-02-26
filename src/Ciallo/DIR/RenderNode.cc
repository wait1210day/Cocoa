#include <memory>
#include <utility>

#include "Ciallo/GrBase.h"
#include "Ciallo/DDR/GrBaseRenderLayer.h"
#include "Ciallo/DIR/RenderNode.h"
#include "Ciallo/DIR/CompositeNode.h"
#include "Ciallo/DIR/PaintNode.h"
CIALLO_BEGIN_NS

RenderNode *RenderNode::MakeFromParent(BaseNode *parent,
                                           const std::string& id,
                                           int32_t width,
                                           int32_t height,
                                           int32_t x, int32_t y,
                                           int zindex)
{
    if (parent == nullptr)
        return nullptr;

    auto *compositeNode = parent->cast<CompositeNode>();

    std::shared_ptr<GrBaseRenderLayer> renderLayer
        = compositeNode->asCompositor()->newRenderLayer(width, height, x, y, zindex);
    if (renderLayer == nullptr)
        return nullptr;

    return new RenderNode(parent, id, std::move(renderLayer));
}

RenderNode::RenderNode(BaseNode *parent,
                           std::string id,
                           std::shared_ptr<GrBaseRenderLayer>&& layer)
    : BaseNode(NodeKind::kRenderNode, parent),
      fNodeId(std::move(id)),
      fRenderLayer(layer)
{
}

std::string RenderNode::nodeID() const
{
    return fNodeId;
}

std::shared_ptr<GrBaseRenderLayer> RenderNode::asRenderLayer()
{
    return fRenderLayer;
}

void RenderNode::update()
{
    for (auto *child : this->children())
    {
        auto *paintNode = child->cast<PaintNode>();
        sk_sp<SkPicture> picture = paintNode->asPicture();
        if (picture == nullptr)
            continue;

        if (paintNode->left() >= fRenderLayer->width() ||
            paintNode->top() >= fRenderLayer->height())
            continue;

        fRenderLayer->paint(picture, paintNode->left(), paintNode->top(), nullptr);
    }
    fRenderLayer->update();
}

CIALLO_END_NS
