#ifndef COCOA_SKPSNAPSHOT_H
#define COCOA_SKPSNAPSHOT_H

#include <string>

#include "include/core/SkPicture.h"
#include "include/core/SkData.h"

#include "ciallo/GrBase.h"
CIALLO_BEGIN_NS

enum class SkpSnapshotCompress
{
    kRaw,
    kZlib
};

sk_sp<SkPicture> SkpSnapshotLoad(const std::string& path);
bool SkpSnapshotWriteToFile(const sk_sp<SkPicture>& picture, const std::string& path,
                            SkpSnapshotCompress compress);

CIALLO_END_NS
#endif //COCOA_SKPSNAPSHOT_H
