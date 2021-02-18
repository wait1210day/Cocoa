#ifndef __GR_GPE_H__
#define __GR_GPE_H__

#include "ciallo/GrBase.h"
#include "ciallo/GrGpe.h"
CIALLO_BEGIN_NS

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define GPE_EXPORTED __attribute__((visibility("default")))
#else
#define GPE_EXPORTED
#endif /* __clang__ || __GNUC__ || __GNUG__ */

struct GrGpeModule
{
    void (*fInitializer)(const GrGpeLoaderInfo*);
    void (*fFinalizer)();
    GrGpe *(*fAllocator)(const GrGpeLoaderInfo*);

    uint32_t    fApiVersion;
    std::string fSignature;
};

/**
 * __GpeModule__ symbol help ciallo locate the GrGpeModule structure,
 * so as to obtain the extended function pointer, GrGpe object,
 * trusted signature and other information.
 */
#define GPE_MODULE_SYMNAME          __GpeModule__
#define GPE_MODULE_SYMNAME_STR      "__GpeModule__"

/* __GpeModule__ must be placed in root namespace */
#define GPE_MODULE(pfnInit, pfnAlloc, pfnFinal, sign) \
GrGpeModule GPE_EXPORTED GPE_MODULE_SYMNAME \
{ \
    .fInitializer = pfnInit, \
    .fFinalizer = pfnFinal, \
    .fAllocator = pfnAlloc, \
    .fApiVersion = GPE_CURRENT_API_VERSION, \
    .fSignature = sign \
};

#define GPU_DEFINE_

CIALLO_END_NS
#endif /* __GR_GPE_H__ */
