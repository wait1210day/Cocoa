#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>

constexpr int32_t WIDTH = 800;
constexpr int32_t HEIGHT = 600;

std::vector<uint32_t> color_table;

#define MAKE_RGB(r, g, b)       (0xff000000 | ((r) << 16) | ((g) << 8) | (b))

void fill_color(uint8_t *__ptr, size_t size, uint32_t color)
{
    uint32_t *ptr = reinterpret_cast<uint32_t*>(__ptr);
    for (size_t i = 0; i < size / 4; i++)
        ptr[i] = color;
}

void next_color(uint8_t *__ptr, size_t size)
{
    static size_t color_idx = 0;
    if (color_idx >= color_table.size())
        color_idx = 0;
    fill_color(__ptr, size, color_table[color_idx++]);
}

void generate_color_table()
{
    for (int g = 0; g <= 0xff; g++)                 // 赤到黄
        color_table.push_back(MAKE_RGB(0xff, g, 0));
    for (int r = 0xff; r >= 0; r--)                 // 黄到绿
        color_table.push_back(MAKE_RGB(r, 0xff, 0));
    for (int b = 0; b <= 0xff; b++)                 // 绿到青
        color_table.push_back(MAKE_RGB(0, 0xff, b));
    for (int g = 0xff; g >= 0; g--)                 // 青到蓝
        color_table.push_back(MAKE_RGB(0, g, 0xff));
    for (int r = 0; r <= 0xff; r++)                 // 蓝到紫
        color_table.push_back(MAKE_RGB(r, 0, 0xff));
    for (int g = 0xff; g >= 0; g--)                 // 紫到红
        color_table.push_back(MAKE_RGB(0xff, 0, g));
}

int main(int argc, char const *argv[])
{
    generate_color_table();

    ::xcb_connection_t *connection = ::xcb_connect(nullptr, nullptr);
    const ::xcb_setup_t *setup = ::xcb_get_setup(connection);
    ::xcb_screen_iterator_t screen_iterator = ::xcb_setup_roots_iterator(setup);
    ::xcb_screen_t *screen = screen_iterator.data;

    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t mask_values[2] = { screen->white_pixel, XCB_EVENT_MASK_EXPOSURE };

    int32_t depth = screen->root_depth;

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

    ::xcb_gcontext_t gcontext = ::xcb_generate_id(connection);
    ::xcb_create_gc(connection, gcontext, window, 0, nullptr);

    ::xcb_pixmap_t pixmap = ::xcb_generate_id(connection);
    ::xcb_create_pixmap(connection, depth, pixmap, window, WIDTH, HEIGHT);

    constexpr size_t imageSize = WIDTH * HEIGHT * sizeof(uint32_t);
    uint8_t *imageData = static_cast<uint8_t*>(std::malloc(imageSize));
    std::memset(imageData, 0, imageSize);
    ::xcb_image_t *image = ::xcb_image_create_native(connection,
                                                     WIDTH, HEIGHT,
                                                     XCB_IMAGE_FORMAT_Z_PIXMAP, depth,
                                                     imageData, imageSize, imageData);
    ::xcb_image_put(connection, pixmap, gcontext, image, 0, 0, 0);

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

    ::xcb_generic_event_t *evt;
    while ((evt = ::xcb_wait_for_event(connection)))
    {
        switch (evt->response_type & ~0x80)
        {
            case XCB_EXPOSE:
            {
                ::xcb_expose_event_t *exposeEvent = reinterpret_cast<::xcb_expose_event_t*>(evt);
                ::xcb_copy_area(connection,
                                pixmap,
                                window,
                                gcontext,
                                exposeEvent->x, exposeEvent->y,
                                exposeEvent->x, exposeEvent->y,
                                exposeEvent->width, exposeEvent->height);

                next_color(imageData, imageSize);
                ::xcb_image_put(connection, pixmap, gcontext, image, 0, 0, 0);

                ::xcb_send_event(connection, true, window,
                                 XCB_EVENT_MASK_EXPOSURE, reinterpret_cast<char const*>(&xcbExposeEvent));
                ::xcb_flush(connection);
            }
            break;
        }

        std::free(evt);
    }

    ::xcb_image_destroy(image);
    ::xcb_free_pixmap(connection, pixmap);
    ::xcb_destroy_window(connection, window);
    ::xcb_disconnect(connection);
    return 0;
}
