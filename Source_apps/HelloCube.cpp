#include "App/app.h"
#include "RenderContent/pipeline.h"
#include "RenderContent/dataHandler.h"
#include "RenderContent/drawList.h"
#include "RenderContent/uniformData.h"
#include "RenderContent/uniformHandler.h"
#include "Maths/vec.h"
#include "Maths/mat.h"
#include "Maths/Transform.h"

#include <array>
#include <vector>

class CubeApp : public App 
{
protected:
    void SetupPipelines() override {
        mUniformHandler.Setup(mContext.mDevice.mLogicalDevice, mContext.mAllocator, 2);

        mPipeline.Init(mContext.mDevice.mLogicalDevice, mContext.mWindow.mSwapchainFormat);
        mPipeline.AddShader("cube.vert.spv");
        mPipeline.AddShader("cube.frag.spv");
        mPipeline.Setup(mUniformHandler.mLayout);

        std::vector<VkDescriptorSet> sets = { mUniformHandler.GetSet(0), mUniformHandler.GetSet(1) };
        mDrawList.AddPipeline(mPipeline.mPipe, mPipeline.mLayout, sets);
    }

    void UpdateUniforms(uint32_t aFrameInd) override {
        mElapsedSeconds += 1.0f / 60.0f; // placeholder — see note below
        MVPUniform mvp;
        maths::mat4f model = maths::RotateYaw(mElapsedSeconds);
        maths::mat4f view = maths::Translate({0.0f, 0.0f, -3.0f}); // pushes the cube back from the eye
        maths::mat4f proj = maths::Perspective(1.0f /*~57 deg*/, 800.0f/600.0f, 0.1f, 100.0f);
        mvp.mvp = proj * view * model;
        mUniformHandler.Update(aFrameInd, mvp);
    }

    void SetupRenderObjects() override {
        
        mDataHandler.Setup(mContext.mAllocator);

        std::vector<maths::vec3f> positions = {
            {-0.5f, -0.5f,  0.5f}, // 0: front-bottom-left
            { 0.5f, -0.5f,  0.5f}, // 1: front-bottom-right
            { 0.5f,  0.5f,  0.5f}, // 2: front-top-right
            {-0.5f,  0.5f,  0.5f}, // 3: front-top-left
            {-0.5f, -0.5f, -0.5f}, // 4: back-bottom-left
            { 0.5f, -0.5f, -0.5f}, // 5: back-bottom-right
            { 0.5f,  0.5f, -0.5f}, // 6: back-top-right
            {-0.5f,  0.5f, -0.5f}, // 7: back-top-left
        };
        std::vector<maths::vec3f> colors = {
            {1.0f, 0.0f, 0.0f}, // red
            {0.0f, 1.0f, 0.0f}, // green
            {0.0f, 0.0f, 1.0f}, // blue
            {1.0f, 1.0f, 0.0f}, // yellow
            {1.0f, 0.0f, 1.0f}, // magenta
            {0.0f, 1.0f, 1.0f}, // cyan
            {1.0f, 1.0f, 1.0f}, // white
            {0.5f, 0.5f, 0.5f}, // gray
        };
        std::vector<uint32_t> indices = {
            0,1,2,  0,2,3, // front  (+Z)
            4,6,5,  4,7,6, // back   (-Z)
            1,5,6,  1,6,2, // right  (+X)
            0,7,4,  0,3,7, // left   (-X)
            3,6,7,  3,2,6, // top    (+Y)
            0,4,5,  0,5,1, // bottom (-Y)
        };

        mDataHandler.AddRenderObject(positions, colors, indices);
        mDrawList.AddRenderObject(mDataHandler.mObjects.back().GetRenderData());
    }

    bool DrawFrame() override {
        return mRenderer.RenderFrame(mDrawList);
    }

    VulkanPipeline<maths::vec3f, maths::vec3f> mPipeline;
    VulkanDataHandler<maths::vec3f, maths::vec3f> mDataHandler;
    VulkanUniformHandler<MVPUniform> mUniformHandler;
    DrawList mDrawList;
    float mElapsedSeconds = 0.0f;
};

int main(int argc, char** argv) {
    CubeApp app;
    if (!app.Start(800, 600))
        return -1;
    app.RunLoop();
    return 0;
}