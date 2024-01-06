#pragma once
#include"Renderer/Graphics.h"
#include"VulkanBase.h"

namespace Czuch
{
	struct Pipeline_Vulkan;

	class VulkanPipelineBuilder
	{
	public:
		VulkanPipelineBuilder(const VkDevice deviceObj, Pipeline_Vulkan* pipelineObj,const PipelineStateDesc* pipelineDesc);
		bool BuildPipeline(const VkRenderPass renderPass);
	private:
		void CreateInputAssemblyInfo();
		void CreateRasterizationStateInfo();
		bool CreatePipelineLayout();
		VkPipelineVertexInputStateCreateInfo CreateVertexInputInfo();
		VkPipelineMultisampleStateCreateInfo CreateMultisamplingInfo();
		VkPipelineColorBlendAttachmentState CreateBlendingAttachmentInfo();
		VkPipelineColorBlendStateCreateInfo CreateBlendingInfo();
	private:
		Pipeline_Vulkan* pipeline;
		VkDevice device;
		const PipelineStateDesc* pipelineDescPtr;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	};

}

