#pragma once
#include"./Core/Common.h"
#include"VulkanBase.h"
#include"./Renderer/Graphics.h"
#include"./Subsystems/Logging.h"
#include"./Renderer/Vulkan/DescriptorAllocator.h"

namespace Czuch
{
	struct Pipeline_Vulkan;
	struct Shader_Vulkan;
	struct RenderPass_Vulkan;
	struct FrameBuffer_Vulkan;
	struct DescriptorSetLayout_Vulkan;
	struct Texture_Vulkan;
	struct Buffer_Vulkan;


	constexpr const char* VkResultToString(VkResult result)
	{
		switch (result)
		{
		case VK_SUCCESS: return "VK_SUCCESS";
		case VK_NOT_READY: return "VK_NOT_READY";
		case VK_TIMEOUT: return "VK_TIMEOUT";
		case VK_EVENT_SET: return "VK_EVENT_SET";
		case VK_EVENT_RESET: return "VK_EVENT_RESET";
		case VK_INCOMPLETE: return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
		case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
		case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
		default: return "UNKNOWN_VK_RESULT";
		}
	}




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
		VkShaderStageFlags flags = 0;

		if ((stage & (U32)ShaderStage::VS) == (U32)ShaderStage::VS)
		{
			flags |= VK_SHADER_STAGE_VERTEX_BIT;
		}

		if ((stage & (U32)ShaderStage::PS) == (U32)ShaderStage::PS)
		{
			flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		if ((stage & (U32)ShaderStage::CS) == (U32)ShaderStage::CS)
		{
			flags |= VK_SHADER_STAGE_COMPUTE_BIT;
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
		case BindPoint::BIND_POINT_RAY_TRACING:
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
		if (mode == TextureAddressMode::EDGE_CLAMP)
		{
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		}

		return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
	}

	constexpr VkDescriptorType ConvertDescriptorType(DescriptorType descType)
	{
		switch (descType)
		{
		case DescriptorType::SAMPLER:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		case DescriptorType::UNIFORM_BUFFER:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case DescriptorType::UNIFORM_BUFFER_DYNAMIC:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		case DescriptorType::STORAGE_BUFFER:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case DescriptorType::STORAGE_BUFFER_DYNAMIC:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		case DescriptorType::COMBINED_IMAGE_SAMPLER:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case DescriptorType::SAMPLED_IMAGE:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
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

	constexpr VkCommandBufferUsageFlags ConvertCommandBufferUsageFlags(CommandBufferUseFlag flags)
	{
		VkCommandBufferUsageFlags outFlags = 0;

		if ((flags & (U32)CommandBufferUseFlag::ONE_TIME_SUBMIT) == CommandBufferUseFlag::ONE_TIME_SUBMIT)
		{
			outFlags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}

		if ((flags & (U32)CommandBufferUseFlag::RENDER_PASS_CONTINUE) == CommandBufferUseFlag::RENDER_PASS_CONTINUE)
		{
			outFlags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		}

		if ((flags & (U32)CommandBufferUseFlag::SIMULTANEOUS_USE) == CommandBufferUseFlag::SIMULTANEOUS_USE)
		{
			outFlags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		}

		return outFlags;
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

	constexpr Format ConvertVkFormat(VkFormat value)
	{
		switch (value)
		{
		case VK_FORMAT_UNDEFINED:
			return Format::UNKNOWN;
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return Format::R32G32B32A32_FLOAT;
		case VK_FORMAT_R32G32B32A32_UINT:
			return Format::R32G32B32A32_UINT;
		case VK_FORMAT_R32G32B32A32_SINT:
			return Format::R32G32B32A32_SINT;
		case VK_FORMAT_R32G32B32_SFLOAT:
			return Format::R32G32B32_FLOAT;
		case VK_FORMAT_R32G32B32_UINT:
			return Format::R32G32B32_UINT;
		case VK_FORMAT_R32G32B32_SINT:
			return Format::R32G32B32_SINT;
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			return Format::R16G16B16A16_FLOAT;
		case VK_FORMAT_R16G16B16A16_UNORM:
			return Format::R16G16B16A16_UNORM;
		case VK_FORMAT_R16G16B16A16_UINT:
			return Format::R16G16B16A16_UINT;
		case VK_FORMAT_R16G16B16A16_SNORM:
			return Format::R16G16B16A16_SNORM;
		case VK_FORMAT_R16G16B16A16_SINT:
			return Format::R16G16B16A16_SINT;
		case VK_FORMAT_R32G32_SFLOAT:
			return Format::R32G32_FLOAT;
		case VK_FORMAT_R32G32_UINT:
			return Format::R32G32_UINT;
		case VK_FORMAT_R32G32_SINT:
			return Format::R32G32_SINT;
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return Format::D32_FLOAT_S8X24_UINT;
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
			return Format::R10G10B10A2_UNORM;
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:
			return Format::R10G10B10A2_UINT;
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
			return Format::R11G11B10_FLOAT;
		case VK_FORMAT_R8G8B8A8_UNORM:
			return Format::R8G8B8A8_UNORM;
		case VK_FORMAT_R8G8B8A8_SRGB:
			return Format::R8G8B8A8_UNORM_SRGB;
		case VK_FORMAT_R8G8B8A8_UINT:
			return Format::R8G8B8A8_UINT;
		case VK_FORMAT_R8G8B8A8_SNORM:
			return Format::R8G8B8A8_SNORM;
		case VK_FORMAT_R8G8B8A8_SINT:
			return Format::R8G8B8A8_SINT;
		case VK_FORMAT_R16G16_SFLOAT:
			return Format::R16G16_FLOAT;
		case VK_FORMAT_R16G16_UNORM:
			return Format::R16G16_UNORM;
		case VK_FORMAT_R16G16_UINT:
			return Format::R16G16_UINT;
		case VK_FORMAT_R16G16_SNORM:
			return Format::R16G16_SNORM;
		case VK_FORMAT_R16G16_SINT:
			return Format::R16G16_SINT;
		case VK_FORMAT_D32_SFLOAT:
			return Format::D32_FLOAT;
		case VK_FORMAT_R32_SFLOAT:
			return Format::R32_FLOAT;
		case VK_FORMAT_R32_UINT:
			return Format::R32_UINT;
		case VK_FORMAT_R32_SINT:
			return Format::R32_SINT;
		case VK_FORMAT_D24_UNORM_S8_UINT:
			return Format::D24_UNORM_S8_UINT;
		case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
			return Format::R9G9B9E5_SHAREDEXP;
		case VK_FORMAT_R8G8_UNORM:
			return Format::R8G8_UNORM;
		case VK_FORMAT_R8G8_UINT:
			return Format::R8G8_UINT;
		case VK_FORMAT_R8G8_SNORM:
			return Format::R8G8_SNORM;
		case VK_FORMAT_R8G8_SINT:
			return Format::R8G8_SINT;
		case VK_FORMAT_R16_SFLOAT:
			return Format::R16_FLOAT;
		case VK_FORMAT_D16_UNORM:
			return Format::D16_UNORM;
		case VK_FORMAT_R16_UNORM:
			return Format::R16_UNORM;
		case VK_FORMAT_R16_UINT:
			return Format::R16_UINT;
		case VK_FORMAT_R16_SNORM:
			return Format::R16_SNORM;
		case VK_FORMAT_R16_SINT:
			return Format::R16_SINT;
		case VK_FORMAT_R8_UNORM:
			return Format::R8_UNORM;
		case VK_FORMAT_R8_UINT:
			return Format::R8_UINT;
		case VK_FORMAT_R8_SNORM:
			return Format::R8_SNORM;
		case VK_FORMAT_R8_SINT:
			return Format::R8_SINT;
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
			return Format::BC1_UNORM;
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
			return Format::BC1_UNORM_SRGB;
		case VK_FORMAT_BC2_UNORM_BLOCK:
			return Format::BC2_UNORM;
		case VK_FORMAT_BC2_SRGB_BLOCK:
			return Format::BC2_UNORM_SRGB;
		case VK_FORMAT_BC3_UNORM_BLOCK:
			return Format::BC3_UNORM;
		case VK_FORMAT_BC3_SRGB_BLOCK:
			return Format::BC3_UNORM_SRGB;
		case VK_FORMAT_BC4_UNORM_BLOCK:
			return Format::BC4_UNORM;
		case VK_FORMAT_BC4_SNORM_BLOCK:
			return Format::BC4_SNORM;
		case VK_FORMAT_BC5_UNORM_BLOCK:
			return Format::BC5_UNORM;
		case VK_FORMAT_BC5_SNORM_BLOCK:
			return Format::BC5_SNORM;
		case VK_FORMAT_B8G8R8A8_UNORM:
			return Format::B8G8R8A8_UNORM;
		case VK_FORMAT_B8G8R8A8_SRGB:
			return Format::B8G8R8A8_UNORM_SRGB;
		case VK_FORMAT_BC6H_UFLOAT_BLOCK:
			return Format::BC6H_UF16;
		case VK_FORMAT_BC6H_SFLOAT_BLOCK:
			return Format::BC6H_SF16;
		case VK_FORMAT_BC7_UNORM_BLOCK:
			return Format::BC7_UNORM;
		case VK_FORMAT_BC7_SRGB_BLOCK:
			return Format::BC7_UNORM_SRGB;
		default:
			return Format::UNKNOWN;
		}
	}

	constexpr VkSampleCountFlagBits ConvertSamplesCount(U32 count)
	{
		switch (count)
		{
		case 1:
			return VK_SAMPLE_COUNT_1_BIT;
		case 2:
			return VK_SAMPLE_COUNT_2_BIT;
		case 4:
			return VK_SAMPLE_COUNT_4_BIT;
		case 8:
			return VK_SAMPLE_COUNT_8_BIT;
		case 16:
			return VK_SAMPLE_COUNT_16_BIT;
		case 32:
			return VK_SAMPLE_COUNT_32_BIT;
		case 64:
			return VK_SAMPLE_COUNT_64_BIT;
		default:
			return VK_SAMPLE_COUNT_1_BIT;
		}

	}

	constexpr VkImageUsageFlags ConvertImageUsageFlags(U32 flags)
	{
		VkImageUsageFlags outFlags = 0;

		if ((flags & (U32)ImageUsageFlag::TRANSFER_SRC) == (U32)ImageUsageFlag::TRANSFER_SRC)
		{
			outFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		if ((flags & (U32)ImageUsageFlag::TRANSFER_DST) == (U32)ImageUsageFlag::TRANSFER_DST)
		{
			outFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		if ((flags & (U32)ImageUsageFlag::SAMPLED) == (U32)ImageUsageFlag::SAMPLED)
		{
			outFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}

		if ((flags & (U32)ImageUsageFlag::STORAGE) == (U32)ImageUsageFlag::STORAGE)
		{
			outFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		}

		if ((flags & (U32)ImageUsageFlag::COLOR_ATTACHMENT) == (U32)ImageUsageFlag::COLOR_ATTACHMENT)
		{
			outFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}

		if ((flags & (U32)ImageUsageFlag::DEPTH_STENCIL_ATTACHMENT) == (U32)ImageUsageFlag::DEPTH_STENCIL_ATTACHMENT)
		{
			outFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}

		return outFlags;
	}

	constexpr VkImageAspectFlags ConvertImageAspect(U32 aspectsFlags)
	{
		VkImageAspectFlags outFlags=0;

		if ((aspectsFlags & (U32)ImageAspectFlag::COLOR) == (U32)ImageAspectFlag::COLOR)
		{
			outFlags |= VK_IMAGE_ASPECT_COLOR_BIT;
		}

		if ((aspectsFlags & (U32)ImageAspectFlag::DEPTH) == (U32)ImageAspectFlag::DEPTH)
		{
			outFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		else if ((aspectsFlags & (U32)ImageAspectFlag::STENCIL) == (U32)ImageAspectFlag::STENCIL)
		{
			outFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		if ((aspectsFlags & (U32)ImageAspectFlag::METADATA) == (U32)ImageAspectFlag::METADATA)
		{
			outFlags |= VK_IMAGE_ASPECT_METADATA_BIT;
		}

		if ((aspectsFlags & (U32)ImageAspectFlag::PLANE_0) == (U32)ImageAspectFlag::PLANE_0)
		{
			outFlags |= VK_IMAGE_ASPECT_PLANE_0_BIT;
		}

		if ((aspectsFlags & (U32)ImageAspectFlag::PLANE_1) == (U32)ImageAspectFlag::PLANE_1)
		{
			outFlags |= VK_IMAGE_ASPECT_PLANE_1_BIT;
		}

		if ((aspectsFlags & (U32)ImageAspectFlag::PLANE_2) == (U32)ImageAspectFlag::PLANE_2)
		{
			outFlags |= VK_IMAGE_ASPECT_PLANE_2_BIT;
		}

		if ((aspectsFlags & (U32)ImageAspectFlag::MEMORY_PLANE_0) == (U32)ImageAspectFlag::MEMORY_PLANE_0)
		{
			outFlags |= VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT;
		}

		if ((aspectsFlags & (U32)ImageAspectFlag::MEMORY_PLANE_1) == (U32)ImageAspectFlag::MEMORY_PLANE_1)
		{
			outFlags |= VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT;
		}

		if ((aspectsFlags & (U32)ImageAspectFlag::MEMORY_PLANE_2) == (U32)ImageAspectFlag::MEMORY_PLANE_2)
		{
			outFlags |= VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT;
		}
		return outFlags;
	}

	constexpr CzuchStr VulkanImageLayoutToString(VkImageLayout layout)
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			return CzuchStr("VK_IMAGE_LAYOUT_UNDEFINED");
		case VK_IMAGE_LAYOUT_GENERAL:
			return CzuchStr("VK_IMAGE_LAYOUT_GENERAL");
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return CzuchStr("VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL");
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return CzuchStr("VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL");
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			return CzuchStr("VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL");
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return CzuchStr("VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL");
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			return CzuchStr("VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL");
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			return CzuchStr("VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL");
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			return CzuchStr("VK_IMAGE_LAYOUT_PREINITIALIZED");
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
			return CzuchStr("VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL");
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
			return CzuchStr("VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL");
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			return CzuchStr("VK_IMAGE_LAYOUT_PRESENT_SRC_KHR");
		case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
			return CzuchStr("VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR");
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
			return CzuchStr("VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL");
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:	
			return CzuchStr("VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL");
		case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
			return CzuchStr("VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL");
		case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
			return CzuchStr("VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL");
		default: return CzuchStr("VK_IMAGE_LAYOUT_UNDEFINED");
		}
	}

	constexpr VkImageLayout ConvertImageLayout(ImageLayout layout)
	{
		switch (layout)
		{
		case ImageLayout::UNDEFINED:
			return VK_IMAGE_LAYOUT_UNDEFINED;
		case ImageLayout::GENERAL:
			return VK_IMAGE_LAYOUT_GENERAL;
		case ImageLayout::COLOR_ATTACHMENT_OPTIMAL:
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		case ImageLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		case ImageLayout::SHADER_READ_ONLY_OPTIMAL:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case ImageLayout::TRANSFER_SRC_OPTIMAL:
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case ImageLayout::TRANSFER_DST_OPTIMAL:
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case ImageLayout::PREINITIALIZED:
			return VK_IMAGE_LAYOUT_PREINITIALIZED;
		case ImageLayout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
		case ImageLayout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
			return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
		case ImageLayout::PRESENT_SRC_KHR:
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		case ImageLayout::SHARED_PRESENT_KHR:
			return VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR;
		case ImageLayout::DEPTH_ATTACHMENT_OPTIMAL:
			return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		case ImageLayout::DEPTH_READ_ONLY_OPTIMAL:
			return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
		case ImageLayout::STENCIL_ATTACHMENT_OPTIMAL:
			return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
		case ImageLayout::STENCIL_READ_ONLY_OPTIMAL:
			return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
		}
	}

#pragma endregion

	struct VulkanRenderPassDesc : public RenderPassDesc
	{
		virtual ~VulkanRenderPassDesc() = default;
		bool mainRenderPass = false; 
		VkRenderPassCreateInfo renderPassInfo;
	};


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
			Release();
		}

		void Release()
		{
			if (framebuffer)
			{
				vkDestroyFramebuffer(device, framebuffer, nullptr);
				framebuffer = VK_NULL_HANDLE;
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
			Release();
		}

		void Release()
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
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceSize size;
		VkBufferUsageFlags flags;
		U32 offset = 0;
		bool ready = true;
		bool persistentMapping = false;
		VmaAllocation allocation;

		void* GetMappedData();

		~Buffer_Vulkan()
		{
			if (buffer != VK_NULL_HANDLE)
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
		ShaderParamsSet* desc;
		DescriptorSetLayout* descriptorLayout;
	};

	class Renderer;
	struct ParamSetVulkanUpdateControl: public ParamSetUpdateControl
	{
		DescriptorSet* descriptorSet[MAX_FRAMES_IN_FLIGHT];
		DescriptorWriter writer;
		GraphicsDevice* device;
		void UpdateDescriptorSet(ShaderParamInfo& param, bool forceFullUpdate) override;
		U32 GetCurrentFrameIndex() const override;

		~ParamSetVulkanUpdateControl() override;
	};
}