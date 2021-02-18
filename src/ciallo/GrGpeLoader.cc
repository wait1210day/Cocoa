#include <string>
#include <memory>
#include <list>

#include "core/Journal.h"
#include "ciallo/GrGpeLoader.h"
#include "ciallo/GrGpeFramework.h"

namespace
{

#if defined(__linux__)

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/stat.h>

constexpr bool platform_supported() { return true; }

bool file_valid_name_impl(const std::string& file)
{
    int p;
    for (p = file.size() - 1; p > 0; p--)
    {
        if (file[p] == '.')
            break;
    }
    if (p <= 0 || file[p] != '.')
        return false;

    std::string postfix;
    for (; p < file.size(); p++)
        postfix += file[p];
    return postfix == ".so";
}

bool file_valid_impl(const std::string& file)
{
    int fd = ::open(file.c_str(), O_RDONLY);
    if (fd < 0)
        return false;
    
    struct stat stbuf;
    int ret = ::fstat(fd, &stbuf);
    if (ret < 0 || stbuf.st_size == 0)
        return false;
    
    ::close(fd);
    return true;
}

void open_library_impl(const std::string& file, cocoa::ciallo::GrGpeLoader::GpeDynamicModule& out)
{
    out.fFile = file;
    out.fHandle = ::dlopen(file.c_str(), RTLD_LAZY | RTLD_GLOBAL);
}

void close_library_impl(const cocoa::ciallo::GrGpeLoader::GpeDynamicModule& mod)
{
    if (mod.fHandle)
        ::dlclose(mod.fHandle);
}

void *symbol_library_impl(const cocoa::ciallo::GrGpeLoader::GpeDynamicModule& mod, const std::string& sym)
{
    if (!mod.fHandle)
        return nullptr;
    
    return ::dlsym(mod.fHandle, sym.c_str());
}

#else

constexpr bool platform_supported() { return false; }

#endif /* defined(__linux__) */

} // namespace

CIALLO_BEGIN_NS

std::list<GrGpeLoader::GpeDynamicModule> GrGpeLoader::fModules;

std::shared_ptr<GrGpe> GrGpeLoader::Load(const std::string& file)
{
    if constexpr(!platform_supported())
    {
        logOut(LOG_ERROR) << "Couldn\'t load extension, unsupported platform"
                          << logEndl;
        return nullptr;
    }

    if (!file_valid_name_impl(file))
    {
        logOut(LOG_ERROR) << "Couldn\'t load extension " << file << ", invalid file name"
                          << logEndl;
        return nullptr;
    }

    if (!file_valid_impl(file))
    {
        logOut(LOG_ERROR) << "Couldn\'t load extension " << file << ", failed to open file"
                          << logEndl;
        return nullptr;
    }

    GpeDynamicModule dynamicModule;
    open_library_impl(file, dynamicModule);
    if (dynamicModule.fHandle == nullptr)
    {
        logOut(LOG_ERROR) << "Couldn\'t load extension " << file << ", failed to open library"
                          << logEndl;
        return nullptr;
    }

    /* Resolving the symbols and verifcating */
    const GrGpeLoaderInfo loaderInfo{
        .fName = "GrGpeLoader",
        .fDescription = "Standard GPE module loader for Cocoa/Ciallo engine",
        .fApiVersion = GPE_CURRENT_API_VERSION
    };
    GrGpeModule *selfDesc = static_cast<GrGpeModule*>(
        symbol_library_impl(dynamicModule, GPE_MODULE_SYMNAME_STR));
    if (selfDesc == nullptr)
    {
        logOut(LOG_ERROR) << "Couldn\'t load extension "
                          << file << ", failed to find proper symbols"
                          << logEndl;
        close_library_impl(dynamicModule);
        return nullptr;
    }

    if (selfDesc->fApiVersion != GPE_CURRENT_API_VERSION)
    {
        logOut(LOG_ERROR) << "Couldn\'t load extension "
                          << file << ", unexpected API version"
                          << logEndl;
        close_library_impl(dynamicModule);
        return nullptr;
    }
    if (selfDesc->fAllocator == nullptr)
    {
        logOut(LOG_ERROR) << "Couldn\'t load extension "
                          << file << ", Null allocator"
                          << logEndl;
        close_library_impl(dynamicModule);
        return nullptr;
    }

    if (selfDesc->fInitializer)
        selfDesc->fInitializer(&loaderInfo);
    dynamicModule.fGpeObject = std::shared_ptr<GrGpe>(selfDesc->fAllocator(&loaderInfo));
    dynamicModule.fSelfDescriptor = selfDesc;
    fModules.push_back(dynamicModule);

    logOut(LOG_INFO) << "Loading GPE module \"" << dynamicModule.fGpeObject->name()
                     << "\" from " << file << logEndl;

    return dynamicModule.fGpeObject;
}

void GrGpeLoader::Unload(std::shared_ptr<GrGpe>& ptr)
{
    if (ptr.use_count() > 2)
    {
        logOut(LOG_WARNING) << "Unloading GPE: There're still other objects own this module"
                            << logEndl;
        logOut(LOG_WARNING) << "Unloading GPE: " << ptr.use_count()
                            << " reference(s) in total" << logEndl;
    }

    auto itr = fModules.begin();
    for (; itr != fModules.end(); ++itr)
    {
        if (itr->fGpeObject == ptr)
            break;
    }
    if (itr == fModules.end())
        return;
    
    /* Release caller's reference */
    ptr = nullptr;
    /* Release GrGpeLoader's reference */
    itr->fGpeObject = nullptr;

    /* Close the library */
    if (itr->fSelfDescriptor->fFinalizer)
        itr->fSelfDescriptor->fFinalizer();
    close_library_impl(*itr);
    itr->fHandle = nullptr;

    fModules.erase(itr);
}

CIALLO_END_NS
