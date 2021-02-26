#ifndef COCOA_GRAPHICSCONTEXT_H
#define COCOA_GRAPHICSCONTEXT_H

#include <memory>
#include <thread>
#include <queue>
#include <condition_variable>

#include "Core/Exception.h"
#include "Ciallo/GrBase.h"
#include "Ciallo/Thread.h"
#include "Ciallo/DDR/GrBasePlatform.h"
#include "Ciallo/DDR/GrBaseCompositor.h"
#include "Ciallo/DDR/GrBaseRenderLayer.h"
#include "Ciallo/DIR/RenderNode.h"
#include "Ciallo/DIR/CompositeNode.h"
CIALLO_BEGIN_NS

/**
 * A GraphicsContext is a instance of Ciallo engine. A single
 * application can only create one GraphicsContext.
 * GraphicsContext will create a thread named Renderer to
 * rasterize and composite (or blend) layers.
 *
 * The rendering of Ciallo engine is based on rendering tree,
 * which has following structure:
 *
 * Composite Node ---------- Root Node
 *                          /   |    \
 *                        /     |      \
 *                      /       |        \
 * Render Node ----- Layer#1  Layer#2 ... Layer#N
 *                   /     \
 *                  /       \
 *                 /         \
 *             Picture#1 ... Picture#N ----+
 *            /                            | Paint nodes
 *       Picture#1  -----------------------+
 *
 * All of the nodes are created in heap, and will be destroyed
 * automatically. That means a node will destroy its child nodes
 * when it is destroyed. So we only need to delete "Root Node"
 * then the whole rendering tree will be destroyed. Each node
 * has its own size of canvas and the position in the father node.
 */
class GraphicsContext
{
public:
    explicit GraphicsContext(std::unique_ptr<GrBasePlatform> platform);
    ~GraphicsContext();


    std::shared_ptr<Thread::Fence> emitCmdPresent();
    std::shared_ptr<Thread::Fence> emitCmdRenderNodeUpdate(RenderNode *renderNode);
    std::shared_ptr<Thread::Fence> emitCmdTightenResources();

    RenderNode *createRenderNode(const std::string& identifier,
                                 int32_t width, int32_t height,
                                 int32_t x, int32_t y,
                                 int zindex);

    CompositeNode *asNode()
    { return fRootNode.get(); }

    GrBasePlatform *asPlatform()
    { return fPlatform.get(); }

private:
    std::unique_ptr<GrBasePlatform>     fPlatform;
    std::unique_ptr<CompositeNode>      fRootNode;
    Worker                             *fRenderWorker;
    Thread                             *fRendererThread;
};

CIALLO_END_NS
#endif //COCOA_GRAPHICSCONTEXT_H
