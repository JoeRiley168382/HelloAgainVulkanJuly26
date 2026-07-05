#include "triangleStubApp0.h"

void BeeTriangleApp::SetupPipelines()
{
    mTriPipe.Init(mContext.mDevice.mLogicalDevice, mContext.mWindow.mSwapchainFormat);
    mTriPipe.AddShader("blackTriangle.vert.spv");
    mTriPipe.AddShader("blackTriangle.frag.spv");
    mTriPipe.Setup();

    mRenderer.AddPipeline(mTriPipe.mPipe, mTriPipe.mLayout);
}
