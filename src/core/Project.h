#ifndef __VERSION_H__
#define __VERSION_H__

#ifndef COCOA_PROJECT
#error "Project.h only can be included in Cocoa Project"
#endif

#define COCOA_MAJOR     1
#define COCOA_MINOR     0
#define COCOA_PATCH     0

#define COCOA_NAME      "Cocoa"

#define COCOA_MAKE_VERSION(major, minor, patch)     (((major) << 24) | ((minor) << 16) | (patch))

#define COCOA_VERSION   "Cocoa 0.0.1-develop"
#define COCOA_COPYRIGHT "Copyright(C) wait1210day <masshiro.io@qq.com> 2020"
#define COCOA_LICENSE   "General Public License (GPLv3)"

#define make_string_constant(def)   #def
#define COCOA_FRAMEWORK_VERSION     "Cocoa Core Framework, version 1.0"
#define COCOA_GOM_VERSION           "Cocoa Global Object Manager, version 1.0"

#define PROJ_DOC_USER       "[Cocoa User's Specification]"
#define PROJ_DOC_DEVELOPER  "[Cocoa Developer's Specification]"

#define PROJ_DOCUMENT_REF_STR(spec, chp, sec, cop) \
    (spec " Chapter "#chp", Section "#sec" (#COP"#cop")")

#define PROJ_CONF_SIGNATURE     "org.CocoaProject.configuration"
#endif // __VERSION_H__
