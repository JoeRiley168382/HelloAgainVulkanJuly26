#ifndef _VULKAN_PIPELINE_
#define _VULKAN_PIPELINE_

#include "volk.h"
#include <fstream>
#include <string>
#include <vector>

class VulkanPipeline {
public:
    VulkanPipeline() = default;
    inline void Init(VkDevice const aDevice, VkFormat const aFormat)  { hDevice = aDevice; hFormat = aFormat; }
    bool AddShader(std::string const& aPath);
    bool Setup();
    VkDevice hDevice;
    VkFormat hFormat;
    VkPipeline mPipe = VK_NULL_HANDLE;
    VkPipelineLayout mLayout = VK_NULL_HANDLE;
    std::vector<char> mVertCode{};
    std::vector<char> mFragCode{};
    //Add module variables if we ever use tesselation/compute/geometry etc etc
};

#endif