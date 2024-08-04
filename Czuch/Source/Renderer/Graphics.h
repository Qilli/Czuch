#pragma once
#include"./Core/EngineCore.h"
#include "Core/Math.h"
#include<memory>
#include"Subsystems/Assets/Asset/Asset.h"

namespace Czuch
{

	static const U8 s_max_descriptors_per_set = 32;
	static const U8 s_max_vertex_streams = 16;
	static const U8 s_max_vertex_attributes = 16;
	static const U8 k_max_descriptor_set_layouts = 8;

	typedef I32 Handle;
	#define INVALID_HANDLE(Type) Type() 
	#define HANDLE_IS_VALID(h)(h.handle!=-1)
	#define INVALIDATE_HANDLE(h)h.handle=-1;
	class GraphicsDevice;

	struct SceneData
	{
		Mat4x4 view;
		Mat4x4 proj;
		Mat4x4 viewproj;
		Vec4 ambientColor;
	};

	struct ResourceHandle
	{
	public:
		Handle handle;
		ResourceHandle(int val) :handle(val) {}
		ResourceHandle() { handle = -1; }
	};

	struct PipelineHandle: public ResourceHandle
	{
	};

	struct RenderPassHandle : public ResourceHandle
	{

	};

	struct ShaderHandle : public ResourceHandle
	{

	};

	struct DescriptorSetLayoutHandle : public ResourceHandle
	{

	};

	struct FrameBufferHandle : public ResourceHandle
	{
	};

	struct CommandBufferHandle : public ResourceHandle
	{

	};

	struct BufferHandle : public ResourceHandle
	{

	};

	struct TextureHandle : public ResourceHandle
	{

	};

	struct MaterialHandle : public ResourceHandle
	{

	};

	struct MaterialInstanceHandle : public ResourceHandle
	{

	};

	struct MeshHandle : public ResourceHandle
	{

	};

	enum class CZUCH_API RenderPassType
	{
		MainForward,
		Shadow,
		PostProcess,
		OffscreenTexture,
		Custom
	};

	enum CommandBufferUseFlag
	{
		NONE = 0,
		ONE_TIME_SUBMIT = 1 << 0,
		SECONDARY = 1 << 1,
		RENDER_PASS_CONTINUE = 1 << 2,
		SIMULTANEOUS_USE = 1 << 3,
	};

	enum QUEUE_TYPE
	{
		QUEUE_GRAPHICS,
		QUEUE_COMPUTE,
		QUEUE_COPY,
		QUEUE_VIDEO_DECODE,
		QUEUE_ALL_COUNT,
	};


	enum class GraphicsDeviceCapability
	{
		NONE = 0,
		TESSELLATION = 1 << 0,
		CONSERVATIVE_RASTERIZATION = 1 << 1,
		RASTERIZER_ORDERED_VIEWS = 1 << 2,
		UAV_LOAD_FORMAT_COMMON = 1 << 3,
		UAV_LOAD_FORMAT_R11G11B10_FLOAT = 1 << 4,
		RENDERTARGET_AND_VIEWPORT_ARRAYINDEX_WITHOUT_GS = 1 << 5,
		VARIABLE_RATE_SHADING = 1 << 6,
		VARIABLE_RATE_SHADING_TIER2 = 1 << 7,
		MESH_SHADER = 1 << 8,
		RAYTRACING = 1 << 9,
		PREDICATION = 1 << 10,
		SAMPLER_MINMAX = 1 << 11,
		DEPTH_BOUNDS_TEST = 1 << 12,
		SPARSE_BUFFER = 1 << 13,
		SPARSE_TEXTURE2D = 1 << 14,
		SPARSE_TEXTURE3D = 1 << 15,
		SPARSE_NULL_MAPPING = 1 << 16,
		GENERIC_SPARSE_TILE_POOL = 1 << 17,
		DEPTH_RESOLVE_MIN_MAX = 1 << 18,
		STENCIL_RESOLVE_MIN_MAX = 1 << 19,
	};

	enum class Format
	{
		UNKNOWN,

		R32G32B32A32_FLOAT,
		R32G32B32A32_UINT,
		R32G32B32A32_SINT,

		R32G32B32_FLOAT,
		R32G32B32_UINT,
		R32G32B32_SINT,

		R16G16B16A16_FLOAT,
		R16G16B16A16_UNORM,
		R16G16B16A16_UINT,
		R16G16B16A16_SNORM,
		R16G16B16A16_SINT,

		R32G32_FLOAT,
		R32G32_UINT,
		R32G32_SINT,
		D32_FLOAT_S8X24_UINT,	// depth (32-bit) + stencil (8-bit) | SRV: R32_FLOAT (default or depth aspect), R8_UINT (stencil aspect)

		R10G10B10A2_UNORM,
		R10G10B10A2_UINT,
		R11G11B10_FLOAT,
		R8G8B8A8_UNORM,
		R8G8B8A8_UNORM_SRGB,
		R8G8B8A8_UINT,
		R8G8B8A8_SNORM,
		R8G8B8A8_SINT,
		B8G8R8A8_UNORM,
		B8G8R8A8_UNORM_SRGB,
		R16G16_FLOAT,
		R16G16_UNORM,
		R16G16_UINT,
		R16G16_SNORM,
		R16G16_SINT,
		D32_FLOAT,				// depth (32-bit) | SRV: R32_FLOAT
		R32_FLOAT,
		R32_UINT,
		R32_SINT,
		D24_UNORM_S8_UINT,		// depth (24-bit) + stencil (8-bit) | SRV: R24_INTERNAL (default or depth aspect), R8_UINT (stencil aspect)
		R9G9B9E5_SHAREDEXP,

		R8G8_UNORM,
		R8G8_UINT,
		R8G8_SNORM,
		R8G8_SINT,
		R16_FLOAT,
		D16_UNORM,				// depth (16-bit) | SRV: R16_UNORM
		R16_UNORM,
		R16_UINT,
		R16_SNORM,
		R16_SINT,

		R8_UNORM,
		R8_UINT,
		R8_SNORM,
		R8_SINT,

		// Formats that are not usable in render pass must be below because formats in render pass must be encodable as 6 bits:

		BC1_UNORM,			// Three color channels (5 bits:6 bits:5 bits), with 0 or 1 bit(s) of alpha
		BC1_UNORM_SRGB,		// Three color channels (5 bits:6 bits:5 bits), with 0 or 1 bit(s) of alpha
		BC2_UNORM,			// Three color channels (5 bits:6 bits:5 bits), with 4 bits of alpha
		BC2_UNORM_SRGB,		// Three color channels (5 bits:6 bits:5 bits), with 4 bits of alpha
		BC3_UNORM,			// Three color channels (5 bits:6 bits:5 bits) with 8 bits of alpha
		BC3_UNORM_SRGB,		// Three color channels (5 bits:6 bits:5 bits) with 8 bits of alpha
		BC4_UNORM,			// One color channel (8 bits)
		BC4_SNORM,			// One color channel (8 bits)
		BC5_UNORM,			// Two color channels (8 bits:8 bits)
		BC5_SNORM,			// Two color channels (8 bits:8 bits)
		BC6H_UF16,			// Three color channels (16 bits:16 bits:16 bits) in "half" floating point
		BC6H_SF16,			// Three color channels (16 bits:16 bits:16 bits) in "half" floating point
		BC7_UNORM,			// Three color channels (4 to 7 bits per channel) with 0 to 8 bits of alpha
		BC7_UNORM_SRGB,		// Three color channels (4 to 7 bits per channel) with 0 to 8 bits of alpha

		NV12,				// video YUV420; SRV Luminance aspect: R8_UNORM, SRV Chrominance aspect: R8G8_UNORM
	};

	enum class Usage
	{
		DEFAULT,	
		MEMORY_USAGE_GPU_ONLY,	    // GPU local memory
		MEMORY_USAGE_CPU_ONLY,	// CPU write, can be read by GPU but with performance hit
		MEMORY_USAGE_CPU_TO_GPU, //CPU write, can be read by GPU and is faster than access from cpu(special small memory region on gpu)
		MEMORY_USAGE_GPU_TO_CPU, //memory that can be safely read from CPU
	};

	enum class BindFlag
	{
		NONE = 0,
		VERTEX_BUFFER = 1 << 0,
		INDEX_BUFFER = 1 << 1,
		CONSTANT_BUFFER = 1 << 2,
		SHADER_RESOURCE = 1 << 3,
		RENDER_TARGET = 1 << 4,
		DEPTH_STENCIL = 1 << 5,
		UNORDERED_ACCESS = 1 << 6,
		SHADING_RATE = 1 << 7,
		UNIFORM_BUFFER= 1<<8,
	};

	enum CZUCH_API DescriptorType
	{
		SAMPLER = 0,
		UNIFORM_BUFFER = 1,
		STORAGE_BUFFER = 2,
		UNIFORM_BUFFER_DYNAMIC = 3,
		STORAGE_BUFFER_DYNAMIC = 4,
		INPUT_ATTACHMENT = 5,
	};

	enum class ResourceState
	{
		// Common resource states:
		UNDEFINED = 0,						// invalid state (don't preserve contents)
		SHADER_RESOURCE = 1 << 0,			// shader resource, read only
		SHADER_RESOURCE_COMPUTE = 1 << 1,	// shader resource, read only, non-pixel shader
		UNORDERED_ACCESS = 1 << 2,			// shader resource, write enabled
		COPY_SRC = 1 << 3,					// copy from
		COPY_DST = 1 << 4,					// copy to

		// Texture specific resource states:
		RENDERTARGET = 1 << 5,				// render target, write enabled
		DEPTHSTENCIL = 1 << 6,				// depth stencil, write enabled
		DEPTHSTENCIL_READONLY = 1 << 7,		// depth stencil, read only
		SHADING_RATE_SOURCE = 1 << 8,		// shading rate control per tile

		// GPUBuffer specific resource states:
		VERTEX_BUFFER = 1 << 9,				// vertex buffer, read only
		INDEX_BUFFER = 1 << 10,				// index buffer, read only
		CONSTANT_BUFFER = 1 << 11,			// constant buffer, read only
		INDIRECT_ARGUMENT = 1 << 12,		// argument buffer to DrawIndirect() or DispatchIndirect()
		RAYTRACING_ACCELERATION_STRUCTURE = 1 << 13, // acceleration structure storage or scratch
		PREDICATION = 1 << 14,				// storage for predication comparison value
	};

	enum class ComponentSwizzle
	{
		R,
		G,
		B,
		A,
		ZERO,
		ONE,
	};

	enum class ShaderStage: U32
	{
		MS= (1 << 0),		// Mesh Shader
		AS= (1 << 1),		// Amplification Shader
		VS=(1 << 2),		// Vertex Shader
		HS = (1 << 3),		// Hull Shader
		DS = (1 << 4),		// Domain Shader
		GS = (1 << 5),		// Geometry Shader
		PS = (1 << 6),		// Pixel Shader
		CS= (1 << 7),		// Compute Shader
		LIB = (1 << 8),	// Shader Library
		ALL=0xFF,
		Count,
	};

	ENUM_FLAG_OPERATORS(ShaderStage)

	ShaderStage StringToShaderStage(const CzuchStr& stage);


	enum class ShaderFormat
	{
		NONE,		// Not used
		GLSL,
		SPIRV,		// SPIR-V
	};

	enum class ShaderModel
	{
		SM_5_0,
		SM_6_0,
		SM_6_1,
		SM_6_2,
		SM_6_3,
		SM_6_4,
		SM_6_5,
		SM_6_6,
		SM_6_7,
	};

	enum class PrimitiveTopology
	{
		UNDEFINED,
		TRIANGLELIST,
		TRIANGLESTRIP,
		POINTLIST,
		LINELIST,
		LINESTRIP,
		PATCHLIST,
	};

	enum class CompFunc
	{
		NEVER,
		LESS,
		EQUAL,
		LESS_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_EQUAL,
		ALWAYS,
	};

	enum class StencilOp
	{
		KEEP,
		ZERO,
		REPLACE,
		INCR_AND_CLAMP,
		DECR_AND_CLAMP,
		INVERT,
		INCR_AND_WRAP,
		DECR_AND_WRAP,
	};

	enum class Blend
	{
		ZERO,
		ONE,
		SRC_COLOR,
		ONE_MINUS_SRC_COLOR,
		DST_COLOR,
		ONE_MINUS_DST_COLOR,
		SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA,
		CONSTANT_COLOR,
		ONE_MINUS_CONSTANT_COLOR,
		CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA,
		SRC_ALPHA_SATURATE,
		SRC1_COLOR,
		ONE_MINUS_SRC1_COLOR,
		SRC1_ALPHA,
		ONE_MINUS_SRC1_ALPHA,
		DST_ALPHA,
		ONE_MINUS_DST_ALPHA,
	};

	enum class BlendOp
	{
		ADD,
		SUBTRACT,
		REV_SUBTRACT,
		MIN,
		MAX,
	};

	enum class PolygonMode
	{
		WIREFRAME,
		SOLID,
		POINTS,
	};

	enum class CullMode
	{
		NONE,
		FRONT,
		BACK,
	};

	enum class InputClassification
	{
		PER_VERTEX_DATA,
		PER_INSTANCE_DATA,
	};

	enum class TextureAddressMode
	{
		WRAP,
		MIRROR,
		CLAMP,
		BORDER,
		MIRROR_ONCE,
		EDGE_CLAMP,
	};

	enum class TextureFilter
	{
		LINEAR,
		NEAREST
	};

	enum class ColorWrite
	{
		DISABLE = 0,
		ENABLE_RED = 1 << 0,
		ENABLE_GREEN = 1 << 1,
		ENABLE_BLUE = 1 << 2,
		ENABLE_ALPHA = 1 << 3,
		ENABLE_ALL = ~0,
	};

	enum class DepthWriteMask
	{
		ZERO,	// Disables depth write
		ALL,	// Enables depth write
	};

	enum class BindPoint
	{
		BIND_POINT_GRAPHICS = 0,
		BIND_POINT_COMPUTE = 1,
		BIND_POINT_RAY_TRACING=2,
	};

	CZUCH_API enum class RenderLayer
	{
		LAYER_0,
		LAYER_1,
		LAYER_2,
		LAYER_3,
		LAYER_4,
		LAYER_5,
		LAYER_6,
		LAYER_7,
	};

	CZUCH_API enum class RenderType
	{
		General,
		UI,
		Debug,
		Editor,
		Overlay,
	};

	enum class ImageUsageFlag : U32
	{	
		TRANSFER_SRC = 1 << 0,
		TRANSFER_DST = 1 << 1,
		SAMPLED = 1 << 2,
		STORAGE = 1 << 3,
		COLOR_ATTACHMENT = 1 << 4,
		DEPTH_STENCIL_ATTACHMENT = 1 << 5,
		TRANSIENT_ATTACHMENT = 1 << 6,
		INPUT_ATTACHMENT = 1 << 7,
	};

	ENUM_FLAG_OPERATORS(ImageUsageFlag)

	enum class ImageAspectFlag : U32
	{
		COLOR = 1 << 0,
		DEPTH = 1 << 1,
		STENCIL = 1 << 2,
		METADATA = 1 << 3,
		PLANE_0 = 1 << 4,
		PLANE_1 = 1 << 5,
		PLANE_2 = 1 << 6,
		MEMORY_PLANE_0 = 1 << 7,
		MEMORY_PLANE_1 = 1 << 8,
		MEMORY_PLANE_2 = 1 << 9,
	};

	ENUM_FLAG_OPERATORS(ImageAspectFlag)

	enum class ImageLayout
	{
		UNDEFINED,
		GENERAL,
		COLOR_ATTACHMENT_OPTIMAL,
		DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		SHADER_READ_ONLY_OPTIMAL,
		TRANSFER_SRC_OPTIMAL,
		TRANSFER_DST_OPTIMAL,
		PREINITIALIZED,
		PRESENT_SRC,
		SHADING_RATE_OPTIMAL,
		FRAGMENT_DENSITY_MAP_OPTIMAL,
		DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
		DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
		PRESENT_SRC_KHR,
		SHARED_PRESENT_KHR,
		DEPTH_ATTACHMENT_OPTIMAL,
		DEPTH_READ_ONLY_OPTIMAL,
		STENCIL_ATTACHMENT_OPTIMAL,
		STENCIL_READ_ONLY_OPTIMAL,
	};


#pragma region Info definitions
	struct Texture;
	struct RenderPass;


	struct GraphicsDeviceResource
	{
		std::shared_ptr<void> m_InternalResourceState;
		GraphicsDevice* device;
		AssetHandle assetHandle = {.handle=-1};

		inline bool IsValid() const { return m_InternalResourceState != nullptr; }
		GraphicsDeviceResource() = default;
		virtual ~GraphicsDeviceResource() = default;
	};

	struct Shader : public GraphicsDeviceResource
	{
		ShaderStage stage = ShaderStage::Count;
	};

	struct SwapChainDesc
	{
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t buffer_count = 2;
		float clear_color[4] = { 0,0,0,1 };
		Format format = Format::R10G10B10A2_UNORM;
		bool fullscreen = false;
		bool vsync = true;
		bool allow_hdr = true;
	};

	struct RenderPassDesc
	{
		RenderPassType type;

		RenderPassDesc()
		{
			type = RenderPassType::MainForward;
		}
	};

	struct ViewportDesc
	{
		float x;
		float y;
		float width;
		float height;
		float minDepth;
		float maxDepth;
	};

	struct ScissorsDesc
	{
		I32 offsetX;
		I32 offsetY;
		U32 width;
		U32 height;
	};

	struct FrameBufferDesc
	{
		RenderPassHandle renderPass;
		U32 width;
		U32 height;
	};

	struct BufferDesc
	{
		U64 size = 0;
		U64 elementsCount = 0;
		Usage usage = Usage::DEFAULT;
		BindFlag bind_flags = BindFlag::NONE;
		U32 stride = 0;
		Format format = Format::UNKNOWN;
		bool createMapped = false;
		void* initData = nullptr;

	};

	struct DescriptorSetLayoutDesc
	{
		struct Binding
		{
			StringID bindingName;
			DescriptorType type = DescriptorType::UNIFORM_BUFFER;
			U32 size = 0;
			U16 index = 0;
			U16 count = 0;
			bool internalParam = false;
		};

		Binding bindings[s_max_descriptors_per_set];
		U32 bindingsCount = 0;
		U32 setIndex = 0;
		U32 shaderStage;

		DescriptorSetLayoutDesc()
		{
			shaderStage = (U32)ShaderStage::ALL;
			Reset();
		}

		DescriptorSetLayoutDesc& Reset();
		DescriptorSetLayoutDesc& AddBinding(CzuchStr name,DescriptorType type, U32 bindingIndex, U32 count,U32 size,bool internalParam);
	};


	struct Buffer;
	struct DescriptorSetLayout;
	struct ShaderParamsSet
	{
		struct ShaderParamInfo
		{
			StringID paramName;
			I32 resource;
			DescriptorType type;
			U16 binding;
		};

		ShaderParamInfo descriptors[s_max_descriptors_per_set];
		U16 descriptorsCount;

		ShaderParamsSet()
		{
			Reset();
		}

		ShaderParamsSet& Reset();
		ShaderParamsSet& AddBuffer(CzuchStr name,BufferHandle buffer,U16 binding);
		ShaderParamsSet& AddSampler(CzuchStr name,TextureHandle color_texture, U16 binding);
	};

	union ClearValue
	{
		float color[4];
		struct ClearDepthStencil
		{
			float depth;
			uint32_t stencil;
		} depth_stencil;
	};


	struct Swizzle
	{
		ComponentSwizzle r = ComponentSwizzle::R;
		ComponentSwizzle g = ComponentSwizzle::G;
		ComponentSwizzle b = ComponentSwizzle::B;
		ComponentSwizzle a = ComponentSwizzle::A;
	};

	struct RasterizerState
	{
		PolygonMode fill_mode = PolygonMode::SOLID;
		CullMode cull_mode = CullMode::NONE;
	};

	struct BlendState
	{
		bool alpha_to_coverage_enable = false;
		bool independent_blend_enable = false;

		struct RenderTargetBlendState
		{
			bool blendEnable = false;
			Blend srcColorBlendFactor = Blend::SRC_ALPHA;
			Blend dstColorBlendFactor = Blend::ONE_MINUS_SRC_ALPHA;
			BlendOp colorBlendOp = BlendOp::ADD;
			Blend srcAlphaBlendFactor = Blend::ONE;
			Blend dstAlphaBlendFactor = Blend::ONE;
			BlendOp alphaBlendOp = BlendOp::ADD;
			ColorWrite colorWriteMask = ColorWrite::ENABLE_ALL;
		};
		RenderTargetBlendState blendSettings;

		void SetAdditiveBlending()
		{
			blendSettings.colorWriteMask = ColorWrite::ENABLE_ALL;
			blendSettings.blendEnable = true;
			blendSettings.srcColorBlendFactor = Blend::ONE;
			blendSettings.dstColorBlendFactor = Blend::DST_ALPHA;
			blendSettings.colorBlendOp = BlendOp::ADD;
			blendSettings.srcAlphaBlendFactor = Blend::ONE;
			blendSettings.dstAlphaBlendFactor = Blend::ZERO;
			blendSettings.alphaBlendOp = BlendOp::ADD;
		}

		void SetAlphaBlend()
		{
			blendSettings.colorWriteMask = ColorWrite::ENABLE_ALL;
			blendSettings.blendEnable = true;
			blendSettings.srcColorBlendFactor = Blend::ONE_MINUS_DST_ALPHA;
			blendSettings.dstColorBlendFactor = Blend::DST_ALPHA;
			blendSettings.colorBlendOp = BlendOp::ADD;
			blendSettings.srcAlphaBlendFactor = Blend::ONE;
			blendSettings.dstAlphaBlendFactor = Blend::ZERO;
			blendSettings.alphaBlendOp = BlendOp::ADD;
		}

		void DisableBlending()
		{
			blendSettings.blendEnable = false;
		}
	};

	struct DepthStencilState
	{
		bool depth_enable = true;
		bool depth_write_enable = true;
		DepthWriteMask depth_write_mask = DepthWriteMask::ZERO;
		CompFunc depth_func = CompFunc::NEVER;
		bool stencil_enable = false;
		uint8_t stencil_read_mask = 0xff;
		uint8_t stencil_write_mask = 0xff;

		struct StencilSettings
		{
			StencilOp stencil_fail_op = StencilOp::KEEP;
			StencilOp stencil_depth_fail_op = StencilOp::KEEP;
			StencilOp stencil_pass_op = StencilOp::KEEP;
			CompFunc stencil_func = CompFunc::NEVER;
		};
		StencilSettings stencilSettings;
	};

	struct VertexAttribute {

		U16 location = 0;
		U16 binding = 0;
		U32 offset = 0;
		Format format = Format::UNKNOWN;
	};

	struct VertexStream {

		U16 binding = 0;
		U16 stride = 0;
		InputClassification input_rate = InputClassification::PER_VERTEX_DATA;
	};

	struct InputVertexLayout
	{
		U16 vertexStreamsCount = 0;
		U16 vertexAttributesCount = 0;

		VertexAttribute attributes[s_max_vertex_attributes];
		VertexStream streams[s_max_vertex_streams];

		void Reset();
		void AddAttribute(const VertexAttribute& attribute);
		void AddStream(const VertexStream& stream);
	};

	struct MaterialInstanceDesc;
	struct MaterialInstanceParams;
	struct PipelineStateDesc
	{
		AssetHandle vs;
		AssetHandle ps;
		BlendState bs;
		RasterizerState rs;
		DepthStencilState dss;
		InputVertexLayout il;
		PrimitiveTopology pt = PrimitiveTopology::TRIANGLELIST;
		DescriptorSetLayoutDesc layouts[k_max_descriptor_set_layouts];
		U16 layoutsCount = 0;
		BindPoint bindPoint;

		bool IsTransparent() const
		{
			return bs.blendSettings.blendEnable;
		}

		void AddLayout(DescriptorSetLayoutDesc layout)
		{
			if (layoutsCount >= k_max_descriptor_set_layouts)
			{
				return;
			}
			layouts[layoutsCount++] = layout;
		}


		void SetParams(MaterialInstanceDesc& desc,MaterialInstanceParams& params);


		PipelineStateDesc() = default;

		PipelineStateDesc(PipelineStateDesc& other) noexcept
		{
			*this = std::move(other);
		}

		PipelineStateDesc& operator=(PipelineStateDesc&& other) noexcept
		{
			if (&other != this)
			{
				this->vs=std::move(other.vs);
				this->ps = std::move(other.ps);
				this->bs = std::move(other.bs);
				this->rs = std::move(other.rs);
				this->dss = std::move(other.dss);
				this->il = std::move(other.il);
				this->pt = std::move(other.pt);
				this->layoutsCount = std::move(other.layoutsCount);

				for (int a = 0; a < this->layoutsCount; ++a)
				{
					this->layouts[a] = std::move(other.layouts[a]);
				}

				this->bindPoint = std::move(other.bindPoint);
			}
			return *this;
		}

		PipelineStateDesc& operator=(PipelineStateDesc& other) noexcept
		{
			if (&other != this)
			{
				this->vs = (other.vs);
				this->ps = (other.ps);
				this->bs = (other.bs);
				this->rs = (other.rs);
				this->dss =(other.dss);
				this->il = (other.il);
				this->pt = (other.pt);
				this->layoutsCount = (other.layoutsCount);

				for (int a = 0; a < this->layoutsCount; ++a)
				{
					this->layouts[a] = (other.layouts[a]);
				}

				this->bindPoint = (other.bindPoint);
			}
			return *this;
		}


	};

	struct  CZUCH_API MaterialDesc
	{
		PipelineStateDesc pipelineDesc;
		CzuchStr materialName;
		int materialFlags;

		MaterialDesc()
		{
			materialFlags = 0;
		}

		MaterialDesc(MaterialDesc& other) noexcept
		{
			*this = std::move(other);
		}

		bool IsTransparent() const
		{
			return pipelineDesc.IsTransparent();
		}

		MaterialDesc& operator=(MaterialDesc&& other) noexcept
		{
			if (&other != this)
			{
				this->materialName = std::move(other.materialName);
				this->pipelineDesc = std::move(other.pipelineDesc);
				this->materialFlags = std::move(other.materialFlags);
			}
			return *this;
		}

		MaterialDesc& operator=(MaterialDesc& other) noexcept
		{
			if (&other != this)
			{
				this->materialFlags = other.materialFlags;
				this->pipelineDesc = other.pipelineDesc;
			}
			return *this;
		}

		void AddLayout(DescriptorSetLayoutDesc layout)
		{
			pipelineDesc.AddLayout(layout);
		}
	};

	struct CZUCH_API MaterialInstanceDesc
	{
		struct ShaderParamDesc
		{
			CzuchStr name;
			DescriptorType type;
			I32 resource;
		};

		AssetHandle materialAsset;
		bool isTransparent = false;
		std::vector<ShaderParamDesc> paramsDesc;
		MaterialInstanceDesc()
		{
			Reset();
		}

		MaterialInstanceDesc& Reset();
		MaterialInstanceDesc& AddBuffer(const CzuchStr& name, BufferHandle buffer);
		MaterialInstanceDesc& AddSampler(const CzuchStr& name, TextureHandle color_texture);
		void SetTransparent(bool value){isTransparent=value;}
	};

	struct MaterialInstanceParams
	{
		ShaderParamsSet shaderParamsDesc[k_max_descriptor_set_layouts];
		int setsCount = 0;
		MaterialInstanceParams()
		{
			Reset();
		}

		MaterialInstanceParams& Reset();
		MaterialInstanceParams& AddBuffer(int set,CzuchStr& name, BufferHandle buffer, U16 binding);
		MaterialInstanceParams& AddSampler(int set,CzuchStr& name, TextureHandle color_texture, U16 binding);
	};

	struct SamplerDesc
	{
		TextureAddressMode addressModeU = TextureAddressMode::WRAP;
		TextureAddressMode addressModeV = TextureAddressMode::WRAP;
		TextureAddressMode addressModeW = TextureAddressMode::WRAP;
		TextureFilter magFilter = TextureFilter::LINEAR;
		TextureFilter minFilter = TextureFilter::LINEAR;
		bool anisoEnabled=false;
	};

	struct TextureDesc
	{
		SamplerDesc samplerDesc;
		enum class Type
		{
			TEXTURE_1D,
			TEXTURE_2D,
			TEXTURE_3D,
			TEXTURE_CUBE
		} type = Type::TEXTURE_2D;
		U32 width = 1;
		U32 height = 1;
		U32 depth = 1;
		U32 array_size = 1;
		U32 mip_levels = 1;
		Format format = Format::UNKNOWN;
		U32 sample_count = 1;
		Usage usage = Usage::DEFAULT;
		BindFlag bind_flags = BindFlag::NONE;
		ClearValue clear = {};
		ResourceState resourceType = ResourceState::SHADER_RESOURCE;
		ImageUsageFlag usageFlags = ImageUsageFlag::SAMPLED|ImageUsageFlag::TRANSFER_DST;
		ImageLayout initialLayout = ImageLayout::SHADER_READ_ONLY_OPTIMAL;
		ImageAspectFlag aspectFlags = ImageAspectFlag::COLOR;
		Swizzle swizzle;
		U8* texData;

		inline U32 GetSize() const
		{
			return width * height * 4;
		}
	};

	struct MeshData
	{
		std::vector<Vec3> positions;
		std::vector<Vec3> normals;
		std::vector<Vec4> colors;
		std::vector<Vec4> uvs0;
		std::vector<U32> indices;
		MaterialInstanceHandle material;
		CzuchStr meshName;

	public:
		MeshData()
		{
			material = INVALID_HANDLE(MaterialInstanceHandle);
		}

		MeshData(MeshData& other) noexcept
		{
			*this = std::move(other);
		}

		MeshData(MeshData&& other) noexcept
		{
			*this = std::move(other);
		}

		MeshData& operator=(MeshData&& other) noexcept
		{
			if (&other != this)
			{
				this->positions = std::move(other.positions);
				this->normals = std::move(other.normals);
				this->colors = std::move(other.colors);
				this->uvs0 = std::move(other.uvs0);
				this->material = other.material;
				this->meshName = std::move(other.meshName);
				this->indices = std::move(other.indices);

				INVALIDATE_HANDLE(other.material);
			}
			return *this;
		}
	};

	struct Mesh: GraphicsDeviceResource
	{
		MeshData* data;

		inline bool HasNormals() const { return data->normals.size() > 0; }
		inline bool HasColors() const { return data->colors.size() > 0; }
		inline bool HasUV0() const { return data->uvs0.size() > 0; }
		
		constexpr const MeshData& GetMeshData() const { return *data; }

		BufferHandle positionsHandle;
		BufferHandle normalsHandle;
		BufferHandle colorsHandle;
		BufferHandle uvs0Handle;
		BufferHandle indicesHandle;
		MaterialInstanceHandle materialHandle;

		Mesh()
		{
			positionsHandle = INVALID_HANDLE(BufferHandle);
			normalsHandle = INVALID_HANDLE(BufferHandle);
			colorsHandle = INVALID_HANDLE(BufferHandle);
			uvs0Handle = INVALID_HANDLE(BufferHandle);
			indicesHandle = INVALID_HANDLE(BufferHandle);
			materialHandle = INVALID_HANDLE(MaterialInstanceHandle);
		}

		~Mesh();
	};

	enum class MaterialFlag
	{
		NONE =0,
		USE_SCENE_DATA = 1<<0 
	};

	struct Material : public GraphicsDeviceResource
	{
		MaterialDesc desc{};
		constexpr const MaterialDesc& GetDesc() const { return desc; }

		PipelineHandle pipeline;

	};

	struct MaterialInstance : public GraphicsDeviceResource
	{
		MaterialInstanceDesc desc{};
		MaterialInstanceParams params;
		MaterialHandle handle;
		bool IsTransparent() const { return desc.isTransparent; }
		constexpr const MaterialInstanceDesc& GetDesc() const { return desc; }
	};

	struct Pipeline : public GraphicsDeviceResource
	{
		PipelineStateDesc m_desc{};
		constexpr const PipelineStateDesc& GetDesc() const { return m_desc; }

		ShaderHandle vs;
		ShaderHandle ps;
		DescriptorSetLayoutHandle layouts[k_max_descriptor_set_layouts];
		U16 layoutsCount;
		void AddLayout(DescriptorSetLayoutHandle layout)
		{
			if (layoutsCount >= k_max_descriptor_set_layouts)
			{
				return;
			}
			layouts[layoutsCount++] = layout;
		}
	};

	struct SwapChain : public GraphicsDeviceResource
	{
		SwapChainDesc desc;
		constexpr const SwapChainDesc& GetResourceInfo() const { return desc; }
	};

	struct RenderPass : public GraphicsDeviceResource
	{
		RenderPassDesc desc;
		constexpr const RenderPassDesc& GetDesc() const { return desc; }
	};

	struct FrameBuffer : public GraphicsDeviceResource
	{
		FrameBufferDesc desc;
		constexpr const FrameBufferDesc& GetDesc() const { return desc; }
	};

	struct Buffer : public GraphicsDeviceResource
	{
		BufferDesc desc;
		constexpr const BufferDesc& GetResourceInfo() const { return desc; }
	};

	struct DescriptorSetLayout : public GraphicsDeviceResource
	{
		DescriptorSetLayoutDesc desc;
		constexpr const DescriptorSetLayoutDesc& GetResourceInfo() const { return desc; }
	};

	struct Texture : public GraphicsDeviceResource
	{
		TextureDesc	desc;
		constexpr const TextureDesc& GetDesc() const { return desc; }
	};

#pragma endregion 



}