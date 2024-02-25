#pragma once
#include"./Core/Common.h"
#include"VulkanBase.h"
#include"./Renderer/Graphics.h"
#include"./Subsystems/Logging.h"

namespace Czuch
{
	struct Pipeline_Vulkan;
	struct Shader_Vulkan;
	struct RenderPass_Vulkan;
	struct FrameBuffer_Vulkan;
	struct DescriptorSetLayout_Vulkan;
	struct Texture_Vulkan;
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

	inline Texture_Vulkan* Internal_to_Texture(const Texture* tex)
	{
		return (Texture_Vulkan*)tex->m_InternalResourceState.get();
	}

	inline Buffer_Vulkan* Internal_to_Buffer(const Buffer* buffer)
	{
		return (Buffer_Vulkan*)buffer->m_InternalResourceState.get();
	}

	inline VkShaderStageFlags ConvertShaderStage(U32 stage)
	{
		VkShaderStageFlags flags=0;

		if ((stage&(U32)ShaderStage::VS) == (U32)ShaderStage::VS)
		{
			flags|= VK_SHADER_STAGE_VERTEX_BIT;
		}

		if ((stage & (U32)ShaderStage::PS) == (U32)ShaderStage::PS)
		{
			flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		if ((stage & (U32)ShaderStage::ALL) == (U32)ShaderStage::ALL)
		{
			flags |= VK_SHADER_STAGE_ALL;
		}

	
		return flags;
	}

	constexpr VkImageType ConvertImageType(TextureDesc::Type inTexType)
	{
		if (inTexType == TextureDesc::Type::TEXTURE_1D)
		{
			return VK_IMAGE_TYPE_1D;
		}

		if (inTexType == TextureDesc::Type::TEXTURE_2D)
		{
			return VK_IMAGE_TYPE_2D;
		}

		if (inTexType == TextureDesc::Type::TEXTURE_3D)
		{
			return VK_IMAGE_TYPE_3D;
		}

		if (inTexType == TextureDesc::Type::TEXTURE_CUBE)
		{
			return VK_IMAGE_TYPE_3D;
		}

		return VK_IMAGE_TYPE_2D;
	}

	constexpr VkShaderStageFlagBits ConvertShaderStageBits(ShaderStage stage)
	{
		if (stage == ShaderStage::VS)
		{
			return VK_SHADER_STAGE_VERTEX_BIT;
		}
		else if (stage == ShaderStage::PS)
		{
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		else if (stage == ShaderStage::ALL)
		{
			return VK_SHADER_STAGE_ALL;
		}

		return VK_SHADER_STAGE_VERTEX_BIT;
	}

	constexpr VkPipelineBindPoint ConvertBindPoint(BindPoint bp)
	{
		switch (bp)
		{
		case BindPoint::BIND_POINT_GRAPHICS:
			return VK_PIPELINE_BIND_POINT_GRAPHICS;
		case BindPoint::BIND_POINT_COMPUTE:
			return VK_PIPELINE_BIND_POINT_COMPUTE;
		case BindPoint::_BIND_POINT_RAY_TRACING:
			return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
		}
		return VK_PIPELINE_BIND_POINT_GRAPHICS;
	}

	constexpr VmaMemoryUsage ConvertMemoryUsage(Usage memUsage)
	{
		switch (memUsage)
		{
		case Usage::DEFAULT:
			return VMA_MEMORY_USAGE_AUTO;
		case Usage::MEMORY_USAGE_CPU_ONLY:
			return VMA_MEMORY_USAGE_CPU_ONLY;
		case Usage::MEMORY_USAGE_CPU_TO_GPU:
			return VMA_MEMORY_USAGE_CPU_TO_GPU;
		case Usage::MEMORY_USAGE_GPU_ONLY:
			return VMA_MEMORY_USAGE_GPU_ONLY;
		default:
			return VMA_MEMORY_USAGE_AUTO;
		}
	}

	constexpr VkFilter ConvertFilterType(TextureFilter type)
	{
		if (type == TextureFilter::LINEAR)
		{
			return VK_FILTER_LINEAR;
		}
		else return VK_FILTER_NEAREST;
	}

	constexpr VkSamplerAddressMode ConvertAddressMode(TextureAddressMode mode)
	{
		if (mode == TextureAddressMode::WRAP)
		{
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}
		if (mode == TextureAddressMode::MIRROR)
		{
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		}
		if (mode == TextureAddressMode::CLAMP)
		{
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		}
		if (mode == TextureAddressMode::BORDER)
		{
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		}

		return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
	}

	constexpr VkDescriptorType ConvertDescriptorType(DescriptorType descType)
	{
		switch (descType)
		{
		case SAMPLER:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
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

	struct BufferInternalSettings
	{
		//out
		VkBuffer outBuffer;
		VmaAllocation outMemAlloc;
		//in
		VkDeviceSize inSize;
		VkBufferUsageFlags inFlags;
		Usage inUsage;
		bool inStagingBuffer;
		bool inCreateMapped;
	};

	struct VulkanDeviceRef
	{
		VkDevice device;
		VmaAllocator allocator;
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

	struct Texture_Vulkan : public VulkanDeviceRef
	{
		VkImage image;
		VkImageView imageView;
		VkSampler sampler;
		VmaAllocation allocation;
		~Texture_Vulkan()
		{
			if (image)
			{
				vkDestroySampler(device, sampler, nullptr);
				vkDestroyImageView(device, imageView, nullptr);
				vmaDestroyImage(allocator, image, allocation);
				image = VK_NULL_HANDLE;
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
		VkBuffer buffer= VK_NULL_HANDLE;
		VkDeviceSize size;
		VkBufferUsageFlags flags;
		U32 offset = 0;
		bool ready = true;
		bool mapped = false;
		VmaAllocation allocation;

		void* GetMappedData();

		~Buffer_Vulkan()
		{
			if (buffer!=VK_NULL_HANDLE)
			{
				vmaDestroyBuffer(allocator, buffer, allocation);
				buffer = VK_NULL_HANDLE;
			}
		}
	};

	struct DescriptorSetLayout_Vulkan : public VulkanDeviceRef
	{
		VkDescriptorSetLayout layout;
		~DescriptorSetLayout_Vulkan()
		{
			
		}
	};

	struct DescriptorSet : public VulkanDeviceRef
	{
		VkDescriptorSet descriptorSet;
		DescriptorSetDesc desc;
		DescriptorSetLayout* descriptorLayout;
	};
}