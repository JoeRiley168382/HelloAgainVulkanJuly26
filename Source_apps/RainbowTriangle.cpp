#include "App/app.h"
#include "RenderContent/pipeline.h"
#include "RenderContent/dataHandler.h"
#include "RenderContent/drawList.h"

#include <array>
#include <vector>

class RainbowTriangleApp : public App {
protected:
    void SetupPipelines() override {
        mPipeline.Init(mContext.mDevice.mLogicalDevice, mContext.mWindow.mSwapchainFormat);
        mPipeline.AddShader("passthru2DwithColor.vert.spv");
        mPipeline.AddShader("passthru.frag.spv");
        mPipeline.Setup();
        mDrawList.AddPipeline(mPipeline.mPipe, mPipeline.mLayout);
    }

    void SetupRenderObjects() override {
        mDataHandler.Setup(mContext.mAllocator);

        std::vector<std::array<float,2>> positions = {
            { 0.0f, -0.5f},
            { 0.5f,  0.5f},
            {-0.5f,  0.5f}
        };
        std::vector<std::array<float,3>> colors = {
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f}
        };
        std::vector<uint32_t> indices = {0, 1, 2};

        mDataHandler.AddRenderObject(positions, colors, indices);
        mDrawList.AddRenderObject(mDataHandler.mObjects.back().GetRenderData());
    }

    bool DrawFrame() override {
        return mRenderer.RenderFrame(mDrawList);
    }

    VulkanPipeline<std::array<float,2>, std::array<float,3>> mPipeline;
    VulkanDataHandler<std::array<float,2>, std::array<float,3>> mDataHandler;
    DrawList mDrawList;
};

int main(int argc, char** argv) {
    RainbowTriangleApp app;
    if (!app.Start(800, 600))
        return -1;
    app.RunLoop();
    return 0;
}