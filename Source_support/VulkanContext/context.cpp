#include "context.h"

bool VulkanContext::Setup(int aW, int aH)
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
    allocCreateInfo.vulkanApiVersion = VK_MAKE_API_VERSION(0,1,4,0);
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

VulkanContext::~VulkanContext()
{
    if(mAllocator != VK_NULL_HANDLE)
        vmaDestroyAllocator(mAllocator);
}
