#ifndef _RENDER_DATA_STRUCT_
#define _RENDER_DATA_STRUCT_

#include "volk.h"
#include "vk_mem_alloc.h"

#include <tuple>
#include <vector>

template<typename T>
struct VulkanDataAllocation {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    static constexpr uint32_t stride = sizeof(T);
    uint32_t count = 0;
    void Destroy(VmaAllocator aAllocator) {
        if(buffer != VK_NULL_HANDLE)
            vmaDestroyBuffer(aAllocator, buffer, allocation);
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }
};

//This is the Data we need to Render in Vulkan
//does not need to know about CPU-side attribute types
struct VulkanRenderData {
    std::vector<VkBuffer> vertexBufferList{};
    std::vector<VkDeviceSize> vertexOffsetList{};
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    uint32_t numIndices = 0;
};

//Claude: 
// Fold-over-tuple functors used via std::apply. Not templated on AttrTypes
// themselves - their operator() is independently generic over whatever
// pack std::apply hands them, so they're plain reusable types.
struct DataSetup{
    VulkanRenderData& hData;
    template<typename ... AttributeData>
    void operator()(AttributeData const& ... aData) const {
        (hData.vertexBufferList.push_back(aData.buffer), ...);
        (hData.vertexOffsetList.push_back(0), ...);
    }
};

struct DataShutdown{
    VmaAllocator hAllocator;
    template<typename ... AttributeData>
    void operator()(AttributeData& ... aData) const {
        (aData.Destroy(hAllocator), ...);
    }
};

template<typename ... Attributes>
struct RenderObject
{
    std::tuple<VulkanDataAllocation<Attributes>...> attributeData;
    VulkanDataAllocation<uint32_t> indData;
    VulkanRenderData GetRenderData() const {
        VulkanRenderData data;
        std::apply(DataSetup{data}, attributeData);
        data.indexBuffer = indData.buffer;
        data.numIndices = indData.count;
        return data;
    }
    void Destroy(VmaAllocator aAlloc) {
        std::apply(DataShutdown{aAlloc}, attributeData);
        indData.Destroy(aAlloc);
    }
};



#endif