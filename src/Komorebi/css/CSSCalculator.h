#ifndef COCOA_CSSCALCULATOR_H
#define COCOA_CSSCALCULATOR_H

#include <memory>
#include <vector>

#include "Core/Exception.h"
#include "Komorebi/Komorebi.h"
#include "Komorebi/css/CSSDOMVisitor.h"
#include "Komorebi/css/CSSSelectionContext.h"
#include "Komorebi/css/CSSComputedStyle.h"

KMR_NAMESPACE_BEGIN

class CSSCalculator
{
public:
    void elevate(std::shared_ptr<CSSSelectionContext> context,
                 CSSDOMVisitor *visitor);

private:
    void toRetChecked(int ret, const char *func, const char *action);
};

KMR_NAMESPACE_END
#endif //COCOA_CSSCALCULATOR_H
