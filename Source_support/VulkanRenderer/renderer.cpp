#include "renderer.h"

VulkanRenderer::VulkanRenderer()
{
}

VulkanRenderer::~VulkanRenderer()
{
    if(hDevice != VK_NULL_HANDLE)
        vkDeviceWaitIdle(hDevice);
    for(uint32_t i = 0; i < mPipelineList.size(); i++)
    {
        vkDestroyPipeline(hDevice, mPipelineList[i], nullptr);
        vkDestroyPipelineLayout(hDevice, mPipeLayoutList[i], nullptr);
    }
    vkDestroyFence(hDevice, mFrameInFlight[1], nullptr);
    vkDestroySemaphore(hDevice, mImageAvailable[1], nullptr);
    vkDestroyFence(hDevice, mFrameInFlight[0], nullptr);
    vkDestroySemaphore(hDevice, mImageAvailable[0], nullptr);
    for(uint32_t i = 0; i < mRenderFinished.size(); i++)
        vkDestroySemaphore(hDevice, mRenderFinished[i], nullptr);
    vkDestroyCommandPool(hDevice, mCmdPool, nullptr);
}

bool VulkanRenderer::Setup(
    VkDevice aLogicalDevice, 
    VkQueue aGraphicsQueue, 
    VkQueue aPresentQueue, 
    uint32_t aGraphicsIndex, 
    VkSwapchainKHR const aSwapchain, 
    const std::vector<VkImageView> & aImageViewList,
    const std::vector<VkImage> & aImageList, 
    VkFormat aSwapchainFormat, 
    VkExtent2D aSwapchainExtent)
{
    hDevice = aLogicalDevice;
    hGraphicsQueueFamilyIndex = aGraphicsIndex;
    hGraphicsQueue = aGraphicsQueue;
    hPresentQueue = aPresentQueue;
    hRenderFormat = aSwapchainFormat;
    hRenderExtent = aSwapchainExtent;
    hSwapchain = aSwapchain;
    hImageViewList = &aImageViewList;
    hImageList = &aImageList;

    //Vulkan context probably screwed up if any of these apply
    if( hGraphicsQueueFamilyIndex == UINT32_MAX ||
        hDevice == VK_NULL_HANDLE ||
        hGraphicsQueue == VK_NULL_HANDLE ||
        hPresentQueue == VK_NULL_HANDLE)
        return false;

    VkCommandPoolCreateInfo cmdPoolInfo{};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = hGraphicsQueueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if(mCmdPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(hDevice, mCmdPool, nullptr);
    if(vkCreateCommandPool(hDevice, &cmdPoolInfo, nullptr, &mCmdPool) != VK_SUCCESS)
        return false;
    mCmdBuff.resize(2);
    VkCommandBufferAllocateInfo cmdBuffInfo{};
    cmdBuffInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBuffInfo.commandPool = mCmdPool;
    cmdBuffInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBuffInfo.commandBufferCount = 2;
    if(vkAllocateCommandBuffers(hDevice, &cmdBuffInfo, mCmdBuff.data()) != VK_SUCCESS)
        return false;
    
    if(!mRenderFinished.empty())
    {
        for(VkSemaphore &sem : mRenderFinished)
            vkDestroySemaphore(hDevice, sem, nullptr);
        mRenderFinished.clear();
    }
    for (uint32_t i = 0; i < aImageList.size(); i++) {
        VkSemaphoreCreateInfo renderFinishedInfo{};
        renderFinishedInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkSemaphore currentRenderFinished = VK_NULL_HANDLE;
        if(vkCreateSemaphore(hDevice, &renderFinishedInfo, nullptr, &currentRenderFinished) != VK_SUCCESS)
            return false;
        mRenderFinished.push_back(currentRenderFinished);
    }
    if(mFrameInFlight.empty()) {
        mImageAvailable.resize(2);
        mFrameInFlight.resize(2);
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(hDevice, &fenceInfo, nullptr, &mFrameInFlight[0]);
        vkCreateFence(hDevice, &fenceInfo, nullptr, &mFrameInFlight[1]);
        VkSemaphoreCreateInfo imgAvailInfo{};
        imgAvailInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vkCreateSemaphore(hDevice, &imgAvailInfo, nullptr, &mImageAvailable[0]);
        vkCreateSemaphore(hDevice, &imgAvailInfo, nullptr, &mImageAvailable[1]);
    }
    return true;
}

void VulkanRenderer::AddPipeline(VkPipeline const aPipe, VkPipelineLayout const aLO)
{
    if(aPipe == VK_NULL_HANDLE || aLO == VK_NULL_HANDLE)
        return;
    mPipelineList.push_back(aPipe);
    mPipeLayoutList.push_back(aLO);
}

bool VulkanRenderer::RenderFrame()
{
    vkWaitForFences(hDevice, 1, &mFrameInFlight[mCurrentFrameInd], VK_TRUE, UINT64_MAX);
    vkResetFences(hDevice, 1, &mFrameInFlight[mCurrentFrameInd]);
    if(!AcquireSwapchain()) return false;
    RecordCommands();
    SubmitCommands();
    //This info is passed through submitCommands, helps catch swapchain resizes a frame earlier
    if(mSwapchainBeingRecreated) return false;
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &mRenderFinished[mCurrentImageInd];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &hSwapchain;
    presentInfo.pImageIndices = &mCurrentImageInd;
    VkResult presentResult = vkQueuePresentKHR(hPresentQueue, &presentInfo);
    if(presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
        return false;
    mCurrentFrameInd += 1;
    mCurrentFrameInd %= 2;
    return true;
}

bool VulkanRenderer::AcquireSwapchain()
{
    VkResult result = vkAcquireNextImageKHR(hDevice, hSwapchain, UINT64_MAX, mImageAvailable[mCurrentFrameInd], VK_NULL_HANDLE, &mCurrentImageInd);
    if(result == VK_ERROR_OUT_OF_DATE_KHR)
        return false;
    mSwapchainBeingRecreated = result == VK_SUBOPTIMAL_KHR;
    return result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR;
}

void VulkanRenderer::RecordCommands()
{
    vkResetCommandBuffer(mCmdBuff[mCurrentFrameInd], 0);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;
    vkBeginCommandBuffer(mCmdBuff[mCurrentFrameInd], &beginInfo);
    VkImageMemoryBarrier2 undef2ColorImgBarr{};

    undef2ColorImgBarr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

    undef2ColorImgBarr.image = (*hImageList)[mCurrentImageInd];
    
    undef2ColorImgBarr.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    undef2ColorImgBarr.srcAccessMask = VK_ACCESS_2_NONE;
    undef2ColorImgBarr.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    undef2ColorImgBarr.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    undef2ColorImgBarr.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    undef2ColorImgBarr.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    undef2ColorImgBarr.subresourceRange = VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkDependencyInfo undef2ColorDepInfo{};
    undef2ColorDepInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
    undef2ColorDepInfo.imageMemoryBarrierCount = 1;
    undef2ColorDepInfo.pImageMemoryBarriers = &undef2ColorImgBarr;
    vkCmdPipelineBarrier2(mCmdBuff[mCurrentFrameInd], &undef2ColorDepInfo);

    VkRenderingAttachmentInfo colorAttach{};
    colorAttach.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttach.imageView = (*hImageViewList)[mCurrentImageInd];
    colorAttach.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkClearValue clearYellow{};
    clearYellow.color = {{1.0f, 1.0f, 0.0f, 1.0f}};
    colorAttach.clearValue = clearYellow;

    VkRenderingInfo renderInfo{};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.renderArea = {0,0,hRenderExtent.width, hRenderExtent.height};
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = &colorAttach;
    //TODO: Need to figure out how switching pipelines will work in terms of passing messages from the app class
    vkCmdBeginRendering(mCmdBuff[mCurrentFrameInd], &renderInfo);
    //Need this since viewport is dynamically set
    VkViewport viewport{};
    viewport.width = (float)hRenderExtent.width;
    viewport.height = (float)hRenderExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(mCmdBuff[mCurrentFrameInd], 0, 1, &viewport);
    //Need this since scissor is dynamically set
    VkRect2D scissor{};
    scissor.extent = hRenderExtent;
    vkCmdSetScissor(mCmdBuff[mCurrentFrameInd], 0, 1, &scissor);
    
    if(mPipelineList.empty() == false)
    {
        vkCmdBindPipeline(mCmdBuff[mCurrentFrameInd], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineList[mCurrentPipelineInd]);
        //TODO: change this when using buffers!!
        vkCmdDraw(mCmdBuff[mCurrentFrameInd], 3, 1, 0, 0);
    }
    vkCmdEndRendering(mCmdBuff[mCurrentFrameInd]);

    VkImageMemoryBarrier2 color2undefImgBarr{};

    color2undefImgBarr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    color2undefImgBarr.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    color2undefImgBarr.image = (*hImageList)[mCurrentImageInd];
    color2undefImgBarr.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    color2undefImgBarr.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    color2undefImgBarr.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color2undefImgBarr.dstAccessMask = VK_ACCESS_2_NONE;
    color2undefImgBarr.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
    color2undefImgBarr.subresourceRange = VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo color2undefDepInfo{};
    color2undefDepInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
    color2undefDepInfo.imageMemoryBarrierCount = 1;
    color2undefDepInfo.pImageMemoryBarriers = &color2undefImgBarr;
    vkCmdPipelineBarrier2(mCmdBuff[mCurrentFrameInd], &color2undefDepInfo);
    vkEndCommandBuffer(mCmdBuff[mCurrentFrameInd]);
}

void VulkanRenderer::SubmitCommands()
{
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &mImageAvailable[mCurrentFrameInd];
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCmdBuff[mCurrentFrameInd];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &mRenderFinished[mCurrentImageInd];
    vkQueueSubmit(hGraphicsQueue, 1, &submitInfo, mFrameInFlight[mCurrentFrameInd]);
}
