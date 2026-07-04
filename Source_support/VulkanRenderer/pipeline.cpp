#include "pipeline.h"

bool VulkanPipeline::AddShader(std::string const &aPath)
{
    std::ifstream file(std::string(SHADER_DIR) + aPath, std::ios::binary | std::ios::ate);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    char *shaderData = (char*) malloc(fileSize * sizeof(char));
    file.read(shaderData, fileSize);

    VkShaderModuleCreateInfo shaderInfo{};
    bool retVal;
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = fileSize;
    shaderInfo.pCode = reinterpret_cast<const uint32_t*> (shaderData);
    if(aPath.rfind(".vert.spv") != std::string::npos)
        retVal = vkCreateShaderModule(hDevice, &shaderInfo, nullptr, &mVertModule) == VK_SUCCESS;
    else if(aPath.rfind(".frag.spv") != std::string::npos)
        retVal = vkCreateShaderModule(hDevice, &shaderInfo, nullptr, &mFragModule) == VK_SUCCESS;
    else
        retVal = false;
    free(shaderData);
    return retVal;
}

bool VulkanPipeline::Setup()
{
    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].pName = "main";
    stages[0].module = mVertModule;

    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].pName = "main";
    stages[1].module = mFragModule;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

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
    rasterInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

    //Need to expand with whatever we'll be using for passing in data
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
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

    bool retVal = vkCreateGraphicsPipelines(hDevice, VK_NULL_HANDLE, 1, &pipeInfo, nullptr, &mPipe) == VK_SUCCESS;
    vkDestroyShaderModule(hDevice, mVertModule, nullptr);
    mVertModule = VK_NULL_HANDLE;
    vkDestroyShaderModule(hDevice, mFragModule, nullptr);
    mFragModule = VK_NULL_HANDLE;
    return retVal;
}
