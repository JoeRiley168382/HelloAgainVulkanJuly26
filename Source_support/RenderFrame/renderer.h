#ifndef _VULKAN_RENDERER_
#define _VULKAN_RENDERER_

#include "volk.h"
#include <vector>

#include "RenderContent/drawList.h"

class Renderer{
public:
    Renderer();
    ~Renderer();
    bool Setup(VkDevice, VkQueue, VkQueue, uint32_t, VkSwapchainKHR const, const std::vector<VkImageView>&, const std::vector<VkImage>&, VkFormat, VkExtent2D);
    bool RenderFrame(DrawList const& aDrawList);
    //Frame-in-flight index that the *next* RenderFrame call will record/present with.
    //Uniform data for that frame should be written before calling RenderFrame.
    inline uint32_t GetCurrentFrameInd() const { return mCurrentFrameInd; }
    //Handles: we obtain these from an App object
    VkDevice hDevice = VK_NULL_HANDLE;
    uint32_t hGraphicsQueueFamilyIndex = UINT32_MAX;
    VkQueue hGraphicsQueue = VK_NULL_HANDLE;
    VkQueue hPresentQueue = VK_NULL_HANDLE;
    const std::vector<VkImageView>* hImageViewList = nullptr;
    const std::vector<VkImage>* hImageList = nullptr;
    VkFormat hRenderFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D hRenderExtent{};
    VkSwapchainKHR hSwapchain = VK_NULL_HANDLE;

//May be in get/set hell with this down the line but I'm not sure if anything else will want to interfere with these
protected:
    //TRUE = we can use the swapchain as is. FALSE = we need to recreate it (somehow)
    bool AcquireSwapchain();
    //TODO: Maybe need an UpdateState/UpdateUniforms down the line?
    void RecordCommands(DrawList const& aDrawList);
    void SubmitCommands();
    //Members: these belong to us in full
    VkCommandPool mCmdPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> mCmdBuff{};
    std::vector<VkSemaphore> mImageAvailable{};
    std::vector<VkSemaphore> mRenderFinished{};
    std::vector<VkFence> mFrameInFlight{};

    uint32_t mCurrentImageInd = 0;
    uint32_t mCurrentFrameInd = 0;
    bool mSwapchainBeingRecreated = false;
};


#endif