#include "czpch.h"
#include "VulkanPipelineBuilder.h"
#include"./Subsystems/Logging.h"
#include"VulkanCore.h"

namespace Czuch
{
    VulkanPipelineBuilder::VulkanPipelineBuilder(const VkDevice deviceObj, Pipeline_Vulkan* pipelineObj,const PipelineStateDesc* pipelineDesc):pipeline(pipelineObj),device(deviceObj),pipelineDescPtr(pipelineDesc)
    {
    }

     bool VulkanPipelineBuilder::BuildPipeline(const VkRenderPass renderPass)
    {
        pipeline->device = device;

        if (pipelineDescPtr->vs.IsValid())
        {
            Shader_Vulkan* vulkan_shader_vs = Internal_To_Shader(&pipelineDescPtr->vs);
            shaderStages.push_back(vulkan_shader_vs->shaderStageInfo);
        }

        if (pipelineDescPtr->ps.IsValid())
        {
            Shader_Vulkan* vulkan_shader_ps = Internal_To_Shader(&pipelineDescPtr->ps);
            shaderStages.push_back(vulkan_shader_ps->shaderStageInfo);
        }

        std::vector<VkDynamicState> dynamicStates = {
             VK_DYNAMIC_STATE_VIEWPORT,
             VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = CreateVertexInputInfo();
        VkPipelineMultisampleStateCreateInfo multiSamplingInfo = CreateMultisamplingInfo();
        CreateInputAssemblyInfo();
        CreateRasterizationStateInfo();
        VkPipelineColorBlendStateCreateInfo blendingInfo = CreateBlendingInfo();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        CreatePipelineLayout();

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = nullptr;

        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &pipeline->inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &pipeline->rasterizer;
        pipelineInfo.pMultisampleState = &multiSamplingInfo;
        pipelineInfo.pColorBlendState = &blendingInfo;
        pipelineInfo.layout = pipeline->pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(
            device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline->pipeline) != VK_SUCCESS) {
            LOG_BE_ERROR("{0} Failed to create new graphics pipeline.", "[VulkanPipelineBuilder]");
            return false;
        }
        else
        {
            return true;
        }
    }

    VkPipelineVertexInputStateCreateInfo VulkanPipelineBuilder::CreateVertexInputInfo()
    {
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkVertexInputBindingDescription bindingDescription[s_max_vertex_streams];

        const InputVertexLayout& vertLayout = pipelineDescPtr->il;

        for (int a = 0; a < vertLayout.vertexStreamsCount; ++a)
        {
            bindingDescription[a].binding = vertLayout.streams[a].binding;
            bindingDescription[a].stride = vertLayout.streams[a].stride;
            bindingDescription[a].inputRate = vertLayout.streams[a].input_rate == InputClassification::PER_VERTEX_DATA ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
        }
        
        vertexInputInfo.vertexBindingDescriptionCount = vertLayout.vertexStreamsCount;

        VkVertexInputAttributeDescription attributeDescription[s_max_vertex_attributes];

        for (int a = 0; a < vertLayout.vertexAttributesCount; ++a)
        {
            attributeDescription[a].binding = vertLayout.attributes[a].binding;
            attributeDescription[a].location = vertLayout.attributes[a].location;
            attributeDescription[a].offset = vertLayout.attributes[a].offset;
            attributeDescription[a].format = ConvertFormat(vertLayout.attributes[a].format);
        }

        vertexInputInfo.vertexAttributeDescriptionCount = vertLayout.vertexAttributesCount;

        //vertex bindings and attributes
        vertexInputInfo.pVertexBindingDescriptions = bindingDescription; 
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescription; 
        return vertexInputInfo;
    }

    void VulkanPipelineBuilder::CreateInputAssemblyInfo()
    {
        pipeline->inputAssembly.sType= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipeline->inputAssembly.topology = ConvertTopology(pipelineDescPtr->pt);
        pipeline->inputAssembly.primitiveRestartEnable = VK_FALSE;
        pipeline->inputAssembly.pNext = 0;
    }

    void VulkanPipelineBuilder::CreateRasterizationStateInfo()
    {
        pipeline->rasterizer.sType= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipeline->rasterizer.pNext = nullptr;
        pipeline->rasterizer.depthClampEnable = VK_FALSE;
        pipeline->rasterizer.rasterizerDiscardEnable = VK_FALSE;
        pipeline->rasterizer.lineWidth = 1.0f;
        pipeline->rasterizer.polygonMode = ConvertPolygonMode(pipelineDescPtr->rs.fill_mode);
        pipeline->rasterizer.cullMode = ConvertCullMode(pipelineDescPtr->rs.cull_mode);
        pipeline->rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        pipeline->rasterizer.depthBiasEnable = VK_FALSE;
        pipeline->rasterizer.depthBiasConstantFactor = 0.0f;
        pipeline->rasterizer.depthBiasClamp = 0.0f;
        pipeline->rasterizer.depthBiasSlopeFactor = 0.0f;
    }

    bool VulkanPipelineBuilder::CreatePipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &this->pipeline->pipelineLayout) != VK_SUCCESS) {
            LOG_BE_ERROR("{0} Failed to create pipeline layout.", "[VulkanPipelineBuilder]");
            return false;
        }
        return true;
    }

    VkPipelineMultisampleStateCreateInfo VulkanPipelineBuilder::CreateMultisamplingInfo()
    {
        VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
        multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleCreateInfo.pNext = nullptr;

        multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleCreateInfo.minSampleShading = 1.0f;
        multisampleCreateInfo.pSampleMask = nullptr;
        multisampleCreateInfo.alphaToCoverageEnable = this->pipelineDescPtr->bs.alpha_to_coverage_enable?VK_TRUE:VK_FALSE;
        multisampleCreateInfo.alphaToOneEnable = VK_FALSE;
        return multisampleCreateInfo;
    }

    VkPipelineColorBlendAttachmentState VulkanPipelineBuilder::CreateBlendingAttachmentInfo()
    {
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        if (pipelineDescPtr->bs.blendSettings.blend_enable == false)
        {
            colorBlendAttachment.blendEnable = VK_FALSE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        }
        else
        {
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.colorBlendOp = ConvertBlendOp(pipelineDescPtr->bs.blendSettings.blend_op); 
            colorBlendAttachment.dstColorBlendFactor = ConvertBlendingFactor(pipelineDescPtr->bs.blendSettings.dest_blend);    
            colorBlendAttachment.srcColorBlendFactor = ConvertBlendingFactor(pipelineDescPtr->bs.blendSettings.src_blend);     
            colorBlendAttachment.srcAlphaBlendFactor = ConvertBlendingFactor(pipelineDescPtr->bs.blendSettings.src_blend_alpha);     
            colorBlendAttachment.dstAlphaBlendFactor = ConvertBlendingFactor(pipelineDescPtr->bs.blendSettings.dest_blend_alpha);    
            colorBlendAttachment.alphaBlendOp = ConvertBlendOp(pipelineDescPtr->bs.blendSettings.blend_op_alpha);
        }
        return colorBlendAttachment;
    }

    VkPipelineColorBlendStateCreateInfo VulkanPipelineBuilder::CreateBlendingInfo()
    {
        auto colorBlendAttachment = CreateBlendingAttachmentInfo();
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional
        return colorBlending;
    }
    
}
