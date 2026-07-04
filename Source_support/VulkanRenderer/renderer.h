#ifndef _VULKAN_RENDERER_
#define _VULKAN_RENDERER_

#include "volk.h"
#include <vector>

class VulkanRenderer{
public:
    VulkanRenderer();
    ~VulkanRenderer();
    bool Setup(VkDevice, VkQueue, VkQueue, uint32_t, VkSwapchainKHR const, const std::vector<VkImageView>&, const std::vector<VkImage>&, VkFormat, VkExtent2D);
    //When we setup the app or add another class to handle data processing we can yank the info from the Vulkanpipeline class here instead of having to inherit it
    void AddPipeline(VkPipeline const, VkPipelineLayout const);
    void RenderFrame();
    //Handles: we obtain these from a VulkanApp object
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
    void RecordCommands();
    void SubmitCommands();
    //Members: these belong to us in full
    std::vector<VkPipeline> mPipelineList{};
    std::vector<VkPipelineLayout> mPipeLayoutList{};
    VkCommandPool mCmdPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> mCmdBuff{};
    std::vector<VkSemaphore> mImageAvailable{};
    std::vector<VkSemaphore> mRenderFinished{};
    std::vector<VkFence> mFrameInFlight{};
    uint32_t mCurrentImageInd = 0;
    uint32_t mCurrentFrameInd = 0;
    uint32_t mCurrentPipelineInd = 0;
};


#endif