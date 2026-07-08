#include "drawList.h"

void DrawList::AddPipeline(VkPipeline const aPipe, VkPipelineLayout const aLO, std::vector<VkDescriptorSet> const& aDescriptorSets)
{
    if(aPipe == VK_NULL_HANDLE || aLO == VK_NULL_HANDLE)
        return;
    mPipelineList.push_back(aPipe);
    mPipeLayoutList.push_back(aLO);
    mDescriptorSetList.push_back(aDescriptorSets);
}

void DrawList::AddRenderObject(VulkanRenderData aRenderData)
{
    mRenderObjectList.push_back(std::move(aRenderData));
}