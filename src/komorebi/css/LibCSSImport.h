#ifndef COCOA_LIBCSSIMPORT_H
#define COCOA_LIBCSSIMPORT_H

/**
 * There's a bug for LibCSS header file when we compile by a C++ compiler.
 * In include file compute.h line 82:
 *      const css_computed_style *restrict parent
 *      ...
 * It's fine using a C compiler like clang to compile it.
 * But in C++, we should use "__restrict__" instead of "restrict".
 */

#define restrict __restrict__

extern "C" {
#include <libcss/libcss.h>
};

#undef restrict

#endif //COCOA_LIBCSSIMPORT_H
