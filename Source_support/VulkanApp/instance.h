#ifndef _VULKAN_INSTANCE_H_
#define _VULKAN_INSTANCE_H_

#include <vector>
#include <unordered_set>
#include <string>
#include "volk.h"

class VulkanInstance {
public:
    VulkanInstance() = default;
    ~VulkanInstance();
    bool Setup();
    VkInstance mInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT mDebugMsgr = VK_NULL_HANDLE;
    VkApplicationInfo mVulkanAppInfo{};    
};

#endif // _VULKAN_INSTANCE_H_