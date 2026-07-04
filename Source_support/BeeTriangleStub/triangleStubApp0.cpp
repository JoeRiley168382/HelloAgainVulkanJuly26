#include "triangleStubApp0.h"

void BeeTriangleApp::Start()
{
    mContext.Setup(mWinWidth, mWinHeight);

    mTriPipe.Init(mContext.mDevice.mLogicalDevice, mContext.mWindow.mSwapchainFormat);
    mTriPipe.AddShader("blackTriangle.vert.spv");
    mTriPipe.AddShader("blackTriangle.frag.spv");
    mTriPipe.Setup();

    mRenderer.Setup(mContext.mDevice.mLogicalDevice,
                mContext.mDevice.mGraphicsQueue, 
                mContext.mDevice.mPresentQueue, 
                mContext.mDevice.mGraphicsQueueFamilyIndex, 
                mContext.mWindow.mSwapchain, 
                mContext.mWindow.mSwapViewList, 
                mContext.mWindow.mSwapImageList, 
                mContext.mWindow.mSwapchainFormat,
                mContext.mWindow.mSwapchainExtent);

    mRenderer.AddPipeline(mTriPipe.mPipe, mTriPipe.mLayout);
}

void BeeTriangleApp::RunLoop()
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

void BeeTriangleApp::Stop()
{
}
