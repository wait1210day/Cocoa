#ifndef __GR_GPE_LOADER_H__
#define __GR_GPE_LOADER_H__

#include <string>
#include <memory>
#include <list>

#include "ciallo/GrBase.h"
#include "ciallo/GrGpe.h"

struct GrGpeModule;

CIALLO_BEGIN_NS

class GrGpeLoader
{
public:
    struct GpeDynamicModule
    {
        std::string      fFile;
        void            *fHandle = nullptr;
        std::shared_ptr<GrGpe>
                         fGpeObject = nullptr;
        GrGpeModule     *fSelfDescriptor = nullptr;
    };

    static std::shared_ptr<GrGpe> Load(const std::string& file);
    static void Unload(std::shared_ptr<GrGpe>& ptr);

private:
    static std::list<GpeDynamicModule>  fModules;
};

CIALLO_END_NS
#endif /* __GR_GPE_LOADER_H__ */
