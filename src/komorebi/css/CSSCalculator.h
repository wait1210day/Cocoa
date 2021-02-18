#ifndef COCOA_CSSCALCULATOR_H
#define COCOA_CSSCALCULATOR_H

#include <memory>
#include <vector>

#include "core/Exception.h"
#include "komorebi/Komorebi.h"
#include "komorebi/css/CSSDOMVisitor.h"
#include "komorebi/css/CSSSelectionContext.h"
#include "komorebi/css/CSSComputedStyle.h"

KMR_NAMESPACE_BEGIN

class CSSCalculator : public Throwable
{
public:
    void elevate(std::shared_ptr<CSSSelectionContext> context,
                 CSSDOMVisitor *visitor);

private:
    void toRetChecked(int ret, const char *func, const char *action);
};

KMR_NAMESPACE_END
#endif //COCOA_CSSCALCULATOR_H
