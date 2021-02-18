#define TEST_CIALLO         1

#include <xcb/xcb.h>

#include "core/Journal.h"
#include "core/Exception.h"
#include "core/ObjectHolder.h"
#include "core/Configurator.h"
#include "core/IODispatcher.h"

#include "ciallo/DDR/GrXcbPlatform.h"
#include "ciallo/DIR/PaintNode.h"
#include "ciallo/GraphicsContext.h"

#include "komorebi/css/CSSStylesheet.h"
#include "komorebi/css/CSSSelectionContext.h"
#include "komorebi/css/CSSCalculator.h"
#include "komorebi/KTDLParser.h"

#if TEST_CIALLO
#include "include/core/SkPicture.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPixmap.h"
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

void DumpPropertiesTree()
{
    StringVector sv;
    GOM->dumpPropertiesTree(sv);

    logOut(LOG_DEBUG) << "Properties:" << logEndl;
    for (auto& str : sv)
        logOut(LOG_DEBUG) << str << logEndl;
}

constexpr uint32_t WIDTH = 1270;
constexpr uint32_t HEIGHT = 720;

sk_sp<SkImage> readTextureImage(const std::string& file)
{
    sk_sp<SkData> data = SkData::MakeFromFileName(file.c_str());
    return SkImage::MakeFromEncoded(data);
}

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

void render()
{
    int screenp;
    ::xcb_connection_t *connection = ::xcb_connect(nullptr, &screenp);
    const ::xcb_setup_t *setup = ::xcb_get_setup(connection);
    ::xcb_screen_iterator_t screen_iterator = ::xcb_setup_roots_iterator(setup);
    for (int i = screenp; i > 0; i--)
        ::xcb_screen_next(&screen_iterator);

    ::xcb_screen_t *screen = screen_iterator.data;

    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t mask_values[2] = {
        screen->white_pixel,
        XCB_EVENT_MASK_EXPOSURE
        | XCB_EVENT_MASK_POINTER_MOTION
    };

    ::xcb_window_t window = ::xcb_generate_id(connection);
    ::xcb_create_window(connection,
                        XCB_COPY_FROM_PARENT,
                        window,
                        screen->root,
                        0, 0,
                        WIDTH, HEIGHT,
                        10,
                        XCB_WINDOW_CLASS_INPUT_OUTPUT,
                        screen->root_visual,
                        mask,
                        mask_values);

    ::xcb_map_window(connection, window);
    ::xcb_flush(connection);


    xcb_intern_atom_reply_t *deleteReply = xcb_intern_atom_reply(connection,
                                                                 xcb_intern_atom(connection,
                                                                                 0, 16,
                                                                                 "WM_DELETE_WINDOW"),
                                                                 nullptr);
    xcb_intern_atom_reply_t *protocolReply = xcb_intern_atom_reply(connection,
                                                                   xcb_intern_atom(connection,
                                                                                   1, 12,
                                                                                   "WM_PROTOCOLS"),
                                                                   nullptr);
    xcb_atom_t deleteWindowAtom = deleteReply->atom;
    xcb_atom_t protocolAtom = protocolReply->atom;
    std::free(deleteReply);
    std::free(protocolReply);

    xcb_change_property_checked(connection,
                                XCB_PROP_MODE_REPLACE,
                                window,
                                protocolAtom,
                                XCB_ATOM_ATOM, 32, 1,
                                &deleteWindowAtom);

    ::xcb_expose_event_t xcbExposeEvent{
            .response_type = XCB_EXPOSE,
            .sequence = 0,
            .window = window,
            .x = 0,
            .y = 0,
            .width = WIDTH,
            .height = HEIGHT,
            .count = 1
    };

    ciallo::GrPlatformOptions options{
        .use_gpu_accel = false,
        .use_opencl_accel = true,
        .use_strict_accel = false,
        .vulkan_debug = false
    };

    {
        using namespace ciallo;
        GraphicsContext ctx(GrXcbPlatform::MakeFromXcbWindow(connection, window,
                                                             screenp, options));

        RenderNode *renderNode = ctx.createRenderNode("#front",
                                                      800, 600, 50, 20, -1);
        RenderNode *background = ctx.createRenderNode("#background",
                                                      ctx.asNode()->asCompositor()->width(),
                                                      ctx.asNode()->asCompositor()->height(),
                                                      0, 0, -2);
        PaintNode *paintNode = PaintNode::MakeFromParent(renderNode, 800, 600, 0, 0);

        renderNode->asRenderLayer()->setVisibility(true);
        background->asRenderLayer()->setVisibility(true);
        background->asRenderLayer()->drawImageFile("/home/sora/Pictures/Library/ACG/灵梦.jpg");

        ctx.emitCmdRenderNodeUpdate(background);

        ::xcb_generic_event_t *evt;
        while ((evt = ::xcb_wait_for_event(connection)))
        {
            uint8_t type = evt->response_type & ~0x80;
            if (type == XCB_EXPOSE)
            {
                draw(paintNode, nullptr);

                ctx.emitCmdRenderNodeUpdate(renderNode);
                ctx.emitCmdPresent()->wait();

                ctx.asPlatform()->expose();

                displayFps();

                ::xcb_send_event(connection, true, window,
                                 XCB_EVENT_MASK_EXPOSURE, reinterpret_cast<char const*>(&xcbExposeEvent));
                ::xcb_flush(connection);
            }
            else if (type == XCB_CLIENT_MESSAGE)
            {
                auto *clientMessageEvent = reinterpret_cast<xcb_client_message_event_t*>(evt);
                if (clientMessageEvent->data.data32[0] == deleteWindowAtom)
                {
                   std::cout << "Window closed" << std::endl;
                   std::free(evt);
                   break;
                }
            }
            std::free(evt);
        }
    }

    ::xcb_destroy_window(connection, window);
    ::xcb_disconnect(connection);
}

void MainProcessEntry(int argc, char const **argv)
{
    GOM->addObject(new Config(GOM));
    Config::instance()->parseFromCLI(argc, argv);
    if (Config::instance()->needExit())
        return;

    if (!GOM->hasPropertyOrDirectory("/config/JSONConfigFile"))
        throw std::runtime_error("You must specify a configuration file");

    Config::instance()->parseFromJSON(GOM->getPropertyString("/config/JSONConfigFile"));
    Journal::InitializeJournal();

    GOM->addObject(new IODispatcher());

    // render();
#if 0
    auto stylesheet = kmr::CSSStylesheet::Make(kmr::CSSStylesheet::Level::kCSSLevel_Default,
                                               kmr::CSSStylesheet::Charset::kUtf8,
                                               "cocoa://test/css/stylesheet/sample.css",
                                               "Sample",
                                               std::make_unique<kmr::CSSStylesheetDefaultEnvironment>());
    std::string css = "h1 { color: red }\n"
                      "h4 { color: #321; }\n"
                      "div { background: url(cocoa://backgrounds/user/chino.bg); }\n";

    stylesheet->appendData(css);
    stylesheet->parse();

    auto selectCtx = kmr::CSSSelectionContext::Make();
    selectCtx->appendSheet(stylesheet);
#endif
    std::string data = ".dialog-box {\n"
                       "    background-color: #66ccff\n"
                       "    position: relative;\n"
                       "}\n";

    auto stylesheet = kmr::CSSStylesheet::Make(kmr::CSSStylesheet::Level::kCSSLevel_3,
                                               kmr::CSSStylesheet::Charset::kUtf8,
                                               "file:///",
                                               "test stylesheet",
                                               std::make_unique<kmr::CSSStylesheetDefaultEnvironment>());
    auto selectionContext = kmr::CSSSelectionContext::Make();
    kmr::CSSCalculator calculator;
    kmr::CSSDOMVisitor *visitor = nullptr;

    stylesheet->appendData(data);
    selectionContext->appendSheet(stylesheet);

    calculator.elevate(selectionContext, visitor);
}

int NamespacedMain(int argc, char const **argv)
{
    // ProfilerStart("cocoa.prof");
    OCInitialize();

    try
    {
        MainProcessEntry(argc, argv);
    }
    ARTEXCEPT_CATCH(ARTException)
    {
        if (Journal::instance())
        {
            __except->dumpError();
            __except->backtrace();
        }
        else
            std::cerr << "Error: " << __except->who() << ": " << __except->what() << std::endl;
        ARTEXCEPT_CATCH_END
    }
    /*
    catch (std::exception& except)
    {
        std::cerr << "Error: " << except.what() << std::endl;
    }
    */

    OCFinalize();
    // ProfilerStop();
    return 0;
}

} // namespace cocoa

int main(int argc, char const *argv[])
{
    return cocoa::NamespacedMain(argc, argv);
}
