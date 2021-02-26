#include "Ciallo/GrUtils.h"

CIALLO_BEGIN_NS

Poco::SharedPtr<ColorSpecifierList> buildGradientColorTable()
{
    Poco::SharedPtr<ColorSpecifierList> ret(new ColorSpecifierList());
    for (int g = 0; g <= 0xff; g++)                 // 赤到黄
        ret->push_back(GrColorSpecifier(0xff, g, 0));
    for (int r = 0xff; r >= 0; r--)                 // 黄到绿
        ret->push_back(GrColorSpecifier(r, 0xff, 0));
    for (int b = 0; b <= 0xff; b++)                 // 绿到青
        ret->push_back(GrColorSpecifier(0, 0xff, b));
    for (int g = 0xff; g >= 0; g--)                 // 青到蓝
        ret->push_back(GrColorSpecifier(0, g, 0xff));
    for (int r = 0; r <= 0xff; r++)                 // 蓝到紫
        ret->push_back(GrColorSpecifier(r, 0, 0xff));
    for (int g = 0xff; g >= 0; g--)                 // 紫到红
        ret->push_back(GrColorSpecifier(0xff, 0, g));

    return ret;
}

CIALLO_END_NS
