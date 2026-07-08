#ifndef _VULKAN_PIPELINE_
#define _VULKAN_PIPELINE_

#include "volk.h"
#include <array>
#include <fstream>
#include <string>
#include <vector>
#include <utility>

#include "Maths/vec.h"

template <typename T> struct Stride2VkFormat;
template <> struct Stride2VkFormat<std::array<float,1>> { static constexpr VkFormat fmt = VK_FORMAT_R32_SFLOAT; };
template <> struct Stride2VkFormat<std::array<float,2>> { static constexpr VkFormat fmt = VK_FORMAT_R32G32_SFLOAT; };
template <> struct Stride2VkFormat<std::array<float,3>> { static constexpr VkFormat fmt = VK_FORMAT_R32G32B32_SFLOAT; };
template <> struct Stride2VkFormat<std::array<float,4>> { static constexpr VkFormat fmt = VK_FORMAT_R32G32B32A32_SFLOAT; };
template <> struct Stride2VkFormat<maths::vec3f> {  static constexpr VkFormat fmt = VK_FORMAT_R32G32B32_SFLOAT; };
template <> struct Stride2VkFormat<maths::vec4f> { static constexpr VkFormat fmt = VK_FORMAT_R32G32B32A32_SFLOAT; };


template <typename ... Attributes>
class VulkanPipeline {
public:
    VulkanPipeline() = default;
    ~VulkanPipeline() 
    {
        if(mPipe != VK_NULL_HANDLE)
            vkDestroyPipeline(hDevice, mPipe, nullptr);
        if(mLayout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(hDevice, mLayout, nullptr);
    }  
    inline void Init(VkDevice const aDevice, VkFormat const aFormat)  { hDevice = aDevice; hFormat = aFormat; }
    bool AddShader(std::string const& aPath) 
    {
        std::ifstream file(std::string(SHADER_DIR) + aPath, std::ios::binary | std::ios::ate);
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
         if(aPath.rfind(".vert.spv") != std::string::npos) {
            mVertCode.resize(fileSize);
            file.read(mVertCode.data(), fileSize);
        }
        else if(aPath.rfind(".frag.spv") != std::string::npos) {
            mFragCode.resize(fileSize);
            file.read(mFragCode.data(), fileSize);
        }
        else
            return false;

        return true;
    }
    bool Setup(VkDescriptorSetLayout aDescriptorLayout = VK_NULL_HANDLE)
    {
        VkPipelineShaderStageCreateInfo stages[2]{};
        stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].pName = "main";
        VkShaderModuleCreateInfo vertModuleInfo{};
        vertModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertModuleInfo.codeSize = mVertCode.size();
        vertModuleInfo.pCode = reinterpret_cast<const uint32_t*>(mVertCode.data());
        stages[0].pNext = &vertModuleInfo;
        stages[0].module = VK_NULL_HANDLE;

        stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].pName = "main";
        VkShaderModuleCreateInfo fragModuleInfo{};
        fragModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fragModuleInfo.codeSize = mFragCode.size();
        fragModuleInfo.pCode = reinterpret_cast<const uint32_t*>(mFragCode.data());
        stages[1].pNext = &fragModuleInfo;
        stages[1].module = VK_NULL_HANDLE;

        constexpr size_t kAttribCount = sizeof...(Attributes);
        std::array<VkVertexInputBindingDescription, kAttribCount> bindings =
            MakeBindings(std::index_sequence_for<Attributes...>{});
        std::array<VkVertexInputAttributeDescription, kAttribCount> attribs =
            MakeAttribs(std::index_sequence_for<Attributes...>{});

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(kAttribCount);
        vertexInputInfo.pVertexBindingDescriptions = bindings.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(kAttribCount);
        vertexInputInfo.pVertexAttributeDescriptions = attribs.data();

         VkPipelineInputAssemblyStateCreateInfo assemblyInfo{};
        assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        assemblyInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportInfo{};
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.viewportCount = 1;
        viewportInfo.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterInfo{};
        rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterInfo.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo msaaInfo{};
        msaaInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        msaaInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState blendAttach{};
        blendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttach.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo blendInfo{};
        blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blendInfo.logicOpEnable = VK_FALSE;
        blendInfo.attachmentCount = 1;
        blendInfo.pAttachments = &blendAttach;

        VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynInfo{};
        dynInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynInfo.dynamicStateCount = 2;
        dynInfo.pDynamicStates = dynStates;

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = aDescriptorLayout != VK_NULL_HANDLE ? 1 : 0;
        layoutInfo.pSetLayouts = aDescriptorLayout != VK_NULL_HANDLE ? &aDescriptorLayout : nullptr;
        vkCreatePipelineLayout(hDevice, &layoutInfo, nullptr, &mLayout);

        VkPipelineRenderingCreateInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachmentFormats = &hFormat;

        VkGraphicsPipelineCreateInfo pipeInfo{};
        pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeInfo.stageCount = 2;
        pipeInfo.pStages = stages;
        pipeInfo.pVertexInputState = &vertexInputInfo;
        pipeInfo.pInputAssemblyState = &assemblyInfo;
        pipeInfo.pViewportState = &viewportInfo;
        pipeInfo.pRasterizationState = &rasterInfo;
        pipeInfo.pMultisampleState = &msaaInfo;
        pipeInfo.pColorBlendState = &blendInfo;
        pipeInfo.pDynamicState = &dynInfo;
        pipeInfo.pNext = &renderingInfo;
        pipeInfo.layout = mLayout;

        return vkCreateGraphicsPipelines(hDevice, VK_NULL_HANDLE, 1, &pipeInfo, nullptr, &mPipe) == VK_SUCCESS;

    }
    VkDevice hDevice;
    VkFormat hFormat;
    VkPipeline mPipe = VK_NULL_HANDLE;
    VkPipelineLayout mLayout = VK_NULL_HANDLE;
    std::vector<char> mVertCode{};
    std::vector<char> mFragCode{};
    //Add module variables if we ever use tesselation/compute/geometry etc etc
private:
    template<size_t ... I>
    static std::array<VkVertexInputBindingDescription, sizeof...(Attributes)> MakeBindings(std::index_sequence<I...>) {
        return { VkVertexInputBindingDescription{
            static_cast<uint32_t>(I), static_cast<uint32_t>(sizeof(Attributes)), VK_VERTEX_INPUT_RATE_VERTEX
        }... };
    }
    template<size_t ... I>
    static std::array<VkVertexInputAttributeDescription, sizeof...(Attributes)> MakeAttribs(std::index_sequence<I...>) {
        return { VkVertexInputAttributeDescription{
            static_cast<uint32_t>(I), static_cast<uint32_t>(I), Stride2VkFormat<Attributes>::fmt, 0
        }... };
    }

};

#endif