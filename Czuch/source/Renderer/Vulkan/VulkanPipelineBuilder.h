#pragma once
#include"Renderer/Graphics.h"
#include"VulkanBase.h"

namespace Czuch
{
	struct Pipeline_Vulkan;
	class VulkanDevice;

	class VulkanPipelineBuilder
	{
	public:
		VulkanPipelineBuilder(VulkanDevice* deviceObj, Pipeline_Vulkan* pipelineObj,Pipeline* pipeline);
		bool BuildPipeline(const RenderPass* renderPass);
	private:
		void CreateInputAssemblyInfo();
		void CreateRasterizationStateInfo();
		bool CreatePipelineLayout();
		VkPipelineVertexInputStateCreateInfo CreateVertexInputInfo();
		VkPipelineMultisampleStateCreateInfo CreateMultisamplingInfo();
		VkPipelineColorBlendAttachmentState CreateBlendingAttachmentInfo();
		VkPipelineColorBlendStateCreateInfo CreateBlendingInfo(VkPipelineColorBlendAttachmentState& blendInfo);
		VkPipelineDepthStencilStateCreateInfo CreateDepthStencilInfo();
	private:
		Pipeline_Vulkan* pipeline;
		VulkanDevice* device;
		const MaterialPassDesc* pipelineDescPtr;
		Pipeline* pipelineHolder;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		VkVertexInputBindingDescription bindingDescription[s_max_vertex_streams];
		VkVertexInputAttributeDescription attributeDescription[s_max_vertex_attributes];
	};

}

