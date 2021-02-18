#ifndef COCOA_KOMOREBI_H
#define COCOA_KOMOREBI_H

/**
 * Komorebi is a 2D element layout and painting engine.
 * Komorebi is based on Ciallo 2D rendering engine.
 */

#include <cstdint>
#include <stdexcept>
#include <initializer_list>

#include "core/Project.h"
#include "core/Exception.h"

#define KMR_NAMESPACE_BEGIN namespace cocoa { namespace kmr {
#define KMR_NAMESPACE_END   }}

#define KMR_MAJOR           1
#define KMR_MINOR           1
#define KMR_PATCH           0
#define KMR_VERSION         COCOA_MAKE_VERSION(KMR_MAJOR, KMR_MINOR, KMR_PATCH)

KMR_NAMESPACE_BEGIN

class KmrException : public ARTException
{
public:
    KmrException() = default;
    ~KmrException() override = default;
};

KMR_NAMESPACE_END
#endif //COCOA_WIDGET_H
