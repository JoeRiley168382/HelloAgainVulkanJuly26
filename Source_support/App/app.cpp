#include "app.h"

bool App::Start(int aW, int aH)
{

    if(!mContext.Setup(aW, aH)) return false;
    SetupPipelines();
    SetupRenderObjects();
    if(!mRenderer.Setup(
                        mContext.mDevice.mLogicalDevice,
                        mContext.mDevice.mGraphicsQueue,
                        mContext.mDevice.mPresentQueue,
                        mContext.mDevice.mGraphicsQueueFamilyIndex,
                        mContext.mWindow.mSwapchain,
                        mContext.mWindow.mSwapViewList,
                        mContext.mWindow.mSwapImageList,
                        mContext.mWindow.mSwapchainFormat,
                        mContext.mWindow.mSwapchainExtent))
        return false;

    return true;
}

void App::RunLoop()
{
    bool IsRunning = true;
    uint64_t numFrames = 0;
    while(IsRunning)
    {
        while(SDL_PollEvent(&mCurrentEvent))
        {
            if(mCurrentEvent.type == SDL_EVENT_QUIT)
                IsRunning = false;
            if(mCurrentEvent.type == SDL_EVENT_WINDOW_RESIZED)
                HandleResize();
        }
        //Update
        numFrames++;
        UpdateUniforms(mRenderer.GetCurrentFrameInd());
        if(!DrawFrame())
            HandleResize();
    }
    //Derived-app members (pipelines, buffers) destruct before mRenderer/mContext
    //do, so we can't rely on Renderer's destructor to wait for the GPU - by the
    //time it runs, those resources are already gone. Wait here instead, before
    //any destruction has begun.
    if(mContext.mDevice.mLogicalDevice != VK_NULL_HANDLE)
        vkDeviceWaitIdle(mContext.mDevice.mLogicalDevice);
}

bool App::HandleResize()
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(mContext.mWindow.mWindow, &w, &h);
    while(w == 0 || h == 0)
    {
        SDL_WaitEvent(&mCurrentEvent);
        SDL_GetWindowSizeInPixels(mContext.mWindow.mWindow, &w, &h);
    }

    if(!mContext.mWindow.RecreateSwapchain())
        return false;

    return mRenderer.Setup(
                        mContext.mDevice.mLogicalDevice,
                        mContext.mDevice.mGraphicsQueue,
                        mContext.mDevice.mPresentQueue,
                        mContext.mDevice.mGraphicsQueueFamilyIndex,
                        mContext.mWindow.mSwapchain,
                        mContext.mWindow.mSwapViewList,
                        mContext.mWindow.mSwapImageList,
                        mContext.mWindow.mSwapchainFormat,
                        mContext.mWindow.mSwapchainExtent);

}