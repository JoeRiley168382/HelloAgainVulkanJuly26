#include "stub.h"

void StubApp::Start()
{
    mContext.Setup(mWinWidth, mWinHeight);
    mRenderer.Setup(mContext.mDevice.mLogicalDevice,
                    mContext.mDevice.mGraphicsQueue, 
                    mContext.mDevice.mPresentQueue, 
                    mContext.mDevice.mGraphicsQueueFamilyIndex, 
                    mContext.mWindow.mSwapchain, 
                    mContext.mWindow.mSwapViewList, 
                    mContext.mWindow.mSwapImageList, 
                    mContext.mWindow.mSwapchainFormat,
                    mContext.mWindow.mSwapchainExtent);
}

void StubApp::RunLoop()
{
    bool IsRunning = true;
    uint64_t numFrames = 0;
    while(IsRunning)
    {
        while(SDL_PollEvent(&mCurrentEvent))
        {
            if(mCurrentEvent.type == SDL_EVENT_QUIT)
                IsRunning = false;
        }
        //Update
        numFrames++;
        mRenderer.RenderFrame();
    }
}

void StubApp::Stop()
{
}
