#define SK_VULKAN

#if defined(__linux__)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include <unistd.h>
#include <sys/types.h>

#endif // defined(__linux__)

#include <utility>
#include <vector>
#include <set>

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkRect.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurfaceMutableState.h"
#include "include/gpu/vk/GrVkBackendContext.h"

#include "core/Exception.h"
#include "core/Journal.h"
#include "ciallo/DDR/GrGpuCompositor.h"
#include "ciallo/DDR/GrGpuRenderLayer.h"

CIALLO_BEGIN_NS

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>
                (vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>
                (vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}

std::shared_ptr<GrBaseCompositor> GrGpuCompositor::Make(const VkSurfaceCreatorFunc& func,
                                                    int32_t width,
                                                    int32_t height,
                                                    GrColorFormat colorFormat,
                                                    GrBasePlatform *platform,
                                                    const std::vector<std::string>& requiredVkInstanceExts,
                                                    const std::vector<std::string>& requiredVkDeviceExts,
                                                    bool debugMode)
{
    std::shared_ptr<GrGpuCompositor> ret = std::make_shared<GrGpuCompositor>(
        debugMode,
        width,
        height,
        colorFormat,
        func,
        platform,
        requiredVkInstanceExts,
        requiredVkDeviceExts);
    ret->initObject();
    return ret;
}

PFN_vkVoidFunction GrGpuCompositor::skia_vulkan_funcptr(char const *sym,
                                                      VkInstance instance,
                                                      VkDevice device)
{
    if (device != VK_NULL_HANDLE)
        return vkGetDeviceProcAddr(device, sym);
    return vkGetInstanceProcAddr(instance, sym);
}

GrGpuCompositor::GrGpuCompositor(bool debug,
                                 int32_t width,
                                 int32_t height,
                                 GrColorFormat colorFormat,
                                 VkSurfaceCreatorFunc creator,
                                 GrBasePlatform *platform,
                                 const std::vector<std::string>& requiredVkInstanceExts,
                                 const std::vector<std::string>& requiredVkDeviceExts)
    : GrBaseCompositor("cocoa::ciallo::GrGpuCompositor",
                       CompositeDevice::kGpuVulkan,
                       width,
                       height,
                       colorFormat,
                       platform),
      fSurfaceMakeFunc(std::move(creator)),
      fVkRequiredInstanceExts(requiredVkInstanceExts),
      fVkRequiredDeviceExts(requiredVkDeviceExts),
      fVkInstance(VK_NULL_HANDLE),
      fHasDebug(debug),
      fVkDebugMessenger(VK_NULL_HANDLE),
      fVkSurface(VK_NULL_HANDLE),
      fVkPhysicalDevice(VK_NULL_HANDLE),
      fVkDevice(VK_NULL_HANDLE),
      fVkGraphicsQueue(VK_NULL_HANDLE),
      fVkPresentQueue(VK_NULL_HANDLE),
      fVkSwapchain(VK_NULL_HANDLE),
      fDirectContext(nullptr)
{
}

GrGpuCompositor::~GrGpuCompositor()
{
    GrBaseCompositor::Dispose();
    finalizeObject();
}

static CompositeDriverSpecDeviceType vkDeviceTypeToDriverSpec(VkPhysicalDeviceType type)
{
    switch (type)
    {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return CompositeDriverSpecDeviceType::kVulkan_Other;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return CompositeDriverSpecDeviceType::kVulkan_IntergratedGpu;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return CompositeDriverSpecDeviceType::kVulkan_DiscreteGpu;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return CompositeDriverSpecDeviceType::kVulkan_VirtualGpu;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return CompositeDriverSpecDeviceType::kVulkan_Cpu;
    default:
        break;
    }
    return CompositeDriverSpecDeviceType::kVulkan_Other;
}

void GrGpuCompositor::initObject()
{
    makeGpuInstance();
    if (fHasDebug)
        makeDebugMessenger();
    makeGpuSurface();
    pickGpuPhysicalDevice();
    makeGpuLogicalDevice();
    makeGpuSwapchain();
    makeGpuBuffers();

    makeDirectContext();
    makeSurfaces();

    this->setDriverSpecDeviceTypeInfo(
        vkDeviceTypeToDriverSpec(fVkPhysicalDeviceProps.deviceType));
    this->setDeviceInfo(fVkPhysicalDeviceProps.driverVersion,
                        fVkPhysicalDeviceProps.apiVersion,
                        fVkPhysicalDeviceProps.vendorID,
                        fVkPhysicalDeviceProps.deviceName);
}

void GrGpuCompositor::finalizeObject()
{
    if (fVkDevice != VK_NULL_HANDLE)
        vkDeviceWaitIdle(fVkDevice);

    for (SkSurface *surface : fSurfaces)
        surface->unref();
    if (fDirectContext)
        fDirectContext->unref();
    
    for (GpuBufferInfo& buf : fBuffers)
        vkDestroySemaphore(fVkDevice, buf.fRenderSemaphore, nullptr);
    
    if (fVkSwapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(fVkDevice, fVkSwapchain, nullptr);
    if (fVkDevice != VK_NULL_HANDLE)
        vkDestroyDevice(fVkDevice, nullptr);
    
    if (fHasDebug && fVkDebugMessenger != VK_NULL_HANDLE)
        DestroyDebugUtilsMessengerEXT(fVkInstance, fVkDebugMessenger,
            nullptr);
    
    if (fVkSurface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(fVkInstance, fVkSurface, nullptr);
    if (fVkInstance != VK_NULL_HANDLE)
        vkDestroyInstance(fVkInstance, nullptr);
}

void GrGpuCompositor::makeDirectContext()
{
    GrVkExtensions skiaVkExtensions;
    uint32_t instanceExtNum, deviceExtNum;
    std::vector<char *> instanceExtensions;
    std::vector<char *> deviceExtensions;

    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtNum, nullptr);
    {
        std::vector<VkExtensionProperties> props(instanceExtNum);
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtNum, props.data());
        for (VkExtensionProperties &prop : props)
        {
            instanceExtensions.push_back(prop.extensionName);
            this->appendGpuExtensionsInfo(prop.extensionName);
        }
    }

    vkEnumerateDeviceExtensionProperties(fVkPhysicalDevice, nullptr, &deviceExtNum, nullptr);
    {
        std::vector<VkExtensionProperties> props(deviceExtNum);
        vkEnumerateDeviceExtensionProperties(fVkPhysicalDevice, nullptr, &deviceExtNum, props.data());
        for (VkExtensionProperties &prop : props)
        {
            deviceExtensions.push_back(prop.extensionName);
            this->appendGpuExtensionsInfo(prop.extensionName);
        }
    }

    skiaVkExtensions.init(skia_vulkan_funcptr, fVkInstance, fVkPhysicalDevice,
                          instanceExtNum, instanceExtensions.data(),
                          deviceExtNum, deviceExtensions.data());

    VkPhysicalDeviceFeatures2 features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    vkGetPhysicalDeviceFeatures2(fVkPhysicalDevice, &features);

    GrVkBackendContext backend;
    backend.fInstance = fVkInstance;
    backend.fPhysicalDevice = fVkPhysicalDevice;
    backend.fDevice = fVkDevice;
    backend.fQueue = fVkGraphicsQueue;
    backend.fGraphicsQueueIndex = fVkGraphicsQueueIndex;
    backend.fMaxAPIVersion = VK_API_VERSION_1_1;
    backend.fVkExtensions = &skiaVkExtensions;
    backend.fDeviceFeatures2 = &features;
    backend.fGetProc = GrGpuCompositor::skia_vulkan_funcptr;

    fDirectContext = GrDirectContext::MakeVulkan(backend).release();
    if (fDirectContext == nullptr)
    {
        artfmt(__FUNCTION__) << "Failed to create GrDirectContext";
        arthrow<ARTException>(new ARTException());
    }
}

void GrGpuCompositor::makeSurfaces()
{
    for (auto& fVkSwapchainImage : fVkSwapchainImages)
    {
        GrVkImageInfo imageInfo;
        imageInfo.fImage = fVkSwapchainImage;
        imageInfo.fImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imageInfo.fFormat = fVkImageFormat;
        imageInfo.fImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                                    | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                    | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageInfo.fLevelCount = 1;
        imageInfo.fSharingMode = fVkImagesSharingMode;

        GrBackendRenderTarget target(this->width(), this->height(), imageInfo);
        SkSurface *surface = SkSurface::MakeFromBackendRenderTarget(
                                 fDirectContext,
                                 target,
                                 GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                 ToSkColorType(this->colorFormat()),
                                 SkColorSpace::MakeSRGB(),
                                 nullptr).release();

        if (surface == nullptr)
        {
            artfmt(__FUNCTION__) << "Failed to create SkSurface for Skia";
            arthrow<ARTException>(new ARTException());
        }
        surface->getCanvas()->clear(SK_ColorBLACK);
        fSurfaces.push_back(surface);
    }
}

const std::vector<const char*> _vkValidationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> _vkDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

void GrGpuCompositor::makeGpuInstance()
{
    if (fHasDebug && !checkValidationLayerSupport())
    {
        artfmt(__FUNCTION__) << "Vulkan validation layers requested, but not available";
        arthrow<ARTException>(new ARTException());
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Cocoa";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Cocoa/Ciallo";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.pNext = nullptr;

    auto extensions = chooseRequiredInstanceExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (fHasDebug)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(_vkValidationLayers.size());
        createInfo.ppEnabledLayerNames = _vkValidationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &fVkInstance) != VK_SUCCESS)
    {
        artfmt(__FUNCTION__) << "Failed to create Vulkan instance";
        arthrow<ARTException>(new ARTException());
    }
}

void GrGpuCompositor::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = GrGpuCompositor::debugCallback;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
}

void GrGpuCompositor::makeDebugMessenger()
{
    if (!fHasDebug)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(fVkInstance, &createInfo,
                                     nullptr, &fVkDebugMessenger) != VK_SUCCESS)
    {
        artfmt(__FUNCTION__) << "Failed to set up debug messenger for Vulkan";
        arthrow<ARTException>(new ARTException());
    }
}

void GrGpuCompositor::makeGpuSurface()
{
    fVkSurface = fSurfaceMakeFunc(fVkInstance);
    if (fVkSurface == VK_NULL_HANDLE)
    {
        artfmt(__FUNCTION__) << "Failed to create Vulkan surface";
        arthrow<ARTException>(new ARTException());
    }
}

void GrGpuCompositor::pickGpuPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(fVkInstance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        artfmt(__FUNCTION__) << "Failed to find GPUs with Vulkan support";
        arthrow<ARTException>(new ARTException());
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(fVkInstance, &deviceCount, devices.data());

    for (const auto &device : devices)
    {
        if (isDeviceSuitable(device))
        {
            fVkPhysicalDevice = device;
            vkGetPhysicalDeviceProperties(device, &fVkPhysicalDeviceProps);
            break;
        }
    }

    if (fVkPhysicalDevice == VK_NULL_HANDLE)
    {
       artfmt(__FUNCTION__) << "Failed to find a suitable GPU";
       arthrow<ARTException>(new ARTException());
    }
}

void GrGpuCompositor::makeGpuLogicalDevice()
{
    GpuQueueFamilyIndices indices = findQueueFamilies(fVkPhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.fGraphics.value(),
        indices.fPresent.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(_vkDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = _vkDeviceExtensions.data();

    if (fHasDebug)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(_vkValidationLayers.size());
        createInfo.ppEnabledLayerNames = _vkValidationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(fVkPhysicalDevice, &createInfo, nullptr, &fVkDevice) != VK_SUCCESS)
    {
        artfmt(__FUNCTION__) << "Failed to create logical device for Vulkan";
        arthrow<ARTException>(new ARTException());
    }

    fVkGraphicsQueueIndex = indices.fGraphics.value();
    vkGetDeviceQueue(fVkDevice, indices.fGraphics.value(), 0, &fVkGraphicsQueue);
    vkGetDeviceQueue(fVkDevice, indices.fPresent.value(), 0, &fVkPresentQueue);
}

void GrGpuCompositor::makeGpuSwapchain()
{
    GpuSwapchainDetails swapChainSupport = querySwapchainDetails(fVkPhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.fFormats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.fPresentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.fCaps);

    uint32_t imageCount = swapChainSupport.fCaps.minImageCount + 1;
    if (swapChainSupport.fCaps.maxImageCount > 0
        && imageCount > swapChainSupport.fCaps.maxImageCount)
        imageCount = swapChainSupport.fCaps.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = fVkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                            | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                            | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    GpuQueueFamilyIndices indices = findQueueFamilies(fVkPhysicalDevice);
    uint32_t queueFamilyIndices[] = {
        indices.fGraphics.value(),
        indices.fPresent.value()
    };

    if (indices.fGraphics != indices.fPresent)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    fVkImagesSharingMode = createInfo.imageSharingMode;

    createInfo.preTransform = swapChainSupport.fCaps.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(fVkDevice, &createInfo, nullptr, &fVkSwapchain) != VK_SUCCESS)
    {
        artfmt(__FUNCTION__) << "Failed to create swap chain";
        arthrow<ARTException>(new ARTException());
    }

    vkGetSwapchainImagesKHR(fVkDevice, fVkSwapchain, &imageCount, nullptr);
    fVkSwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(fVkDevice, fVkSwapchain, &imageCount, fVkSwapchainImages.data());

    fVkImageFormat = surfaceFormat.format;
    fVkSwapchainExtent = extent;
    fImagesCount = imageCount;
    fCurrentBackBuffer = 0;
}

void GrGpuCompositor::makeGpuBuffers()
{
    VkSemaphoreCreateInfo semCreateInfo{};
    semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < fImagesCount + 1; i++)
    {
        GpuBufferInfo bufferInfo;
        bufferInfo.fImageIndex = UINT32_MAX;
        if (vkCreateSemaphore(fVkDevice, &semCreateInfo,
                nullptr, &bufferInfo.fRenderSemaphore) != VK_SUCCESS)
        {
            artfmt(__FUNCTION__) << "Failed to create semaphore for buffer #" << i;
            arthrow<ARTException>(new ARTException());
        }
        fBuffers.push_back(bufferInfo);
    }
}

VkSurfaceFormatKHR GrGpuCompositor::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == ToVulkanColorFormat(this->colorFormat())
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR GrGpuCompositor::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D GrGpuCompositor::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    else
    {
        THROWABLE_ASSERT(this->width() > 0 && this->height() > 0);

        VkExtent2D actualExtent = { static_cast<uint32_t>(this->width()),
                                    static_cast<uint32_t>(this->height()) };
        if (static_cast<uint32_t>(this->width()) > capabilities.maxImageExtent.width
            || static_cast<uint32_t>(this->height()) > capabilities.maxImageExtent.height)
        {
            artfmt(__FUNCTION__) << "Width " << this->width() << " or height " << this->height()
                                << " is too large for swap chain";
            arthrow<ARTException>(new ARTException());
        }
        else if (static_cast<uint32_t>(this->width()) < capabilities.minImageExtent.width
            || static_cast<uint32_t>(this->height()) < capabilities.minImageExtent.height)
        {
            artfmt(__FUNCTION__) << "Width " << this->width() << " or height " << this->height()
                                << " is too small for swap chain";
            arthrow<ARTException>(new ARTException());
        }
        return actualExtent;
    }
}

GpuSwapchainDetails GrGpuCompositor::querySwapchainDetails(VkPhysicalDevice device)
{
    GpuSwapchainDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device,
        fVkSurface, &details.fCaps);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, fVkSurface,
        &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.fFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, fVkSurface,
            &formatCount, details.fFormats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, fVkSurface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.fPresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, fVkSurface,
            &presentModeCount, details.fPresentModes.data());
    }

    return details;
}

bool GrGpuCompositor::isDeviceSuitable(VkPhysicalDevice device)
{
    GpuQueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        GpuSwapchainDetails swapChainSupport = querySwapchainDetails(device);
        swapChainAdequate = !swapChainSupport.fFormats.empty()
            && !swapChainSupport.fPresentModes.empty();
    }
    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool GrGpuCompositor::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device,
        nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> exts(extensionCount);
    vkEnumerateDeviceExtensionProperties(device,
        nullptr, &extensionCount, exts.data());

    auto LocalFindExtension = [&exts](const std::string& name) -> bool {
        for (VkExtensionProperties& p : exts)
        {
            if (p.extensionName == name)
                return true;
        }
        return false;
    };

    for (char const *str : _vkDeviceExtensions)
    {
        if (!LocalFindExtension(str))
            return false;
    }

    for (std::string& str : fVkRequiredDeviceExts)
    {
        if (!LocalFindExtension(str))
            return false;
    }
    return true;
}

GpuQueueFamilyIndices GrGpuCompositor::findQueueFamilies(VkPhysicalDevice device)
{
    GpuQueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.fGraphics = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, fVkSurface, &presentSupport);
        if (presentSupport)
            indices.fPresent = i;

        if (indices.isComplete())
            break;
        i++;
    }

    return indices;
}

std::vector<const char *> GrGpuCompositor::chooseRequiredInstanceExtensions()
{
    std::vector<const char *> extensions;
    for (std::string& str : fVkRequiredInstanceExts)
        extensions.push_back(str.c_str());

    if (fHasDebug)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

bool GrGpuCompositor::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : _vkValidationLayers)
    {
        bool layerFound = false;
        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
            return false;
    }
    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
GrGpuCompositor::debugCallback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                               [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type,
                               const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                               [[maybe_unused]] void *pUserData)
{
#if defined(__linux__)
    logOut(LOG_DEBUG) << "<GpuCompsitor.Vulkan@" << ::gettid() << "> " << pCallbackData->pMessage << logEndl;
#else
    logOut(LOG_DEBUG) << "<GpuCompsitor.Vulkan> " << pCallbackData->pMessage << logEndl;
#endif /* __linux___ */
    return VK_FALSE;
}

void GrGpuCompositor::nextFrame()
{
    fCurrentBackBuffer++;
    if (fCurrentBackBuffer > fImagesCount)
        fCurrentBackBuffer = 0;
}

SkSurface *GrGpuCompositor::currentSkSurface()
{
    GpuBufferInfo *buf = &fBuffers[fCurrentBackBuffer];
    SkSurface *surface;
    if (!buf->fAcquired)
    {
        VkSemaphoreCreateInfo semCreateInfo{};
        semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkSemaphore beSemaphore;

        vkCreateSemaphore(fVkDevice, &semCreateInfo, nullptr, &beSemaphore);
        vkAcquireNextImageKHR(fVkDevice, fVkSwapchain, UINT64_MAX, beSemaphore,
            VK_NULL_HANDLE, &buf->fImageIndex);

        surface = fSurfaces[buf->fImageIndex];
        GrBackendSemaphore backendSem;
        backendSem.initVulkan(beSemaphore);
        surface->wait(1, &backendSem);
        buf->fAcquired = true;
    }
    else
        surface = fSurfaces[buf->fImageIndex];

    return surface;
}

uint32_t GrGpuCompositor::ToVulkanColorFormat(GrColorFormat colorFormat)
{
    switch (colorFormat)
    {
    case GrColorFormat::kColor_RGBA_8888:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case GrColorFormat::kColor_BGRA_8888:
        return VK_FORMAT_B8G8R8A8_UNORM;
    }
}

// -----------------------------------------------------------------------------------------
// Public APIs
// -----------------------------------------------------------------------------------------

void GrGpuCompositor::onPresent()
{
    if (fCurrentBackBuffer > fImagesCount)
        return;
    GpuBufferInfo *buf = &fBuffers[fCurrentBackBuffer];
    if (!buf->fAcquired)
        return;

    SkSurface *surface = fSurfaces[buf->fImageIndex];

    GrBackendSemaphore rdrSemaphore;
    rdrSemaphore.initVulkan(buf->fRenderSemaphore);
    GrFlushInfo flushInfo{
        .fNumSemaphores = 1,
        .fSignalSemaphores = &rdrSemaphore
    };

    GrBackendSurfaceMutableState newState(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                          fVkGraphicsQueueIndex);
    GrSemaphoresSubmitted submitted = surface->flush(flushInfo, &newState);
    THROWABLE_ASSERT(submitted == GrSemaphoresSubmitted::kYes);

    fDirectContext->submit();

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &buf->fRenderSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &fVkSwapchain;
    presentInfo.pImageIndices = &buf->fImageIndex;

    vkQueuePresentKHR(fVkPresentQueue, &presentInfo);
    buf->fAcquired = false;
    nextFrame();
}

GrBaseRenderLayer *GrGpuCompositor::onCreateRenderLayer(int32_t width,
                                                    int32_t height,
                                                    int32_t px,
                                                    int32_t py,
                                                    int zindex)
{
    THROWABLE_ASSERT(width > 0 && height > 0);

    SkImageInfo imageInfo = SkImageInfo::Make(SkISize::Make(width, height),
                                              ToSkColorType(this->colorFormat()),
                                              SkAlphaType::kPremul_SkAlphaType,
                                              SkColorSpace::MakeSRGB());
    return new GrGpuRenderLayer(fDirectContext, px, py, zindex, imageInfo);
}

GrTargetSurface GrGpuCompositor::onTargetSurface()
{
    return GrTargetSurface(currentSkSurface());
}

CIALLO_END_NS
