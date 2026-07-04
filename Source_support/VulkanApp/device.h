#ifndef _VULKAN_DEVICE_
#define _VULKAN_DEVICE_

#include "volk.h"
#include <vector>

class VulkanDevice
{
    public:
        VulkanDevice() = default;
        ~VulkanDevice();
        bool Setup(VkInstance, VkSurfaceKHR);
        VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
        VkDevice mLogicalDevice = VK_NULL_HANDLE;
        int mPresentQueueFamilyIndex = -1;
        VkQueue mPresentQueue = VK_NULL_HANDLE;
        int mGraphicsQueueFamilyIndex = -1;
        VkQueue mGraphicsQueue = VK_NULL_HANDLE;
        VkInstance hInstance = VK_NULL_HANDLE;
        VkSurfaceKHR hSurface = VK_NULL_HANDLE;
};


#endif