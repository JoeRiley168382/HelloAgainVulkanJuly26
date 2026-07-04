#include "renderer.h"

VulkanRenderer::VulkanRenderer()
{
}

VulkanRenderer::~VulkanRenderer()
{
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
    if(vkCreateCommandPool(hDevice, &cmdPoolInfo, nullptr, &mCmdPool) != VK_SUCCESS)
        return false;
    VkCommandBufferAllocateInfo cmdBuffInfo{};
    cmdBuffInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBuffInfo.commandPool = mCmdPool;
    cmdBuffInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBuffInfo.commandBufferCount = 1;
    if(vkAllocateCommandBuffers(hDevice, &cmdBuffInfo, &mCmdBuff) != VK_SUCCESS)
        return false;
    
    VkSemaphoreCreateInfo imgAvailInfo{};
    VkSemaphoreCreateInfo renderFinishedInfo{};
    imgAvailInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    renderFinishedInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if(vkCreateSemaphore(hDevice, &imgAvailInfo, nullptr, &mCurrentImageAvailable) != VK_SUCCESS)
        return false;
    if(vkCreateSemaphore(hDevice, &renderFinishedInfo, nullptr, &mCurrentRenderFinished) != VK_SUCCESS)
        return false;
    VkFenceCreateInfo inFlightInfo{};
    inFlightInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    inFlightInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if(vkCreateFence(hDevice, &inFlightInfo, nullptr, &mCurrentFrameInFlight) != VK_SUCCESS)
        return false;

    return true;
}


void VulkanRenderer::RenderFrame()
{
    vkWaitForFences(hDevice, 1, &mCurrentFrameInFlight, VK_TRUE, UINT64_MAX);
    vkResetFences(hDevice, 1, &mCurrentFrameInFlight);
    if(!AcquireSwapchain()) return;
    RecordCommands();
    SubmitCommands();
    
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &mCurrentRenderFinished;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &hSwapchain;
    presentInfo.pImageIndices = &mCurrentImageInd;
    vkQueuePresentKHR(hPresentQueue, &presentInfo);
}

bool VulkanRenderer::AcquireSwapchain()
{
    VkResult result = vkAcquireNextImageKHR(hDevice, hSwapchain, UINT64_MAX, mCurrentImageAvailable, VK_NULL_HANDLE, &mCurrentImageInd);
    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        return false;
    else
        return true;
}

void VulkanRenderer::RecordCommands()
{
    vkResetCommandBuffer(mCmdBuff, 0);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;
    vkBeginCommandBuffer(mCmdBuff, &beginInfo);
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
    vkCmdPipelineBarrier2(mCmdBuff, &undef2ColorDepInfo);

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
    vkCmdBeginRendering(mCmdBuff, &renderInfo);
    vkCmdEndRendering(mCmdBuff);

    VkImageMemoryBarrier2 color2undefImgBarr{};

    color2undefImgBarr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    color2undefImgBarr.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    color2undefImgBarr.image = (*hImageList)[mCurrentImageInd];
    color2undefImgBarr.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    color2undefImgBarr.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    color2undefImgBarr.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color2undefImgBarr.dstAccessMask = VK_ACCESS_2_NONE;
    color2undefImgBarr.dstStageMask = VK_PIPELINE_STAGE_2_NONE;

    VkDependencyInfo color2undefDepInfo{};
    color2undefDepInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
    color2undefDepInfo.imageMemoryBarrierCount = 1;
    color2undefDepInfo.pImageMemoryBarriers = &color2undefImgBarr;
    vkCmdPipelineBarrier2(mCmdBuff, &color2undefDepInfo);
    vkEndCommandBuffer(mCmdBuff);
}

void VulkanRenderer::SubmitCommands()
{
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &mCurrentImageAvailable;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCmdBuff;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &mCurrentRenderFinished;
    vkQueueSubmit(hGraphicsQueue, 1, &submitInfo, mCurrentFrameInFlight);
}
