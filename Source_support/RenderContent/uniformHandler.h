#ifndef _RENDER_UNIFORM_HANDLER_
#define _RENDER_UNIFORM_HANDLER_

#include "volk.h"
#include "vk_mem_alloc.h"

#include <cstring>
#include <vector>

template<typename T>
class VulkanUniformHandler {
public:
    VulkanUniformHandler() = default;
    ~VulkanUniformHandler() {
        for(FrameData const& frame : mFrames)
            if(frame.buffer != VK_NULL_HANDLE)
                vmaDestroyBuffer(hAllocator, frame.buffer, frame.allocation);
        if(mPool != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(hDevice, mPool, nullptr);
        if(mLayout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(hDevice, mLayout, nullptr);
    }

    bool Setup(VkDevice aDevice, VmaAllocator aAllocator, uint32_t aFrameCount) {
        hDevice = aDevice;
        hAllocator = aAllocator;

        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &binding;
        if(vkCreateDescriptorSetLayout(hDevice, &layoutInfo, nullptr, &mLayout) != VK_SUCCESS)
            return false;

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = aFrameCount;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = aFrameCount;
        if(vkCreateDescriptorPool(hDevice, &poolInfo, nullptr, &mPool) != VK_SUCCESS)
            return false;

        mFrames.resize(aFrameCount);
        for(uint32_t i = 0; i < aFrameCount; i++) {
            if(!CreateBuffer(mFrames[i]))
                return false;

            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = mPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &mLayout;
            if(vkAllocateDescriptorSets(hDevice, &allocInfo, &mFrames[i].set) != VK_SUCCESS)
                return false;

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = mFrames[i].buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(T);

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = mFrames[i].set;
            write.dstBinding = 0;
            write.descriptorCount = 1;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.pBufferInfo = &bufferInfo;
            vkUpdateDescriptorSets(hDevice, 1, &write, 0, nullptr);
        }
        return true;
    }

    void Update(uint32_t aFrameInd, T const& aData) {
        memcpy(mFrames[aFrameInd].mapped, &aData, sizeof(T));
    }

    VkDescriptorSet GetSet(uint32_t aFrameInd) const { return mFrames[aFrameInd].set; }
    VkDescriptorSetLayout mLayout = VK_NULL_HANDLE;

private:
    struct FrameData {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        void* mapped = nullptr;
        VkDescriptorSet set = VK_NULL_HANDLE;
    };

    bool CreateBuffer(FrameData& aFrame) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(T);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        VmaAllocationInfo allocResultInfo{};
        if(vmaCreateBuffer(hAllocator, &bufferInfo, &allocInfo, &aFrame.buffer, &aFrame.allocation, &allocResultInfo) != VK_SUCCESS)
            return false;
        aFrame.mapped = allocResultInfo.pMappedData;
        return true;
    }

    VkDevice hDevice = VK_NULL_HANDLE;
    VmaAllocator hAllocator = VK_NULL_HANDLE;
    VkDescriptorPool mPool = VK_NULL_HANDLE;
    std::vector<FrameData> mFrames{};
};

#endif
