#pragma once
#include"VulkanBase.h"

namespace Czuch
{
	struct Pipeline_Vulkan;
	struct Shader_Vulkan;
	struct RenderPass_Vulkan;
	struct FrameBuffer_Vulkan;
	struct DescriptorSetLayout_Vulkan;
	struct Image_Vulkan;
	struct Buffer_Vulkan;
#pragma region Converters

	inline Shader_Vulkan* Internal_To_Shader(const Shader* shader)
	{
		return (Shader_Vulkan*)shader->m_InternalResourceState.get();
	}

	inline Pipeline_Vulkan* Internal_To_Pipeline(const Pipeline* pipeline)
	{
		return (Pipeline_Vulkan*)pipeline->m_InternalResourceState.get();
	}

	inline RenderPass_Vulkan* Internal_to_RenderPass(const RenderPass* renderPass)
	{
		return (RenderPass_Vulkan*)renderPass->m_InternalResourceState.get();
	}

	inline FrameBuffer_Vulkan* Internal_to_Framebuffer(const FrameBuffer* framebuffer)
	{
		return (FrameBuffer_Vulkan*)framebuffer->m_InternalResourceState.get();
	}

	inline DescriptorSetLayout_Vulkan* Internal_to_DescriptorSetLayout(const DescriptorSetLayout* dsl)
	{
		return (DescriptorSetLayout_Vulkan*)dsl->m_InternalResourceState.get();
	}

	inline Image_Vulkan* Internal_to_Image(const Texture* tex)
	{
		return (Image_Vulkan*)tex->m_InternalResourceState.get();
	}

	inline Buffer_Vulkan* Internal_to_Buffer(const Buffer* buffer)
	{
		return (Buffer_Vulkan*)buffer->m_InternalResourceState.get();
	}

	inline VkShaderStageFlagBits ConvertShaderStage(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::VS:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderStage::PS:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		default:
			return VK_SHADER_STAGE_ALL;
		}
	}

	constexpr VkDescriptorType ConvertDescriptorType(DescriptorType descType)
	{
		switch (descType)
		{
		case SAMPLER:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		case UNIFORM_BUFFER:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case UNIFORM_BUFFER_DYNAMIC:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		case STORAGE_BUFFER:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case STORAGE_BUFFER_DYNAMIC:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		default:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
	}

	constexpr VkPrimitiveTopology ConvertTopology(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case Czuch::PrimitiveTopology::UNDEFINED:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case Czuch::PrimitiveTopology::TRIANGLELIST:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case Czuch::PrimitiveTopology::TRIANGLESTRIP:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case Czuch::PrimitiveTopology::POINTLIST:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case Czuch::PrimitiveTopology::LINELIST:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case Czuch::PrimitiveTopology::LINESTRIP:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case Czuch::PrimitiveTopology::PATCHLIST:
			return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		default:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	}

	constexpr VkPolygonMode ConvertPolygonMode(PolygonMode mode)
	{
		switch (mode)
		{
		case PolygonMode::WIREFRAME:
			return VK_POLYGON_MODE_LINE;
		case PolygonMode::SOLID:
			return VK_POLYGON_MODE_FILL;
		case PolygonMode::POINTS:
			return VK_POLYGON_MODE_POINT;
		}
		return VK_POLYGON_MODE_FILL;
	}

	constexpr VkCullModeFlags ConvertCullMode(CullMode cullMode)
	{
		switch (cullMode)
		{
		case CullMode::NONE:
			return VK_CULL_MODE_NONE;
		case CullMode::FRONT:
			return VK_CULL_MODE_FRONT_BIT;
		case CullMode::BACK:
			return VK_CULL_MODE_BACK_BIT;
		}
		return VK_CULL_MODE_NONE;
	}

	constexpr VkBlendFactor ConvertBlendingFactor(Blend blendFactor)
	{
		switch (blendFactor)
		{
		case Blend::ZERO:
			return VK_BLEND_FACTOR_ZERO;
		case Blend::ONE:
			return VK_BLEND_FACTOR_ONE;
		case Blend::SRC_COLOR:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case Blend::ONE_MINUS_SRC_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case Blend::SRC_ALPHA:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case Blend::ONE_MINUS_SRC_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case Blend::DST_ALPHA:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case Blend::ONE_MINUS_DST_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case Blend::DST_COLOR:
			return VK_BLEND_FACTOR_DST_COLOR;
		case Blend::ONE_MINUS_DST_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case Blend::CONSTANT_COLOR:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case Blend::ONE_MINUS_CONSTANT_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
			break;
		case Blend::SRC1_COLOR:
			return VK_BLEND_FACTOR_SRC1_COLOR;
		case Blend::ONE_MINUS_SRC1_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case Blend::SRC1_ALPHA:
			return VK_BLEND_FACTOR_SRC1_ALPHA;
		case Blend::ONE_MINUS_SRC1_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		default:
			return VK_BLEND_FACTOR_ZERO;
		}
	}

	constexpr VkCompareOp ConvertComparisionFunc(CompFunc CompFunc)
	{
		switch (CompFunc)
		{
		case CompFunc::NEVER:
			return VK_COMPARE_OP_NEVER;
		case CompFunc::LESS:
			return VK_COMPARE_OP_LESS;
		case CompFunc::EQUAL:
			return VK_COMPARE_OP_EQUAL;
		case CompFunc::LESS_EQUAL:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case CompFunc::GREATER:
			return VK_COMPARE_OP_GREATER;
		case CompFunc::NOT_EQUAL:
			return VK_COMPARE_OP_NOT_EQUAL;
		case CompFunc::GREATER_EQUAL:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case CompFunc::ALWAYS:
			return VK_COMPARE_OP_ALWAYS;
		default:
			return VK_COMPARE_OP_NEVER;
		}
	}

	constexpr VkBlendOp ConvertBlendOp(BlendOp value)
	{
		switch (value)
		{
		case BlendOp::ADD:
			return VK_BLEND_OP_ADD;
		case BlendOp::SUBTRACT:
			return VK_BLEND_OP_SUBTRACT;
		case BlendOp::REV_SUBTRACT:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case BlendOp::MIN:
			return VK_BLEND_OP_MIN;
		case BlendOp::MAX:
			return VK_BLEND_OP_MAX;
		default:
			return VK_BLEND_OP_ADD;
		}
	}

	constexpr VkFormat ConvertFormat(Format value)
	{
		switch (value)
		{
		case Format::UNKNOWN:
			return VK_FORMAT_UNDEFINED;
		case Format::R32G32B32A32_FLOAT:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case Format::R32G32B32A32_UINT:
			return VK_FORMAT_R32G32B32A32_UINT;
		case Format::R32G32B32A32_SINT:
			return VK_FORMAT_R32G32B32A32_SINT;
		case Format::R32G32B32_FLOAT:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case Format::R32G32B32_UINT:
			return VK_FORMAT_R32G32B32_UINT;
		case Format::R32G32B32_SINT:
			return VK_FORMAT_R32G32B32_SINT;
		case Format::R16G16B16A16_FLOAT:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case Format::R16G16B16A16_UNORM:
			return VK_FORMAT_R16G16B16A16_UNORM;
		case Format::R16G16B16A16_UINT:
			return VK_FORMAT_R16G16B16A16_UINT;
		case Format::R16G16B16A16_SNORM:
			return VK_FORMAT_R16G16B16A16_SNORM;
		case Format::R16G16B16A16_SINT:
			return VK_FORMAT_R16G16B16A16_SINT;
		case Format::R32G32_FLOAT:
			return VK_FORMAT_R32G32_SFLOAT;
		case Format::R32G32_UINT:
			return VK_FORMAT_R32G32_UINT;
		case Format::R32G32_SINT:
			return VK_FORMAT_R32G32_SINT;
		case Format::D32_FLOAT_S8X24_UINT:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
		case Format::R10G10B10A2_UNORM:
			return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		case Format::R10G10B10A2_UINT:
			return VK_FORMAT_A2B10G10R10_UINT_PACK32;
		case Format::R11G11B10_FLOAT:
			return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		case Format::R8G8B8A8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case Format::R8G8B8A8_UNORM_SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case Format::R8G8B8A8_UINT:
			return VK_FORMAT_R8G8B8A8_UINT;
		case Format::R8G8B8A8_SNORM:
			return VK_FORMAT_R8G8B8A8_SNORM;
		case Format::R8G8B8A8_SINT:
			return VK_FORMAT_R8G8B8A8_SINT;
		case Format::R16G16_FLOAT:
			return VK_FORMAT_R16G16_SFLOAT;
		case Format::R16G16_UNORM:
			return VK_FORMAT_R16G16_UNORM;
		case Format::R16G16_UINT:
			return VK_FORMAT_R16G16_UINT;
		case Format::R16G16_SNORM:
			return VK_FORMAT_R16G16_SNORM;
		case Format::R16G16_SINT:
			return VK_FORMAT_R16G16_SINT;
		case Format::D32_FLOAT:
			return VK_FORMAT_D32_SFLOAT;
		case Format::R32_FLOAT:
			return VK_FORMAT_R32_SFLOAT;
		case Format::R32_UINT:
			return VK_FORMAT_R32_UINT;
		case Format::R32_SINT:
			return VK_FORMAT_R32_SINT;
		case Format::D24_UNORM_S8_UINT:
			return VK_FORMAT_D24_UNORM_S8_UINT;
		case Format::R9G9B9E5_SHAREDEXP:
			return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
		case Format::R8G8_UNORM:
			return VK_FORMAT_R8G8_UNORM;
		case Format::R8G8_UINT:
			return VK_FORMAT_R8G8_UINT;
		case Format::R8G8_SNORM:
			return VK_FORMAT_R8G8_SNORM;
		case Format::R8G8_SINT:
			return VK_FORMAT_R8G8_SINT;
		case Format::R16_FLOAT:
			return VK_FORMAT_R16_SFLOAT;
		case Format::D16_UNORM:
			return VK_FORMAT_D16_UNORM;
		case Format::R16_UNORM:
			return VK_FORMAT_R16_UNORM;
		case Format::R16_UINT:
			return VK_FORMAT_R16_UINT;
		case Format::R16_SNORM:
			return VK_FORMAT_R16_SNORM;
		case Format::R16_SINT:
			return VK_FORMAT_R16_SINT;
		case Format::R8_UNORM:
			return VK_FORMAT_R8_UNORM;
		case Format::R8_UINT:
			return VK_FORMAT_R8_UINT;
		case Format::R8_SNORM:
			return VK_FORMAT_R8_SNORM;
		case Format::R8_SINT:
			return VK_FORMAT_R8_SINT;
		case Format::BC1_UNORM:
			return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		case Format::BC1_UNORM_SRGB:
			return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
		case Format::BC2_UNORM:
			return VK_FORMAT_BC2_UNORM_BLOCK;
		case Format::BC2_UNORM_SRGB:
			return VK_FORMAT_BC2_SRGB_BLOCK;
		case Format::BC3_UNORM:
			return VK_FORMAT_BC3_UNORM_BLOCK;
		case Format::BC3_UNORM_SRGB:
			return VK_FORMAT_BC3_SRGB_BLOCK;
		case Format::BC4_UNORM:
			return VK_FORMAT_BC4_UNORM_BLOCK;
		case Format::BC4_SNORM:
			return VK_FORMAT_BC4_SNORM_BLOCK;
		case Format::BC5_UNORM:
			return VK_FORMAT_BC5_UNORM_BLOCK;
		case Format::BC5_SNORM:
			return VK_FORMAT_BC5_SNORM_BLOCK;
		case Format::B8G8R8A8_UNORM:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case Format::B8G8R8A8_UNORM_SRGB:
			return VK_FORMAT_B8G8R8A8_SRGB;
		case Format::BC6H_UF16:
			return VK_FORMAT_BC6H_UFLOAT_BLOCK;
		case Format::BC6H_SF16:
			return VK_FORMAT_BC6H_SFLOAT_BLOCK;
		case Format::BC7_UNORM:
			return VK_FORMAT_BC7_UNORM_BLOCK;
		case Format::BC7_UNORM_SRGB:
			return VK_FORMAT_BC7_SRGB_BLOCK;
		case Format::NV12:
			return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
		}
		return VK_FORMAT_UNDEFINED;
	}


#pragma endregion

	struct VulkanDeviceRef
	{
		VkDevice device;
	};

	struct FrameBuffer_Vulkan : public VulkanDeviceRef
	{
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		VkFramebufferCreateInfo createInfo;
		~FrameBuffer_Vulkan()
		{
			if (framebuffer)
			{
				vkDestroyFramebuffer(device, framebuffer, nullptr);
			}
		}
	};

	struct Shader_Vulkan : public VulkanDeviceRef
	{
		VkShaderModule shaderModule = VK_NULL_HANDLE;
		VkPipelineShaderStageCreateInfo shaderStageInfo;

		~Shader_Vulkan()
		{
			if (shaderModule)
			{
				vkDestroyShaderModule(device, shaderModule, nullptr);
			}
		}
	};

	struct RenderPass_Vulkan : public VulkanDeviceRef
	{
		VkRenderPass renderPass = VK_NULL_HANDLE;

		~RenderPass_Vulkan()
		{
			if (renderPass)
			{
				vkDestroyRenderPass(device, renderPass, nullptr);
			}
		}
	};

	struct Pipeline_Vulkan : public VulkanDeviceRef
	{
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		VkGraphicsPipelineCreateInfo pipelineInfo;
		VkPipelineShaderStageCreateInfo shaderStages[static_cast<size_t>(ShaderStage::Count)];
		VkPipelineInputAssemblyStateCreateInfo inputAssembly;
		VkPipelineRasterizationStateCreateInfo rasterizer;
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineViewportStateCreateInfo viewportState;
		VkPipelineDepthStencilStateCreateInfo depthstencil;

		~Pipeline_Vulkan()
		{
			if (pipeline)
			{
				vkDestroyPipeline(device, pipeline, nullptr);
				vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			}
		}
	};

	struct Image_Vulkan : public VulkanDeviceRef
	{
		VkImage image;
		VkImageView imageView;
		~Image_Vulkan()
		{
			if (image)
			{
				vkDestroyImageView(device, imageView, nullptr);
				vkDestroyImage(device, image, nullptr);
			}
		}
	};

	struct Framebuffer_Vulkan : public VulkanDeviceRef
	{
		VkFramebuffer framebuffer;
		~Framebuffer_Vulkan()
		{
			if (framebuffer != nullptr)
			{
				vkDestroyFramebuffer(device, framebuffer, nullptr);
			}
		}
	};

	struct Buffer_Vulkan : public VulkanDeviceRef
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDeviceSize size;
		VkBufferUsageFlags flags;
		U32 offset = 0;
		bool ready = true;

		~Buffer_Vulkan()
		{
			if (buffer)
			{
				vkDestroyBuffer(device, buffer, nullptr);
				vkFreeMemory(device, memory, nullptr);
			}
		}
	};

	struct DescriptorSetLayout_Vulkan : public VulkanDeviceRef
	{
		VkDescriptorSetLayout layout;
		~DescriptorSetLayout_Vulkan()
		{
			if (layout)
			{
				vkDestroyDescriptorSetLayout(device, layout, nullptr);
			}
		}
	};
}