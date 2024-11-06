#include"czpch.h"
#include "Graphics.h"
#include "GraphicsDevice.h"

namespace Czuch
{
	DescriptorSetLayoutDesc& DescriptorSetLayoutDesc::Reset()
	{
		bindingsCount = 0;
		setIndex = 0;
		return *this;
	}
	DescriptorSetLayoutDesc& DescriptorSetLayoutDesc::AddBinding(CzuchStr name, DescriptorType type, U32 bindingIndex, U32 count, U32 size, bool internalParam)
	{
		bindings[bindingsCount++] = { StringID::MakeStringID(name),type,size,(U16)bindingIndex, (U16)count,internalParam };
		return *this;
	}


	bool IsDepthFormat(Format format)
	{
		switch (format)
		{
		case Format::D32_FLOAT:
		case Format::D24_UNORM_S8_UINT:
		case Format::D32_FLOAT_S8X24_UINT:
		case Format::D16_UNORM:
			return true;
		default:
			return false;
		}
	}


	bool IsDepthFormatWithStencil(Format format)
	{
		switch (format)
		{
		case Format::D24_UNORM_S8_UINT:
		case Format::D32_FLOAT_S8X24_UINT:
			return true;
		default:
			return false;
		}
	}

	ShaderStage StringToShaderStage(const CzuchStr& stage)
	{
		if (stage == "vert")
		{
			return ShaderStage::VS;
		}
		else if (stage == "frag")
		{
			return ShaderStage::PS;
		}
		return ShaderStage::VS;
	}

	void InputVertexLayout::Reset()
	{
		vertexStreamsCount = 0;
		vertexAttributesCount = 0;
	}
	void InputVertexLayout::AddAttribute(const VertexAttribute& attribute)
	{
		if (vertexAttributesCount < s_max_vertex_attributes)
		{
			attributes[vertexAttributesCount++] = attribute;
		}
	}

	void InputVertexLayout::AddStream(const VertexStream& stream)
	{
		if (vertexStreamsCount < s_max_vertex_streams)
		{
			streams[vertexStreamsCount++] = stream;
		}
	}

	ShaderParamsSet& ShaderParamsSet::Reset()
	{
		descriptorsCount = 0;
		return *this;
	}

	ShaderParamsSet& ShaderParamsSet::AddBuffer(CzuchStr name, BufferHandle buffer, U16 binding)
	{
		if (descriptorsCount >= s_max_descriptors_per_set)
		{
			return *this;
		}
		descriptors[descriptorsCount].paramName = StringID::MakeStringID(name);
		descriptors[descriptorsCount].binding = binding;
		descriptors[descriptorsCount].resource = buffer.handle;
		descriptors[descriptorsCount++].type = DescriptorType::UNIFORM_BUFFER;

		return *this;
	}


	ShaderParamsSet& ShaderParamsSet::AddSampler(CzuchStr name, TextureHandle color_texture, U16 binding)
	{
		if (descriptorsCount >= s_max_descriptors_per_set)
		{
			return *this;
		}

		descriptors[descriptorsCount].paramName = StringID::MakeStringID(name);
		descriptors[descriptorsCount].binding = binding;
		descriptors[descriptorsCount].resource = color_texture.handle;
		descriptors[descriptorsCount++].type = DescriptorType::SAMPLER;

		return *this;
	}


	Mesh::~Mesh()
	{
		if (HANDLE_IS_VALID(positionsHandle))
		{
			device->Release(positionsHandle);
		}

		if (HANDLE_IS_VALID(normalsHandle))
		{
			device->Release(normalsHandle);
		}

		if (HANDLE_IS_VALID(colorsHandle))
		{
			device->Release(colorsHandle);
		}

		if (HANDLE_IS_VALID(uvs0Handle))
		{
			device->Release(uvs0Handle);
		}

		if (HANDLE_IS_VALID(indicesHandle))
		{
			device->Release(indicesHandle);
		}
	}

	MaterialInstanceParams& MaterialInstanceParams::Reset()
	{
		for (int a = 0; a < setsCount; a++)
		{
			shaderParamsDesc[a].Reset();
		}
		setsCount = 0;
		return *this;
	}
	MaterialInstanceParams& MaterialInstanceParams::AddBuffer(int set, CzuchStr& name, BufferHandle buffer, U16 binding)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return *this;
		}
		setsCount = std::max(set, this->setsCount);

		shaderParamsDesc[set].AddBuffer(name, buffer, binding);
		return *this;

	}
	MaterialInstanceParams& MaterialInstanceParams::AddSampler(int set, CzuchStr& name, TextureHandle color_texture, U16 binding)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return *this;
		}
		setsCount = std::max(set, this->setsCount);

		shaderParamsDesc[set].AddSampler(name, color_texture, binding);
		return *this;
	}

	MaterialInstanceDesc& MaterialInstanceDesc::Reset()
	{
		paramsDesc.clear();
		INVALIDATE_HANDLE(materialAsset);
		return *this;
	}
	MaterialInstanceDesc& MaterialInstanceDesc::AddBuffer(const CzuchStr& name, BufferHandle buffer)
	{
		paramsDesc.push_back({ .name = name,.type = DescriptorType::UNIFORM_BUFFER,.resource = buffer.handle });
		return *this;
	}
	MaterialInstanceDesc& MaterialInstanceDesc::AddSampler(const CzuchStr& name, TextureHandle color_texture)
	{
		paramsDesc.push_back({ .name = name,.type = DescriptorType::SAMPLER,.resource = color_texture.handle });
		return *this;
	}

	void PipelineStateDesc::SetParams(MaterialInstanceDesc& desc, MaterialInstanceParams& params)
	{
		for (int a = 0; a < desc.paramsDesc.size(); ++a)
		{
			auto& param = desc.paramsDesc[a];
			StringID nameId = StringID::MakeStringID(param.name);
			for (int i = 0; i < layoutsCount; ++i)
			{
				auto& current = layouts[i];
				for (int b = 0; b < current.bindingsCount; ++b)
				{
					auto& binding = current.bindings[b];
					if (binding.bindingName.Compare(nameId) == 0)
					{
						if (param.type == DescriptorType::SAMPLER)
						{
							params.AddSampler(i, param.name, TextureHandle(param.resource), b);
						}
						else if (param.type == DescriptorType::UNIFORM_BUFFER)
						{
							params.AddBuffer(i, param.name, BufferHandle(param.resource), b);
						}
					}
				}
			}
		}
	}

	RenderPassDesc& RenderPassDesc::AddAttachment(Format format, ImageLayout layout, AttachmentLoadOp loadOp)
	{
		if (attachmentsCount < k_max_image_outputs)
		{
			colorAttachments[attachmentsCount++] = { format,layout,loadOp };
			return *this;
		}
		LOG_BE_ERROR("[VulkanRenderPassDesc]Too many color attachments for render pass with name {0}",name);
		return *this;
	}

	RenderPassDesc& RenderPassDesc::SetDepthStencilTexture(Format format, ImageLayout layout)
	{
		depthStencilFinalLayout = layout;
		depthStencilFormat = format;
		return *this;
	}
	RenderPassDesc& RenderPassDesc::SetName(const char* rpName)
	{
		name = rpName;
		return *this;
	}
	RenderPassDesc& RenderPassDesc::SetDepthAndStencilLoadOp(AttachmentLoadOp depth, AttachmentLoadOp stencil)
	{
		depthLoadOp = depth;
		stencilLoadOp = stencil;
		return *this;
	}
	FrameBufferDesc& FrameBufferDesc::AddRenderTexture(TextureHandle texture)
	{
		if (renderTargetsCount < k_max_image_outputs)
		{
			renderTextures[renderTargetsCount++] = texture;
			return *this;
		}
		LOG_BE_ERROR("[VulkanFrameBufferDesc]Too many render targets for frame buffer with name {0}", name);
		return *this;
	}

	FrameBufferDesc& FrameBufferDesc::SetDepthStencilTexture(TextureHandle texture)
	{
		depthStencilTexture = texture;
		return *this;
	}

	FrameBufferDesc& FrameBufferDesc::SetName(const char* newName)
	{
		name = newName;
		return *this;
	}

}