#ifndef _RENDER_DATA_HANDLER_
#define _RENDER_DATA_HANDLER_

#include "volk.h"
#include "vk_mem_alloc.h"

#include "renderObject.h"

#include <cstring>
#include <vector>

template<typename... Attributes>
class VulkanDataHandler {
public:
    VulkanDataHandler() = default;
    ~VulkanDataHandler() {
        for(RenderObject<Attributes ...>& object : mObjects)
            object.Destroy(hAllocator);
    }
    inline void Setup(VmaAllocator const& aAllocator) {
        hAllocator = aAllocator;
    }
    bool AddRenderObject(std::vector<Attributes> const& ... aAttribs, std::vector<uint32_t> const& aIndices) {
        bool createSuccess = true;
        RenderObject<Attributes ...> obj;
        obj.attributeData = std::tuple<VulkanDataAllocation<Attributes>...>(
            CreateBuffer(aAttribs, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, createSuccess)...
        );
        obj.indData = CreateBuffer(aIndices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, createSuccess);
        if(!createSuccess)
        {
            obj.Destroy(hAllocator);
            return false;
        }
        mObjects.push_back(std::move(obj));
        return true;
    }
    VmaAllocator hAllocator = VK_NULL_HANDLE;
    std::vector<RenderObject<Attributes ...>> mObjects{};
private:
    template<typename T>
    VulkanDataAllocation<T> CreateBuffer(std::vector<T> const& aData, VkBufferUsageFlags aUsage, bool &aSuccess)
    {
        VulkanDataAllocation<T> result;
        result.count = static_cast<uint32_t>(aData.size());
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(T) * aData.size();
        bufferInfo.usage = aUsage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        VmaAllocationInfo allocResultInfo{};
        if(vmaCreateBuffer(hAllocator, &bufferInfo, &allocInfo, &result.buffer, &result.allocation, &allocResultInfo) != VK_SUCCESS) {
            aSuccess = false;
            return result;
        }
        memcpy(allocResultInfo.pMappedData, aData.data(), bufferInfo.size);
        return result;
    }
};

#endif