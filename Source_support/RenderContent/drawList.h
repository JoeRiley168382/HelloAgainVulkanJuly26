#ifndef _VULKAN_DRAW_LIST_
#define _VULKAN_DRAW_LIST_

#include "volk.h"
#include "renderObject.h"

#include <vector>

class DrawList {
public:
    // aDescriptorSets, if non-empty, must have one entry per frame-in-flight
    // (indexed the same way as Renderer's mCurrentFrameInd).
    void AddPipeline(VkPipeline const, VkPipelineLayout const, std::vector<VkDescriptorSet> const& aDescriptorSets = {});
    void AddRenderObject(VulkanRenderData);

    std::vector<VulkanRenderData> mRenderObjectList{};
    std::vector<VkPipelineLayout> mPipeLayoutList{};
    std::vector<VkPipeline> mPipelineList{};
    std::vector<std::vector<VkDescriptorSet>> mDescriptorSetList{};
    uint32_t mCurrentPipelineInd = 0;
};

#endif