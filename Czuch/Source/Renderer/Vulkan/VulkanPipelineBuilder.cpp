#include "czpch.h"
#include "VulkanPipelineBuilder.h"
#include"./Subsystems/Logging.h"
#include"VulkanCore.h"
#include"VulkanDevice.h"

namespace Czuch
{
    VulkanPipelineBuilder::VulkanPipelineBuilder(VulkanDevice* deviceObj, Pipeline_Vulkan* pipelineObj,Pipeline* pipelineHolder_):pipeline(pipelineObj),device(deviceObj),pipelineDescPtr(&pipelineHolder_->GetDesc())
        ,pipelineHolder(pipelineHolder_)
    {
    }

     bool VulkanPipelineBuilder::BuildPipeline(const RenderPass* renderPass)
    {
		 bool dynamicRendering = device->HasDynamicRenderingEnabled();
        pipeline->device = device->GetNativeDevice();
        VkRenderPass vulkanRenderPass = (renderPass != nullptr&&!dynamicRendering) ? Internal_to_RenderPass(renderPass)->renderPass : VK_NULL_HANDLE;

        if (HANDLE_IS_VALID(pipelineDescPtr->vs))
        {
            Shader_Vulkan* vulkan_shader_vs = Internal_To_Shader(device->AccessShader(pipelineHolder->vs));
            shaderStages.push_back(vulkan_shader_vs->shaderStageInfo);
        }

        if (HANDLE_IS_VALID(pipelineDescPtr->ps))
        {
            Shader_Vulkan* vulkan_shader_ps = Internal_To_Shader(device->AccessShader(pipelineHolder->ps));
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

        VkPipelineVertexInputStateCreateInfo vertexInputInfo= CreateVertexInputInfo();
        VkPipelineMultisampleStateCreateInfo multiSamplingInfo = CreateMultisamplingInfo();
        CreateInputAssemblyInfo();
        CreateRasterizationStateInfo();
        VkPipelineColorBlendStateCreateInfo blendingInfo = CreateBlendingInfo();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;
        viewportState.pNext = nullptr;

        auto depthStencilInfo = CreateDepthStencilInfo();

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
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipeline->pipelineLayout;
        pipelineInfo.renderPass = vulkanRenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.pDepthStencilState = &depthStencilInfo;

        VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info{};
        if (dynamicRendering)
        {
            pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
			if (renderPass != nullptr)
			{
                VkFormat colorAttachmentFormats[k_max_image_outputs];

				for (int a = 0; a < renderPass->desc.attachmentsCount; ++a)
				{
                    colorAttachmentFormats[a] = ConvertFormat(renderPass->desc.colorAttachments[a].format);
				}
                auto depthStencilFormat = ConvertFormat(renderPass->desc.depthStencilFormat);
                pipeline_rendering_create_info.colorAttachmentCount = renderPass->desc.attachmentsCount;
                pipeline_rendering_create_info.pColorAttachmentFormats = colorAttachmentFormats;
                pipeline_rendering_create_info.depthAttachmentFormat = depthStencilFormat;
                pipeline_rendering_create_info.stencilAttachmentFormat = IsDepthFormatWithStencil(renderPass->desc.depthStencilFormat)?depthStencilFormat:VK_FORMAT_UNDEFINED;
			}
			else
			{
                pipeline_rendering_create_info.colorAttachmentCount = 1;
                pipeline_rendering_create_info.pColorAttachmentFormats = device->GetSwapchainFormat();
                pipeline_rendering_create_info.depthAttachmentFormat = device->GetDepthFormat();
                pipeline_rendering_create_info.stencilAttachmentFormat = device->GetDepthFormat();
			}
            pipelineInfo.pNext = &pipeline_rendering_create_info;
		}


        if (vkCreateGraphicsPipelines(
            device->GetNativeDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline->pipeline) != VK_SUCCESS) {
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
        const InputVertexLayout& vertLayout = pipelineDescPtr->il;

        for (int a = 0; a < vertLayout.vertexStreamsCount; ++a)
        {
            bindingDescription[a].binding = vertLayout.streams[a].binding;
            bindingDescription[a].stride = vertLayout.streams[a].stride;
            bindingDescription[a].inputRate = vertLayout.streams[a].input_rate == InputClassification::PER_VERTEX_DATA ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
        }
        
        vertexInputInfo.vertexBindingDescriptionCount = vertLayout.vertexStreamsCount;

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
        pipeline->inputAssembly.pNext = nullptr;
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
        VkDescriptorSetLayout layouts[k_max_descriptor_set_layouts];

        for (int a = 0; a < pipelineDescPtr->layoutsCount; ++a)
        {
            layouts[a] = Internal_to_DescriptorSetLayout(device->AccessDescriptorSetLayout(pipelineHolder->layouts[a]))->layout;
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = pipelineDescPtr->layoutsCount; // Optional
        pipelineLayoutInfo.pSetLayouts = layouts; // Optional

        VkPushConstantRange pushConstant{};
        pushConstant.offset = 0;
        pushConstant.size = sizeof(glm::mat4x4);
        pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT|VK_SHADER_STAGE_FRAGMENT_BIT|VK_SHADER_STAGE_VERTEX_BIT;

        pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
        pipelineLayoutInfo.pushConstantRangeCount = 1;

        if (vkCreatePipelineLayout(device->GetNativeDevice(), &pipelineLayoutInfo, nullptr, &this->pipeline->pipelineLayout) != VK_SUCCESS) {
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
        if (pipelineDescPtr->bs.blendSettings.blendEnable == false)
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
            colorBlendAttachment.colorBlendOp = ConvertBlendOp(pipelineDescPtr->bs.blendSettings.colorBlendOp); 
            colorBlendAttachment.dstColorBlendFactor = ConvertBlendingFactor(pipelineDescPtr->bs.blendSettings.dstColorBlendFactor);    
            colorBlendAttachment.srcColorBlendFactor = ConvertBlendingFactor(pipelineDescPtr->bs.blendSettings.srcColorBlendFactor);     
            colorBlendAttachment.srcAlphaBlendFactor = ConvertBlendingFactor(pipelineDescPtr->bs.blendSettings.srcAlphaBlendFactor);     
            colorBlendAttachment.dstAlphaBlendFactor = ConvertBlendingFactor(pipelineDescPtr->bs.blendSettings.dstAlphaBlendFactor);    
            colorBlendAttachment.alphaBlendOp = ConvertBlendOp(pipelineDescPtr->bs.blendSettings.alphaBlendOp);
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

    VkPipelineDepthStencilStateCreateInfo VulkanPipelineBuilder::CreateDepthStencilInfo()
    {
        const DepthStencilState& dss =pipelineDescPtr->dss;
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = dss.depth_enable;
        depthStencil.depthWriteEnable = dss.depth_write_enable;
        depthStencil.depthCompareOp = ConvertComparisionFunc(dss.depth_func);
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;

        depthStencil.stencilTestEnable = dss.stencil_enable;
        depthStencil.front = {};
        depthStencil.back = {};
        return depthStencil;
    }
    
}
