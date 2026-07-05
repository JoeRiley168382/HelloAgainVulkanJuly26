// vulkancontext.h
#ifndef _VULKAN_APP_
#define _VULKAN_APP_

#include "VulkanContext/context.h"
#include "VulkanRenderer/renderer.h"

class VulkanApp {
public:
    bool Start(int aW, int aH);
    void RunLoop();
    bool HandleResize();
    VulkanContext mContext;
    VulkanRenderer mRenderer;
    SDL_Event mCurrentEvent;
protected:
    virtual void SetupPipelines() {}
    virtual ~VulkanApp() = default;
};

#endif