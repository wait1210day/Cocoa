#include <iostream>
#include <string>
#include "include/core/SkData.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkStream.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPath.h"
#include "include/core/SkString.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/core/SkImageInfo.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
using namespace std;

SkString sksl("\
half4 main(float2 pos) \
{ \
    return half4(1, 0, 0.5, 1); \
}\
");

void writeData(sk_sp<SkData> data)
{
    int fd = open("sksl.png", O_RDWR | O_CREAT,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    write(fd, data->data(), data->size());
    close(fd);
}

void draw(SkCanvas *canvas)
{
    SkRuntimeEffect::EffectResult effectResult(SkRuntimeEffect::Make(sksl));
    if (!std::get<0>(effectResult).get())
    {
      	SkDebugf("Error!");
        return;
    }
    canvas->clear(SK_ColorWHITE);

    sk_sp<SkRuntimeEffect> effect(std::get<0>(effectResult));
    SkPaint paint;
    paint.setShader(effect->makeShader(nullptr, nullptr, 0, nullptr, false));
    paint.setAntiAlias(true);
  
    SkPoint points[] = { { 0, 0 }, { 250, 0 }, { 100, 100 }, { 0, 250 } };
    SkPoint texs[] = { { 0, 0 }, { 250, 0 }, { 100, 100 }, { 0, 250 } };
    auto vertices = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode,
            SK_ARRAY_COUNT(points), points, texs, nullptr);
  
    canvas->drawVertices(vertices, SkBlendMode::kSrc, paint);
}

int main(int argc, char const *argv[])
{
    SkImageInfo info(SkImageInfo::MakeN32Premul(800, 600));
    sk_sp<SkSurface> surface(SkSurface::MakeRaster(info));

    SkCanvas *canvas = surface->getCanvas();
    
    draw(canvas);

    sk_sp<SkImage> image(surface->makeImageSnapshot());
    sk_sp<SkData> data(image->encodeToData(SkEncodedImageFormat::kPNG, 100));
    writeData(data);
    return 0;
}
