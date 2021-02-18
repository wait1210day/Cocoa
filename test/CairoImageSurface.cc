#include <cairomm/cairomm.h>

int main(int argc, char const *argv[])
{
    // auto surface = Cairo::ImageSurface::create(Cairo::Format::FORMAT_ARGB32, 1280, 720);
    auto surface = Cairo::SvgSurface::create("./out.svg", 1280, 720);
    auto ctx = Cairo::Context::create(surface);

    ctx->set_antialias(Cairo::Antialias::ANTIALIAS_SUBPIXEL);
    ctx->set_source_rgb(1, 1, 1);
    ctx->paint();

    ctx->set_source_rgb(1, 0, 0);
    ctx->arc(500, 500, 100, M_PI, 0);
    ctx->stroke();

    surface->write_to_png("./out.png");
    return 0;
}
