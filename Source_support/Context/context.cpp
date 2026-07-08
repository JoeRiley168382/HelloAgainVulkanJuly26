#include "context.h"

bool Context::Setup(int aW, int aH)
{
    SDL_Init(SDL_INIT_VIDEO);
    volkInitialize();
    if(!mInstance.Setup())
        return false;
    if(!mWindow.SetupWindow(mInstance.mInstance, aW, aH))
        return false;
    if(!mDevice.Setup(mInstance.mInstance, mWindow.mSurface))
        return false;

    VmaAllocatorCreateInfo allocCreateInfo{};
    allocCreateInfo.physicalDevice = mDevice.mPhysicalDevice;
    allocCreateInfo.device = mDevice.mLogicalDevice;
    allocCreateInfo.instance = mInstance.mInstance;
    //VMA v3.1.0's vmaCreateAllocator asserts minor <= 3 - it predates the
    //Vulkan 1.4 spec. This is only VMA's own feature-detection version, not
    //the actual instance version (see instance.cpp), so under-reporting is safe.
    allocCreateInfo.vulkanApiVersion = VK_MAKE_API_VERSION(0,1,3,0);
    VmaVulkanFunctions vmaFuncs{};
    vmaFuncs.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vmaFuncs.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    allocCreateInfo.pVulkanFunctions = &vmaFuncs;

    if(vmaCreateAllocator(&allocCreateInfo, &mAllocator) != VK_SUCCESS)
        return false;
    if(!mWindow.SetupSwapchain(mDevice))
        return false;

    return true;
}

Context::~Context()
{
    if(mAllocator != VK_NULL_HANDLE)
        vmaDestroyAllocator(mAllocator);
}