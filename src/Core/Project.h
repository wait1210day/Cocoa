#ifndef __VERSION_H__
#define __VERSION_H__

#ifndef COCOA_PROJECT
#error Project.h only can be included in Cocoa Project
#endif

#define COCOA_MAJOR     1
#define COCOA_MINOR     0
#define COCOA_PATCH     0

#define COCOA_NAME      "Cocoa"

#define COCOA_MAKE_VERSION(major, minor, patch)     (((major) << 24) | ((minor) << 16) | (patch))

#define COCOA_VERSION   "0.0.1-develop"
#define COCOA_LICENSE   "General Public License (GPLv3)"

#endif // __VERSION_H__
