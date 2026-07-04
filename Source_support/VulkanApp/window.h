#ifndef _VULKAN_SDL_WINDOW_
#define _VULKAN_SDL_WINDOW_

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "volk.h"

#include "device.h"

#include <vector>
#include <unordered_set>

class VulkanWindow {
public:
    VulkanWindow() = default;
    ~VulkanWindow();
    bool SetupWindow(VkInstance,  int,  int);
    bool SetupSwapchain(VulkanDevice& aDevice);
    SDL_Window* mWindow = nullptr;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;
    VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
    //h = "handle"
    VkInstance hInstance = VK_NULL_HANDLE;
    VulkanDevice* hDevice = nullptr; 
    VkFormat mSwapchainFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D mSwapchainExtent{};
    std::vector<VkImage> mSwapImageList;
    std::vector<VkImageView> mSwapViewList;
};

#endif