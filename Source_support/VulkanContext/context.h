// vulkancontext.h
#ifndef _VULKAN_CONTEXT_
#define _VULKAN_CONTEXT_

#include <SDL3/SDL.h>
#include "volk.h"

#include "instance.h"
#include "device.h"
#include "window.h"

class VulkanContext {
public:
    bool Setup(int aW, int aH);
    //Declaration order matters: members destruct in reverse, and we need
    //mWindow's surface/swapchain gone before mDevice, and mDevice gone
    //before mInstance (a device must not outlive the instance it came from).
    VulkanInstance mInstance;
    VulkanDevice mDevice;
    VulkanWindow mWindow;
};

#endif