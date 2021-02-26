#include "Core/Journal.h"
#include "Ciallo/GraphicsContext.h"
CIALLO_BEGIN_NS

enum GCMDOpcodes
{
    kGCMD_Composite_Present = 1,
    kGCMD_Layer_Update,
    kGCMD_Tighten_Resources
};

class RenderWorker : public Worker
{
public:
    explicit RenderWorker(GraphicsContext *ctx) : fContext(ctx) {}
    ~RenderWorker() override = default;

    Thread::CmdExecuteResult execute(const Thread::Command& cmd) override
    {
        switch (cmd.opcode())
        {
        case kGCMD_Composite_Present:
            GCMD_Composite_Present();
            return Thread::CmdExecuteResult::kNormal;

        case kGCMD_Layer_Update:
            GCMD_Layer_Update(cmd.userdata().extract<RenderNode*>());
            return Thread::CmdExecuteResult::kNormal;

        case kGCMD_Tighten_Resources:
            GCMD_Tighten_Resources();
            return Thread::CmdExecuteResult::kNormal;
        }

        log_write(LOG_WARNING) << "<RenderWorker> Unknown command opcode: " << cmd.opcode() << log_endl;
        return Thread::CmdExecuteResult::kNormal;
    }

private:
    void GCMD_Composite_Present()
    {
        fContext->asNode()->asCompositor()->present();
    }

    void GCMD_Layer_Update(RenderNode *renderNode)
    {
        for (BaseNode *child : fContext->asNode()->children())
        {
            auto *renderChild = child->cast<RenderNode>();
            if (renderChild == renderNode)
            {
                renderChild->update();
                return;
            }
        }
        log_write(LOG_ERROR) << "<RenderWorker> Try executing GCMD_Layer_Update:" << log_endl;
        log_write(LOG_ERROR) << "<RenderWorker>   Given layer is not owned by current GraphicsContext" << log_endl;
    }

    void GCMD_Tighten_Resources()
    {
        /* TODO: Implement this */
    }

private:
    GraphicsContext     *fContext;
};

GraphicsContext::GraphicsContext(std::unique_ptr<GrBasePlatform> platform)
    : fPlatform(std::move(platform)),
      fRenderWorker(nullptr),
      fRendererThread(nullptr)
{
    if (fPlatform == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Not a valid platform handle")
                .make<RuntimeException>();
    }
    fRootNode = std::make_unique<CompositeNode>(fPlatform->compositor());

    fRenderWorker = new RenderWorker(this);
    fRendererThread = new Thread("Renderer", fRenderWorker);
}

GraphicsContext::~GraphicsContext()
{
    delete fRendererThread;
    delete fRenderWorker;
}

RenderNode *GraphicsContext::createRenderNode(const std::string& identifier,
                                              int32_t width, int32_t height,
                                              int32_t x, int32_t y, int zindex)
{
    RenderNode *renderNode = RenderNode::MakeFromParent(fRootNode.get(), identifier,
                                                        width, height, x, y, zindex);
    return renderNode;
}

std::shared_ptr<Thread::Fence> GraphicsContext::emitCmdPresent()
{
    return fRendererThread->enqueueCmd(Thread::Command(kGCMD_Composite_Present,
                                                       Poco::Dynamic::Var()));
}

std::shared_ptr<Thread::Fence> GraphicsContext::emitCmdRenderNodeUpdate(RenderNode *renderNode)
{
    return fRendererThread->enqueueCmd(Thread::Command(kGCMD_Layer_Update,
                                                       Poco::Dynamic::Var(renderNode)));
}

std::shared_ptr<Thread::Fence> GraphicsContext::emitCmdTightenResources()
{
    return fRendererThread->enqueueCmd(Thread::Command(kGCMD_Tighten_Resources,
                                                       Poco::Dynamic::Var()));
}

CIALLO_END_NS
