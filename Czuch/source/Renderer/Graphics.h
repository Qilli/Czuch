#pragma once
#include"./Core/EngineCore.h"
#include "Core/Math.h"
#include"Subsystems/Assets/Asset/Asset.h"


namespace Czuch
{

	static const U8 s_max_descriptors_per_set = 8;
	static const U8 s_max_vertex_streams = 16;
	static const U8 s_max_vertex_attributes = 16;
	static const U8 k_max_descriptor_set_layouts = 8;
	static const U8 k_max_image_outputs = 8;
	static const U8 k_max_render_passes = 4;

	static constexpr U32 MAX_LIGHTS_IN_SCENE = 1024;
	static constexpr U32 MATERIAL_DATA_CAPACITY = 200;
	static constexpr U32 MAX_LIGHTS_IN_TILE = 32;
	static constexpr U32 TILE_SIZE = 32;

	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
	static constexpr int PUSH_CONSTANTS_SIZE = sizeof(glm::mat4x4) + sizeof(glm::ivec4);
	static constexpr int INIT_MAX_RENDER_OBJECTS = 2056;
	static constexpr int MAX_MATERIALS_OBJECTS = 1024;
	static constexpr int MAX_LINES_IN_SCENE = 10000;
	static constexpr int MAX_DEBUG_TRIANGLES_IN_SCENE = 10000;
	static constexpr int MAX_DEBUG_POINTS_IN_SCENE = 10000;

	typedef I32 Handle;
#define INVALID_HANDLE(Type) Type() 
#define HANDLE_IS_VALID(h)(h.handle!=-1)
#define INVALIDATE_HANDLE(h)h.handle=-1; 
	class GraphicsDevice;

	enum DebugRenderingFlag : U32
	{
		MaterialIndexAsColor = 1 << 0,
	};

	struct PositionVertex
	{
		Vec3 position;
	};

	struct SceneData
	{
		Mat4x4 view;
		Mat4x4 proj;
		Mat4x4 viewproj;
		Vec4 ambientColor;
	};

	struct LightsTileData
	{
		U32 lightStart;
		U32 lightCount;
	};

	struct TilesDataContainer
	{
		glm::ivec4 screenSize;
		Array<LightsTileData> tilesData;
	};

	struct SceneDataBuffers
	{
		Handle sceneDataBuffer;
		Handle lightsDataBuffer;
		Handle lightsIndexListBuffer;
		Handle tilesDataBuffer;
		Handle renderObjectsBuffer;
	};

	struct RenderObjectGPUData
	{
		Mat4x4 localToWorldTransformation;
		Mat4x4 invTransposeToWorldMatrix;
		glm::ivec4 materialAndFlags; // material index and flags
	};

	struct MaterialObjectGPUData
	{
		Vec4 diffuseColor;
		Vec4 specularColor;
	};

	struct LightData
	{
		Vec4 positionWithType;
		Vec4 color;
		Vec4 directionWithRange;
		Vec4 spotInnerOuterAngle;
	};


	struct ColorUBO
	{
		Vec4 color;
	};

	struct ResourceHandle
	{
	public:
		Handle handle;
		ResourceHandle(int val) :handle(val) {}
		ResourceHandle() { handle = Invalid_Handle_Id; }
	};

	enum CZUCH_API RenderPassType : U32
	{
		MainForward = 0,
		Shadow = 1,
		PostProcess = 1 << 1,
		OffscreenTexture = 1 << 2,
		UI = 1 << 3,
		Final = 1 << 4,
		DepthPrePass = 1 << 5,
		ForwardLighting = 1 << 7,
		DepthLinearPrePass = 1 << 8,
		ForwardLightingTransparent = 1 << 9,
		DebugDraw = 1 << 10,
		ShadowMap = 1 << 11,
		FullScreenPass = 1 << 12,
		Custom = 1 << 13
	};


	struct FrameGraphNodeHandle
	{
		Handle handle;
	};

	struct ResourceHandleWithAsset : public ResourceHandle
	{
		AssetHandle assetHandle;
		ResourceHandleWithAsset(I32 handle, I32 assetHandleId) :ResourceHandle(handle), assetHandle(assetHandleId)
		{

		}

		ResourceHandleWithAsset(I32 handle, AssetHandle assetHandle) :ResourceHandle(handle)
		{
			assetHandle = assetHandle;
		}

		ResourceHandleWithAsset(I32 handle) :ResourceHandle(handle)
		{
			INVALIDATE_HANDLE(assetHandle);
		}


		ResourceHandleWithAsset() :ResourceHandle(Invalid_Handle_Id)
		{
			assetHandle = { Invalid_Handle_Id };
		}
	};

	struct PipelineHandle : public ResourceHandle
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
		I32 offset = -1; // if -1 then this is exclusive buffer, otherwise its part of bigger buffer
		I32 size = 0;
	};

	struct TextureHandle : public ResourceHandleWithAsset
	{
		TextureHandle() :ResourceHandleWithAsset(Invalid_Handle_Id) {}
		TextureHandle(I32 handle, I32 assetHandle) :ResourceHandleWithAsset(handle, assetHandle) {}
		TextureHandle(I32 handle, AssetHandle assetHandle) :ResourceHandleWithAsset(handle, assetHandle) {}
		TextureHandle(I32 handle) :ResourceHandleWithAsset(handle) {}
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


	enum AttachmentLoadOp
	{
		LOAD = 0,
		CLEAR = 1,
		DONT_CARE = 2,
		NONE_KHR = 1000400000,
		NONE_EXT = 1000400000,
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

	bool IsDepthFormat(Format format);
	bool IsDepthFormatWithStencil(Format format);


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
		UNIFORM_BUFFER = 1 << 8,
		STORAGE_BUFFER = 1 << 9,
		INDIRECT_BUFFER = 1 << 10,
	};

	enum CZUCH_API DescriptorType
	{
		SAMPLER = 0,
		UNIFORM_BUFFER = 1,
		STORAGE_BUFFER = 2,
		UNIFORM_BUFFER_DYNAMIC = 3,
		STORAGE_BUFFER_DYNAMIC = 4,
		INPUT_ATTACHMENT = 5,
		STORAGE_BUFFER_SINGLE_DATA = 6,
	};

	enum BufferType
	{
		CUSTOM = 0,
		POSITION = 1,
		NORMAL = 2,
		UV0 = 3,
		UV1 = 4,
		COLOR = 5,
		INDICES = 6,
	};

	enum ResourceState
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
		INDIRECT_ARGUMENT = 1 << 12,		// argument buffer to DrawIndirect or DispatchIndirect
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

	enum class ShaderStage : U32
	{
		MS = (1 << 0),		// Mesh Shader
		AS = (1 << 1),		// Amplification Shader
		VS = (1 << 2),		// Vertex Shader
		HS = (1 << 3),		// Hull Shader
		DS = (1 << 4),		// Domain Shader
		GS = (1 << 5),		// Geometry Shader
		PS = (1 << 6),		// Pixel Shader
		CS = (1 << 7),		// Compute Shader
		LIB = (1 << 8),	// Shader Library
		ALL = 0xFF,
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
		BIND_POINT_RAY_TRACING = 2,
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

	struct IndirectDrawForCommandBufferData
	{
		/// <summary>
		/// Scene data uniform buffer containing all the matrices for view * projection and other scene related data
		/// it always has 0 set and 0 binding
		/// </summary>
		BufferHandle sceneDataBuffer;
		/// <summary>
		/// Buffer with instances data(like for lines start/end, for triangles vertices, etc.)
		/// </summary>
		BufferHandle instancesBuffer;
		U32 set = 0;//set for instances buffer
		U32 binding = 0;//binding for instances buffer
		U32 instancesSize = 0;//size of one instance in bytes
		/// <summary>
		/// Debug material for draw
		/// </summary>
		MaterialInstanceHandle material;
		/// <summary>
		/// We need vertex buffer with basic vertices, then we transform them to screen space using world transformation from instances buffer
		/// </summary>
		BufferHandle vertexBuffer;
		/// <summary>
		/// (Optional)We do not need index buffer for lines, but we need it for triangles
		/// </summary>
		BufferHandle indexBuffer;
		/// <summary>
		/// Information for indirect draw(VkDrawIndexedIndirectCommand)
		/// </summary>
		BufferHandle indirectDrawsCommandsBuffer;
		/// <summary>
		/// offset for starting indirect draw command in vkCmdDrawIndexedIndirect
		/// </summary>
		U32 indirectDrawsCommandsOffset = 0;
	};

	struct ImageLayouInfo
	{
		ImageLayout currentFormat = ImageLayout::SHADER_READ_ONLY_OPTIMAL;
		ImageLayout lastFormat = ImageLayout::SHADER_READ_ONLY_OPTIMAL;
		ImageLayout initFormat = ImageLayout::UNDEFINED;

		bool TryToTransitionTo(ImageLayout newLayout)
		{
			if (currentFormat == newLayout)
			{
				return false;
			}
			lastFormat = currentFormat;
			currentFormat = newLayout;
			return true;
		}

		void SetTo(ImageLayout newLayout)
		{
			lastFormat = currentFormat;
			currentFormat = newLayout;
		}

		void SetInitLayout(ImageLayout newLayout)
		{
			initFormat = newLayout;
		}

		void SetAllTo(ImageLayout newLayout)
		{
			lastFormat = newLayout;
			currentFormat = newLayout;
			initFormat = newLayout;
		}
	};


	struct GraphicsDeviceResource
	{
		std::shared_ptr<void> m_InternalResourceState;
		GraphicsDevice* device;
		AssetHandle assetHandle;

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
		virtual ~RenderPassDesc() = default;
		U16 attachmentsCount = 0;
		size_t hashCache = 0;

		struct RenderPassColorAttachment
		{
			Format format = Format::UNKNOWN;
			ImageLayout layout = ImageLayout::UNDEFINED;
			AttachmentLoadOp loadOp = AttachmentLoadOp::LOAD;

			bool operator==(const RenderPassColorAttachment& other) const
			{
				return format == other.format &&
					layout == other.layout &&
					loadOp == other.loadOp;
			}

			size_t GetHash() const
			{
				size_t h = 0;
				h = std::hash<U16>{}(static_cast<U16>(format));
				h = h * 31 + std::hash<U16>{}(static_cast<U16>(layout));
				h = h * 31 + std::hash<U16>{}(static_cast<U16>(loadOp));
				return h;
			}
		};

		RenderPassColorAttachment colorAttachments[k_max_image_outputs];

		Format depthStencilFormat = Format::UNKNOWN;
		ImageLayout depthStencilFinalLayout;
		AttachmentLoadOp depthLoadOp = AttachmentLoadOp::DONT_CARE;
		AttachmentLoadOp stencilLoadOp = AttachmentLoadOp::DONT_CARE;

		const char* name = nullptr;

		RenderPassType type = RenderPassType::MainForward;

		//RenderPassDesc& Reset();
		RenderPassDesc& AddAttachment(Format format, ImageLayout layout, AttachmentLoadOp loadOp);
		RenderPassDesc& SetDepthStencilTexture(Format format, ImageLayout layout);
		RenderPassDesc& SetName(const char* rpName);
		RenderPassDesc& SetDepthAndStencilLoadOp(AttachmentLoadOp depth, AttachmentLoadOp stencil);

		size_t GetHash() const
		{
			if (hashCache != 0) // If hash is already computed, return it
			{
				return hashCache;
			}

			size_t h = 0;

			// Hash attachmentsCount
			h = std::hash<U16>{}(attachmentsCount);

			// Hash colorAttachments
			for (U16 i = 0; i < attachmentsCount; ++i)
			{
				h = h * 31 + colorAttachments[i].GetHash();
			}

			// Hash depth and stencil parameters
			h = h * 31 + std::hash<U16>{}(static_cast<U16>(depthStencilFormat));
			h = h * 31 + std::hash<U16>{}(static_cast<U16>(depthStencilFinalLayout));
			h = h * 31 + std::hash<U16>{}(static_cast<U16>(depthLoadOp));
			h = h * 31 + std::hash<U16>{}(static_cast<U16>(stencilLoadOp));

			// Hash type
			h = h * 31 + std::hash<U16>{}(static_cast<U16>(type));

			return h;
		}

		bool operator==(const RenderPassDesc& other) const
		{
			// First, quickly compare hashes if available and reasonable
			// This is an optimization. If hashes are different, the objects are definitely different.
			// However, if hashes are the same, you still need to do a full comparison to handle collisions.
			if (GetHash() != other.GetHash())
			{
				return false;
			}

			// Full comparison (necessary even if hashes match due to potential collisions)
			if (attachmentsCount != other.attachmentsCount) return false;

			for (U16 i = 0; i < attachmentsCount; ++i)
			{
				if (!(colorAttachments[i] == other.colorAttachments[i])) return false;
			}

			if (depthStencilFormat != other.depthStencilFormat)
			{
				return false;
			}

			if (depthStencilFinalLayout != other.depthStencilFinalLayout)
			{
				return false;
			}

			if (depthLoadOp != other.depthLoadOp)
			{
				return false;
			}
			if (stencilLoadOp != other.stencilLoadOp)
			{
				return false;
			}

			if (type != other.type)
			{
				return false;
			}

			return true;
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
		U32 renderTargetsCount = 0;
		bool isFinalFrameBuffer = false;

		TextureHandle renderTextures[k_max_image_outputs];
		TextureHandle depthStencilTexture = { Invalid_Handle_Id };
		const char* name = nullptr;

		FrameBufferDesc& AddRenderTexture(TextureHandle texture);
		FrameBufferDesc& SetDepthStencilTexture(TextureHandle texture);
		FrameBufferDesc& SetRenderPass(RenderPassHandle rp) { renderPass = rp; return *this; }
		FrameBufferDesc& SetWidthAndHeight(U32 w, U32 h) { width = w; height = h; return *this; }
		FrameBufferDesc& SetName(const char* newName);
	};


	enum UBOElementType
	{
		VectorType,
		ColorType,
		MatrixType,
	};

	struct UBOLayout {
		struct Element
		{
			U32 offset;
			U32 size;
			UBOElementType elementType;
			StringID name;
		};
		U32 totalSize;
		U8 elementsCount;

		Element elements[s_max_descriptors_per_set];

		void AddElement(U32 offset, U32 size, UBOElementType elemType, StringID name)
		{
			CZUCH_BE_ASSERT(elementsCount < s_max_descriptors_per_set, "UBO size overflow");
			elements[elementsCount].offset = offset;
			elements[elementsCount].size = size;
			elements[elementsCount].elementType = elemType;
			elements[elementsCount].name = name;
			elementsCount++;
		}

		void Reset()
		{
			totalSize = 0;
			elementsCount = 0;
		}

		const Element& GetElement(StringID& name)
		{
			for (U32 i = 0; i < elementsCount; i++)
			{
				if (elements[i].name.GetStrName() == name.GetStrName())
				{
					return elements[i];
				}
			}
			CZUCH_BE_ASSERT(false, "Element not found");
			return elements[0];
		}

		const Element& GetElement(U32 index)
		{
			CZUCH_BE_ASSERT(index < elementsCount, "Element not found");
			return elements[index];
		}

	};

	enum class DescriptorBindingTagType
	{
		NONE,
		LIGHTS_CONTAINER,
		LIGHTS_TILES,
		LIGHTS_INDEXES,
		RENDER_OBJECTS,
		DEBUG_LINES_INSTANCE_DATA,
		DEBUG_TRIANGLES_INSTANCE_DATA,
		DEBUG_POINTS_INSTANCE_DATA,
		MATERIALS_LIGHTING_DATA,
	};

	struct MaterialCustomBufferData
	{

		Array<U8> dataRaw;
		DescriptorBindingTagType tagType = DescriptorBindingTagType::NONE;

		template<typename T>
		void SetUniform(uint32_t offset, const T& value) {
			memcpy(dataRaw.data() + offset, &value, sizeof(T));
		}
		void* GetData() { return dataRaw.data(); }
		Vec4* GetVec4(uint32_t offset) { return (Vec4*)(dataRaw.data() + offset); }
		U32 GetSize() { return dataRaw.size(); }

		MaterialCustomBufferData(void* data, U32 size, DescriptorBindingTagType tag)
		{
			this->dataRaw.resize(size);
			SetData(data, size);
			this->tagType = tag;
		}

		void SetData(void* data, U32 size)
		{
			dataRaw.resize(size);
			memcpy(dataRaw.data(), data, size);
		}

		MaterialCustomBufferData() = default;
	};

	struct BufferDesc
	{
		MaterialCustomBufferData* customData;
		U64 size = 0;
		U64 elementsCount = 0;
		Usage usage = Usage::DEFAULT;
		BindFlag bind_flags = BindFlag::NONE;
		BufferType bufferType = BufferType::CUSTOM;
		U32 stride = 0;
		Format format = Format::UNKNOWN;
		bool persistentMapped = false;
		bool exclusiveBuffer = true;
		void* initData = nullptr;
	};

	struct IDrawDebugImGuiWindow {
		virtual void DrawDebugWindow() = 0;
	};

	struct MultipleBuffer
	{
		BufferDesc desc;
		BufferHandle handle;
		U32 currentOffset;
		U32 parts;

		BufferHandle AddNewBufferPart(const BufferDesc& descIn)
		{
			CZUCH_BE_ASSERT(descIn.size > 0, "Buffer size must be greater than 0");
			CZUCH_BE_ASSERT(descIn.bind_flags != BindFlag::NONE, "Buffer must have at least one bind flag set");
			CZUCH_BE_ASSERT(descIn.usage != Usage::DEFAULT, "Buffer usage must be specified");

			if (currentOffset + descIn.size > desc.size)
			{
				CZUCH_BE_ASSERT(false, "Not enough space in buffer to add new part");
				return { Invalid_Handle_Id, 0 };
			}

			BufferHandle newHandle = { handle.handle, currentOffset };
			currentOffset += descIn.size;
			parts++;
			return newHandle;
		}

		bool ReleasePart(BufferHandle& handle)
		{
			CZUCH_BE_ASSERT(handle.handle == this->handle.handle, "Buffer handle mismatch");
			CZUCH_BE_ASSERT(handle.offset < currentOffset, "Buffer offset out of range");
			CZUCH_BE_ASSERT(parts > 0, "No parts to release");
			parts--;
			return parts <= 0;
		}
	};

	struct MultiplerBufferContainer : public IDrawDebugImGuiWindow
	{
		Array<MultipleBuffer> multipleBuffers;
		BufferHandle GetBuffer(const BufferDesc& desc);
		bool Release(BufferHandle handle);
		BufferHandle CreateBuffer(const BufferDesc& desc, GraphicsDevice* device, U32 capacity);
		void DrawDebugWindow();
	};


	struct DescriptorSetLayoutDesc
	{
		struct Binding
		{
			StringID bindingName;
			DescriptorType type = DescriptorType::UNIFORM_BUFFER;
			DescriptorBindingTagType tag;
			U32 size = 0;
			U16 index = 0;
			U16 count = 0;
			bool internalParam = false;
		};

		Binding bindings[s_max_descriptors_per_set];
		UBOLayout uboLayout;
		U32 bindingsCount = 0;
		U32 setIndex = 0;
		U32 shaderStage;

		DescriptorSetLayoutDesc()
		{
			shaderStage = (U32)ShaderStage::ALL;
			Reset();
		}

		Binding* GetBindingWithTag(DescriptorBindingTagType tag);

		DescriptorSetLayoutDesc& Reset();
		DescriptorSetLayoutDesc& AddBinding(CzuchStr name, DescriptorType type, U32 bindingIndex, U32 count, U32 size, bool internalParam, DescriptorBindingTagType tagType = DescriptorBindingTagType::NONE);
		DescriptorSetLayoutDesc& SetUBOLayout(UBOLayout& layout);
		UBOLayout* GetUBOLayoutForBinding(const StringID& name);
	};


	struct Buffer;
	struct DescriptorSetLayout;
	struct ShaderParamsSet
	{
		struct ShaderParamInfo
		{
			StringID paramName;
			DescriptorBindingTagType tag;
			MaterialCustomBufferData* customData;
			I32 resource;
			I32 assetHandle;
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
		ShaderParamsSet& AddBuffer(CzuchStr name, BufferHandle buffer, U16 binding);
		ShaderParamsSet& AddSampler(CzuchStr name, TextureHandle color_texture, U16 binding);
		ShaderParamsSet& AddStorageBuffer(CzuchStr name, BufferHandle buffer, U16 binding, DescriptorBindingTagType tag);
		ShaderParamsSet& AddStorageBufferWithData(CzuchStr name, MaterialCustomBufferData* customData, U16 binding, DescriptorBindingTagType tag);
		void SetSampler(int descriptor, TextureHandle color_texture);
		bool TrySetSampler(StringID& name, TextureHandle texture);
		bool TrySetBuffer(StringID& name, BufferHandle buffer);
		TextureHandle GetTextureHandleForName(StringID& name);
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
			blendSettings.dstColorBlendFactor = Blend::ONE;
			blendSettings.colorBlendOp = BlendOp::ADD;
			blendSettings.srcAlphaBlendFactor = Blend::ONE;
			blendSettings.dstAlphaBlendFactor = Blend::ONE;
			blendSettings.alphaBlendOp = BlendOp::ADD;
		}

		void SetAlphaBlend()
		{
			blendSettings.colorWriteMask = ColorWrite::ENABLE_ALL;
			blendSettings.blendEnable = true;
			blendSettings.srcColorBlendFactor = Blend::SRC_ALPHA;
			blendSettings.dstColorBlendFactor = Blend::ONE_MINUS_SRC_ALPHA;
			blendSettings.colorBlendOp = BlendOp::ADD;
			blendSettings.srcAlphaBlendFactor = Blend::SRC_ALPHA;
			blendSettings.dstAlphaBlendFactor = Blend::ONE_MINUS_SRC_ALPHA;
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

	struct CameraPlanesData
	{
		float nearPlane = 0.1f;
		float farPlane = 1000.0f;
	};


	struct MaterialInstanceDesc;
	struct MaterialInstanceParams;
	struct MaterialPassDesc
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
		RenderPassType passType = RenderPassType::MainForward;

		DescriptorSetLayoutDesc::Binding* GetBindingWithTag(DescriptorBindingTagType tag)
		{
			for (int a = 0; a < layoutsCount; ++a)
			{
				auto binding = layouts[a].GetBindingWithTag(tag);
				if (binding != nullptr)
				{
					return binding;
				}
			}
			return nullptr;
		}

		bool IsTransparent() const
		{
			return bs.blendSettings.blendEnable;
		}

		void AddLayout(DescriptorSetLayoutDesc& layout)
		{
			if (layoutsCount >= k_max_descriptor_set_layouts)
			{
				return;
			}
			layouts[layoutsCount++] = std::move(layout);
		}


		void SetParams(MaterialInstanceDesc& desc, MaterialInstanceParams& params);


		MaterialPassDesc() = default;

		MaterialPassDesc(MaterialPassDesc&& other) noexcept
		{
			*this = std::move(other);
		}

		MaterialPassDesc(const MaterialPassDesc& other) noexcept
		{
			*this = other;
		}


		MaterialPassDesc& operator=(MaterialPassDesc&& other) noexcept
		{
			if (&other != this)
			{
				this->vs = std::move(other.vs);
				this->ps = std::move(other.ps);
				this->bs = std::move(other.bs);
				this->rs = std::move(other.rs);
				this->dss = std::move(other.dss);
				this->il = std::move(other.il);
				this->pt = std::move(other.pt);
				this->layoutsCount = std::move(other.layoutsCount);
				this->passType = std::move(other.passType);

				for (int a = 0; a < this->layoutsCount; ++a)
				{
					this->layouts[a] = std::move(other.layouts[a]);
				}

				this->bindPoint = std::move(other.bindPoint);
			}
			return *this;
		}

		MaterialPassDesc& operator=(const MaterialPassDesc& other) noexcept
		{
			if (&other != this)
			{
				this->vs = (other.vs);
				this->ps = (other.ps);
				this->bs = (other.bs);
				this->rs = (other.rs);
				this->dss = (other.dss);
				this->il = (other.il);
				this->pt = (other.pt);
				this->layoutsCount = (other.layoutsCount);
				this->passType = (other.passType);

				for (int a = 0; a < this->layoutsCount; ++a)
				{
					this->layouts[a] = (other.layouts[a]);
				}

				this->bindPoint = (other.bindPoint);
			}
			return *this;
		}
	};

	struct CZUCH_API MaterialDefinitionPassesContainer
	{
		Array<MaterialPassDesc> passes;

		MaterialDefinitionPassesContainer() = default;

		MaterialDefinitionPassesContainer& operator=(MaterialDefinitionPassesContainer&& other) noexcept
		{
			if (&other != this)
			{
				this->passes = std::move(other.passes);
			}
			return *this;
		}

		MaterialDefinitionPassesContainer& operator=(const MaterialDefinitionPassesContainer& other) noexcept
		{
			if (&other != this)
			{
				for (int a = 0; a < passes.size(); ++a)
				{
					this->passes[a] = other.passes[a];
				}
			}
			return *this;
		}

		bool IsTransparent() const
		{
			for (int a = 0; a < passes.size(); ++a)
			{
				if (passes[a].IsTransparent())
				{
					return true;
				}
			}
			return false;
		}

		void EmplacePass(MaterialPassDesc& state)
		{
			passes.emplace_back(std::move(state));
		}

		void AddLayout(RenderPassType type, DescriptorSetLayoutDesc layout)
		{
			for (auto& state : passes)
			{
				if (state.passType == type)
				{
					state.AddLayout(layout);
					return;
				}
			}
		}

		void AddLayout(int passIndex, DescriptorSetLayoutDesc layout)
		{
			if (passIndex < passes.size())
			{
				passes[passIndex].AddLayout(layout);
			}
		}

		bool HasPassWithType(RenderPassType type)
		{
			for (auto& pass : passes)
			{
				if (pass.passType == type)
				{
					return true;
				}
			}
			return false;
		}


		DescriptorSetLayoutDesc::Binding* GetBindingWithTag(DescriptorBindingTagType tag) {
			for (int a = 0; a < passes.size(); ++a)
			{
				auto binding = passes[a].GetBindingWithTag(tag);
				if (binding != nullptr)
				{
					return binding;
				}
			}
			return nullptr;
		}


	};

	struct CZUCH_API MaterialDefinitionDesc
	{
		MaterialDefinitionPassesContainer passesContainer;
		CzuchStr materialName;
		int materialFlags;

		MaterialDefinitionDesc(U32 passesCount)
		{
			materialFlags = 0;
			passesContainer.passes.reserve(passesCount);
		}

		MaterialDefinitionDesc()
		{
			materialFlags = 0;
		}

		MaterialDefinitionDesc(MaterialDefinitionDesc& other) noexcept
		{
			*this = std::move(other);
		}

		bool IsTransparent() const
		{
			return passesContainer.IsTransparent();
		}

		MaterialDefinitionDesc& operator=(MaterialDefinitionDesc&& other) noexcept
		{
			if (&other != this)
			{
				this->materialName = std::move(other.materialName);
				this->passesContainer = std::move(other.passesContainer);
				this->materialFlags = std::move(other.materialFlags);
			}
			return *this;
		}

		MaterialDefinitionDesc& operator=(MaterialDefinitionDesc& other) noexcept
		{
			if (&other != this)
			{
				this->materialFlags = other.materialFlags;
				this->passesContainer = other.passesContainer;
			}
			return *this;
		}

		void EmplacePass(MaterialPassDesc& state)
		{
			passesContainer.EmplacePass(state);
		}

		void AddLayout(RenderPassType type, DescriptorSetLayoutDesc layout)
		{
			passesContainer.AddLayout(type, layout);
		}

		void AddLayout(int passIndex, DescriptorSetLayoutDesc layout)
		{
			passesContainer.AddLayout(passIndex, layout);
		}

		U32 PassesCount() const
		{
			return passesContainer.passes.size();
		}

		const MaterialPassDesc& GetMaterialPassDescAt(U32 index) const
		{
			return passesContainer.passes[index];
		}

		UBOLayout* GetUBOLayoutForName(const StringID& name)
		{
			for (U32 i = 0; i < PassesCount(); i++)
			{
				for (U32 j = 0; j < passesContainer.passes[i].layoutsCount; j++)
				{
					auto layout = passesContainer.passes[i].layouts[j].GetUBOLayoutForBinding(name);
					if (layout)
					{
						return layout;
					}
				}
			}
			CZUCH_BE_ASSERT(false, "UBO layout not found");
			return nullptr;
		}
	};

	enum CZUCH_API MaterialParamType
	{
		PARAM_TEXTURE,
		PARAM_BUFFER,
		PARAM_UNKNOWN
	};

	struct CZUCH_API MaterialInstanceDesc
	{
		struct ShaderParamDesc
		{
			CzuchStr name;
			MaterialCustomBufferData data;
			DescriptorType type;
			I32 resourceAsset;
			I32 resource;
			bool isInternal = false;
		};

		AssetHandle materialAsset;
		bool isTransparent = false;
		Array<ShaderParamDesc> paramsDesc;
		MaterialInstanceDesc()
		{
			Reset();
		}

		void GetAllTexturesDependencies(Array<TextureHandle>& dependencies);

		MaterialInstanceDesc& Reset();
		MaterialInstanceDesc& AddStorageBufferSingleData(const CzuchStr& name, MaterialCustomBufferData&& data);
		MaterialInstanceDesc& AddBuffer(const CzuchStr& name, MaterialCustomBufferData&& data);
		MaterialInstanceDesc& AddBuffer(const CzuchStr& name, BufferHandle buffer);
		MaterialInstanceDesc& AddStorageBuffer(const CzuchStr& name, BufferHandle buffer);
		MaterialInstanceDesc& AddSampler(const CzuchStr& name, TextureHandle color_texture, bool isInternal);

		void SetTransparent(bool value) { isTransparent = value; }

		MaterialInstanceDesc Clone();
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
		MaterialInstanceParams& AddBuffer(int set, const CzuchStr& name, BufferHandle buffer, U16 binding);
		MaterialInstanceParams& AddStorageBuffer(int set, const CzuchStr& name, BufferHandle buffer, U16 binding, DescriptorBindingTagType tag);
		MaterialInstanceParams& AddStorageBufferWithData(int set,const CzuchStr& name, MaterialCustomBufferData* customData, U16 binding, DescriptorBindingTagType tag);
		MaterialInstanceParams& AddSampler(int set, const CzuchStr& name, TextureHandle color_texture, U16 binding);

		void SetSampler(int set, TextureHandle color_texture, int descriptor);
		void SetSampler(StringID& name, TextureHandle texture);
		void SetUniformBuffer(StringID& name, BufferHandle buffer);
		void SetUniformBuffer(int set, BufferHandle buffer,int descriptor);
		void SetStorageBuffer(int set, BufferHandle buffer, int descriptor, DescriptorBindingTagType tag = DescriptorBindingTagType::NONE);
		void SetStorageBufferWithData(int set, MaterialCustomBufferData* customData, int descriptor, DescriptorBindingTagType tag = DescriptorBindingTagType::NONE);
		TextureHandle GetTextureHandleForName(StringID& name);
	};

	struct SamplerDesc
	{
		TextureAddressMode addressModeU = TextureAddressMode::WRAP;
		TextureAddressMode addressModeV = TextureAddressMode::WRAP;
		TextureAddressMode addressModeW = TextureAddressMode::WRAP;
		TextureFilter magFilter = TextureFilter::LINEAR;
		TextureFilter minFilter = TextureFilter::LINEAR;
		float minMipLevel = 0.0f;
		float maxMipLevel = 0.0f;
		bool anisoEnabled = false;
	};

	struct TextureDesc
	{
		SamplerDesc samplerDesc;
		ImageLayouInfo layoutInfo;
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
		ImageUsageFlag usageFlags = ImageUsageFlag::SAMPLED | ImageUsageFlag::TRANSFER_DST;
		ImageAspectFlag aspectFlags = ImageAspectFlag::COLOR;
		Swizzle swizzle;
		U8* texData;
		const char* name = nullptr;

		inline U32 GetSize() const
		{
			return width * height * 4;
		}
	};

	struct MaterialInfo
	{
		AssetHandle materialAsset;
		MaterialInstanceHandle materialInstanceHandle;

		MaterialInfo()
		{
			materialAsset = INVALID_HANDLE(AssetHandle);
			materialInstanceHandle = INVALID_HANDLE(MaterialInstanceHandle);
		}
	};

	struct MeshData
	{
		Array<Vec3> positions;
		Array<Vec3> normals;
		Array<Vec4> colors;
		Array<Vec4> uvs0;
		Array<U32> indices;
		AABB aabb;
		AssetHandle materialInstanceAssetHandle;
		CzuchStr meshName;

	public:

		void ComputeAABB();

		MeshData()
		{
			materialInstanceAssetHandle = INVALID_HANDLE(AssetHandle);
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
				this->materialInstanceAssetHandle = other.materialInstanceAssetHandle;
				this->meshName = std::move(other.meshName);
				this->indices = std::move(other.indices);
				INVALIDATE_HANDLE(other.materialInstanceAssetHandle);
			}
			return *this;
		}

		MeshData& operator=(MeshData& other) noexcept
		{
			if (&other != this)
			{
				this->positions = other.positions;
				this->normals = other.normals;
				this->colors = other.colors;
				this->uvs0 = other.uvs0;
				this->materialInstanceAssetHandle = other.materialInstanceAssetHandle;
				this->meshName = other.meshName;
				this->indices = other.indices;
			}
			return *this;
		}

		void Reserve(U32 count)
		{
			positions.reserve(count);
			normals.reserve(count);
			colors.reserve(count);
			uvs0.reserve(count);
		}
	};

	struct Mesh : GraphicsDeviceResource
	{
		MeshData* data;

		inline bool HasNormals() const { return data->normals.size() > 0; }
		inline bool HasColors() const { return data->colors.size() > 0; }
		inline bool HasUV0() const { return data->uvs0.size() > 0; }

		constexpr const MeshData& GetMeshData() const { return *data; }
		bool IsValid() const { return data != nullptr; }

		BufferHandle positionsHandle;
		BufferHandle normalsHandle;
		BufferHandle colorsHandle;
		BufferHandle uvs0Handle;
		BufferHandle indicesHandle;
		MaterialInstanceHandle materialHandle;

		Mesh()
		{
			data = nullptr;
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
		NONE = 0,
		USE_SCENE_DATA = 1 << 0
	};

	struct Material : public GraphicsDeviceResource
	{
		MaterialDefinitionDesc* desc;
		constexpr const MaterialDefinitionDesc& GetDesc() const { return *desc; }

		Array<PipelineHandle> pipelines;

		I32 GetRenderPassIndexForType(RenderPassType type) const;
		DescriptorSetLayoutDesc::Binding* GetBindingWithTag(DescriptorBindingTagType tag);
	};


	struct StorageBufferTagInfo
	{
		DescriptorBindingTagType tag;
		I32 descriptorIndex;
		I32 index;
	};

	struct MaterialInstance : public GraphicsDeviceResource
	{
		MaterialInstanceDesc* desc;
		MaterialInstanceParams params[k_max_render_passes];
		MaterialHandle handle;
		U32 passesCount = 0;
		bool IsTransparent() const { return desc->isTransparent; }
		constexpr const MaterialInstanceDesc& GetDesc() const { return *desc; }
		void SetSampler(StringID& name, TextureHandle texture);
		void SetUniformBuffer(StringID& name, BufferHandle buffer);
		TextureHandle GetTextureHandleForName(StringID& name);

		MaterialCustomBufferData* GetDataForTag(DescriptorBindingTagType tago, int pass);

		I32 UpdateCustomDataWithTag(DescriptorBindingTagType tag, void* data, U32 size,int pass);
		I32 GetIndexForInternalBufferForTag(DescriptorBindingTagType tag, int pass) const;
		void SetIndexAndBufferForInternalBufferForTag(DescriptorBindingTagType tag, I32 index, BufferHandle buffer, int pass);

		StorageBufferTagInfo GetInfoForDescriptorTag(DescriptorBindingTagType tag, int pass);

	};

	struct Pipeline : public GraphicsDeviceResource
	{
		MaterialPassDesc m_desc{};
		constexpr const MaterialPassDesc& GetDesc() const { return m_desc; }

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

namespace std {
	// Refer to Czuch::RenderPassDesc explicitly
	template <>
	struct hash<Czuch::RenderPassDesc> {
		size_t operator()(const Czuch::RenderPassDesc& rpDesc) const {
			return rpDesc.GetHash(); // Calls the const GetHash() method of Czuch::RenderPassDesc
		}
	};
}
