#ifndef __CIALLO_UTILS_H__

#include "Ciallo/GrBase.h"
#include <vector>
#include <Poco/SharedPtr.h>

CIALLO_BEGIN_NS

using ColorSpecifierList = std::vector<GrColorSpecifier>;
extern Poco::SharedPtr<ColorSpecifierList> buildGradientColorTable();

CIALLO_END_NS

#endif /* __CIALLO_UTILS_H__ */
