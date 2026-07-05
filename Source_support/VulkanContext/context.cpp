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
    if(!mWindow.SetupSwapchain(mDevice))
        return false;
    return true;
}
