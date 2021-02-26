#define TEST_CIALLO         1

#include <xcb/xcb.h>

#include "Core/Utils.h"
#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Core/Configurator.h"
#include "Core/MeasuredTable.h"

#include "Ciallo/DDR/GrXcbPlatform.h"
#include "Ciallo/DIR/PaintNode.h"
#include "Ciallo/GraphicsContext.h"
#include "Ciallo/XCBWindow.h"

#if TEST_CIALLO
#include "include/core/SkPicture.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/codec/SkCodec.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/core/SkVertices.h"
#endif

#include <memory>
#include <iostream>
#include <Poco/Stopwatch.h>
#include <gperftools/profiler.h>

namespace cocoa
{

constexpr uint32_t WIDTH = 1270;
constexpr uint32_t HEIGHT = 720;

void draw(ciallo::PaintNode *node, const sk_sp<SkShader>& shader)
{
    ciallo::PaintNode::ScopedPaint scopedPaint(node);

    SkCanvas *canvas = node->asCanvas();

    static SkScalar px = 128;
    static int direction = 1;
    SkScalar py = ((px * px) / -400) + 2 * px;

    SkPaint paint;
    const SkScalar R = 60.0f;
    SkPath path;
    path.moveTo(px + R, py);
    for (int i = 1; i < 15; ++i)
    {
        SkScalar a = 0.44879895f * i;
        SkScalar r = R + R * (i % 2);
        path.lineTo(px + r * cos(a), py + r * sin(a));
    }

    paint.setPathEffect(SkDiscretePathEffect::Make(10.0f, 4.0f));
    SkPoint points[2] = {SkPoint::Make(0.0f, 0.0f), SkPoint::Make(node->width() - 1, node->height() - 1)};
    SkColor colors[2] = {SkColorSetRGB(66, 133, 244), SkColorSetRGB(15, 157, 88)};
    paint.setShader(
        SkGradientShader::MakeLinear(points, colors, NULL, 2, SkTileMode::kClamp, 0, NULL));
    paint.setAntiAlias(true);
    paint.setAlphaf(0.7);
    canvas->clear(0x9fffffff);

    canvas->drawPath(path, paint);
    px += direction;
    if (px > node->width() - 128)
        direction = -1;
    else if (px < 128.0f)
        direction = 1;
}

void displayFps()
{
    static auto start = std::chrono::steady_clock::now();
    static double cnt = 0;

    auto now = std::chrono::steady_clock::now();
    double sec = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() / 1e3;
    if (sec >= 1)
    {
        std::cout << cnt / sec << " fps" << std::endl;
        cnt = 0;
        start = std::chrono::steady_clock::now();
    }
    cnt++;
}

void xcbRender()
{
    using namespace ciallo;

    XCBWindow window(nullptr, WIDTH, HEIGHT);
    window.createWindow();
    window.setWindowTitle("Ciallo Test");

    RenderNode *renderNode = window.GContext()->createRenderNode("#front",
                                                                 800, 600, 50, 20, 1);

    PaintNode *paintNode = PaintNode::MakeFromParent(renderNode, 800, 600, 0, 0);
    renderNode->asRenderLayer()->setVisibility(true);

    while (!window.isClosed())
    {
        draw(paintNode, nullptr);
        window.GContext()->emitCmdRenderNodeUpdate(renderNode)->wait();
        window.update();
    }
}

Configurator::State Initialize(int argc, char const **argv)
{
    PropertyTree::New();
    PropertyTree *prop = PropertyTree::Instance();

    Configurator conf;
    Configurator::State state = conf.parse(argc, argv);
    if (state == Configurator::State::kShouldExitNormally ||
        state == Configurator::State::kError)
        return state;

    std::string level = prop->asNode("/runtime/journal/level")
                            ->cast<PropertyTreeDataNode>()->extract<std::string>();

    int filter;
    if (level == "debug")
        filter = LogLevel::LOG_LEVEL_DEBUG;
    else if (level == "normal")
        filter = LogLevel::LOG_LEVEL_NORMAL;
    else if (level == "quiet")
        filter = LogLevel::LOG_LEVEL_QUIET;
    else if (level == "silent")
        filter = LogLevel::LOG_LEVEL_SILENT;
    else if (level == "disabled")
        filter = LogLevel::LOG_LEVEL_DISABLED;
    else
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Unknown log level: ")
                .append(level)
                .make<RuntimeException>();
    }

    bool rainbow = prop->asNode("/runtime/journal/textShader")
            ->cast<PropertyTreeDataNode>()->value().extract<bool>();

    std::string redirect = prop->asNode("/runtime/journal/stdout")
                               ->cast<PropertyTreeDataNode>()->value().extract<std::string>();
    if (redirect == "<stdout>")
        Journal::New(STDOUT_FILENO, filter, rainbow);
    else if (redirect == "<stderr>")
        Journal::New(STDERR_FILENO, filter, rainbow);
    else
        Journal::New(redirect.c_str(), filter, rainbow);

    return Configurator::State::kSuccessful;
}

void Finalize()
{
    Journal::Delete();
    PropertyTree::Delete();
}

void Run()
{
    log_write(LOG_DEBUG) << "Content of property tree:" << log_endl;
    utils::DumpPropertyTree(PropertyTree::Instance()->asNode("/"), [](const std::string& str) -> void {
        log_write(LOG_DEBUG) << str << log_endl;
    });

    xcbRender();
}

int Main(int argc, char const **argv)
{
    try
    {
        Configurator::State state = Initialize(argc, argv);
        if (state == Configurator::State::kError)
            return 1;
        if (state == Configurator::State::kShouldExitNormally)
            return 0;

        Run();
        Finalize();
    }
    catch (const RuntimeException& e)
    {
        if (Journal::Instance())
        {
            bool color = PropertyTree::Instance()->asNode("/runtime/journal/exceptionTextShader")
                            ->cast<PropertyTreeDataNode>()->extract<bool>();
            utils::DumpRuntimeException(e, color, [](const std::string& str) -> void {
                log_write(LOG_EXCEPTION) << str << log_endl;
            });
        }
        else
        {
            utils::DumpRuntimeException(e, false, [](const std::string& str) -> void {
                std::cerr << str << std::endl;
            });
        }
    }

    return 0;
}

} // namespace cocoa

int main(int argc, char const *argv[])
{
    return cocoa::Main(argc, argv);
}
