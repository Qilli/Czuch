#include"czpch.h"
#include "Graphics.h"
#include "GraphicsDevice.h"

namespace Czuch
{
	DescriptorSetLayoutDesc::Binding* DescriptorSetLayoutDesc::GetBindingWithTag(DescriptorBindingTagType tag)
	{
		for (auto& binding : bindings)
		{
			if (binding.tag == tag)
			{
				return &binding;
			}
		}
		return nullptr;
	}

	DescriptorSetLayoutDesc& DescriptorSetLayoutDesc::Reset()
	{
		bindingsCount = 0;
		setIndex = 0;
		return *this;
	}
	DescriptorSetLayoutDesc& DescriptorSetLayoutDesc::AddBinding(CzuchStr name, DescriptorType type, U32 bindingIndex, U32 count, U32 size, bool internalParam, DescriptorBindingTagType tagType)
	{
		bindings[bindingsCount++] = { StringID::MakeStringID(name),type,tagType,size,(U16)bindingIndex, (U16)count,internalParam };
		return *this;
	}

	DescriptorSetLayoutDesc& DescriptorSetLayoutDesc::SetUBOLayout(UBOLayout& layout)
	{
		uboLayout = std::move(layout);
		return *this;
	}

	UBOLayout* DescriptorSetLayoutDesc::GetUBOLayoutForBinding(const StringID& name)
	{
		for (int a = 0; a < bindingsCount; ++a)
		{
			if (bindings[a].bindingName.GetGuid() == name.GetGuid())
			{
				return &uboLayout;
			}
		}
		return nullptr;
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
		descriptors[descriptorsCount].assetHandle = color_texture.assetHandle.handle;
		descriptors[descriptorsCount++].type = DescriptorType::SAMPLER;

		return *this;
	}

	void ShaderParamsSet::SetSampler(int descriptor, TextureHandle color_texture)
	{
		if (descriptor < descriptorsCount && descriptors[descriptor].type == DescriptorType::SAMPLER)
		{
			descriptors[descriptor].resource = color_texture.handle;
		}
	}

	bool ShaderParamsSet::TrySetSampler(StringID& name, TextureHandle texture)
	{
		for (int a = 0; a < descriptorsCount; ++a)
		{
			if (descriptors[a].paramName.Compare(name) == 0)
			{
				descriptors[a].resource = texture.handle;
				descriptors[a].assetHandle = texture.assetHandle.handle;
				return true;
			}
		}
		return false;
	}

	bool ShaderParamsSet::TrySetBuffer(StringID& name, BufferHandle buffer)
	{
		for (int a = 0; a < descriptorsCount; ++a)
		{
			if (descriptors[a].paramName.Compare(name) == 0)
			{
				descriptors[a].resource = buffer.handle;
				return true;
			}
		}
		return false;
	}

	TextureHandle ShaderParamsSet::GetTextureHandleForName(StringID& name)
	{
		for (int a = 0; a < descriptorsCount; ++a)
		{
			if (descriptors[a].paramName.Compare(name) == 0)
			{
				return TextureHandle(descriptors[a].resource, descriptors[a].assetHandle);
			}
		}
		return TextureHandle();
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
		setsCount = std::max(set + 1, this->setsCount);

		shaderParamsDesc[set].AddBuffer(name, buffer, binding);
		return *this;

	}
	MaterialInstanceParams& MaterialInstanceParams::AddSampler(int set, CzuchStr& name, TextureHandle color_texture, U16 binding)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return *this;
		}
		setsCount = std::max(set + 1, this->setsCount);

		shaderParamsDesc[set].AddSampler(name, color_texture, binding);
		return *this;
	}

	void MaterialInstanceParams::SetSampler(int set, TextureHandle color_texture)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return;
		}
		shaderParamsDesc[set].SetSampler(0, color_texture);
	}

	void MaterialInstanceParams::SetSampler(StringID& name, TextureHandle texture)
	{
		for (int a = 0; a < setsCount; ++a)
		{
			if (shaderParamsDesc[a].TrySetSampler(name, texture) == true)
			{
				return;
			}
		}
	}

	void MaterialInstanceParams::SetUniformBuffer(StringID& name, BufferHandle buffer)
	{
		for (int a = 0; a < setsCount; ++a)
		{
			if (shaderParamsDesc[a].TrySetBuffer(name, buffer))
			{
				return;
			}
		}
	}

	TextureHandle MaterialInstanceParams::GetTextureHandleForName(StringID& name)
	{
		for (int a = 0; a < setsCount; ++a)
		{
			auto handle = shaderParamsDesc[a].GetTextureHandleForName(name);
			if (HANDLE_IS_VALID(handle))
			{
				return handle;
			}
		}
		return TextureHandle();
	}


	void MaterialInstanceDesc::GetAllTexturesDependencies(Array<TextureHandle>& dependencies)
	{
		for (auto& param : paramsDesc)
		{
			if (param.type == DescriptorType::SAMPLER)
			{
				dependencies.push_back(TextureHandle(param.resource, param.resourceAsset));
			}
		}
	}

	MaterialInstanceDesc& MaterialInstanceDesc::Reset()
	{
		paramsDesc.clear();
		INVALIDATE_HANDLE(materialAsset);
		return *this;
	}
	MaterialInstanceDesc& MaterialInstanceDesc::AddBuffer(const CzuchStr& name, UBO&& data)
	{
		paramsDesc.push_back({ .name = name,.uboData = std::move(data),.type = DescriptorType::UNIFORM_BUFFER,.resource = Invalid_Handle_Id,.isInternal = false });
		return *this;
	}
	MaterialInstanceDesc& MaterialInstanceDesc::AddBuffer(const CzuchStr& name, BufferHandle handle)
	{
		paramsDesc.push_back({ .name = name,.uboData = UBO(),.type = DescriptorType::UNIFORM_BUFFER,.resource = handle.handle,.isInternal = true });
		return *this;
	}
	MaterialInstanceDesc& MaterialInstanceDesc::AddStorageBuffer(const CzuchStr& name, BufferHandle handle)
	{
		paramsDesc.push_back({ .name = name,.uboData = UBO(),.type = DescriptorType::STORAGE_BUFFER,.resource = handle.handle,.isInternal = true });
		return *this;
	}
	MaterialInstanceDesc& MaterialInstanceDesc::AddSampler(const CzuchStr& name, TextureHandle color_texture, bool isInternal)
	{
		paramsDesc.push_back({ .name = name,.uboData = UBO(),.type = DescriptorType::SAMPLER,.resourceAsset = color_texture.assetHandle.handle,.resource = color_texture.handle,.isInternal = isInternal });
		return *this;
	}

	MaterialInstanceDesc MaterialInstanceDesc::Clone()
	{
		MaterialInstanceDesc desc;
		for (int a = 0; a < paramsDesc.size(); a++)
		{
			auto& param = paramsDesc[a];
			if (param.type == DescriptorType::SAMPLER)
			{
				desc.AddSampler(param.name, TextureHandle(param.resource, param.resourceAsset), param.isInternal);
			}
			else if (param.type == DescriptorType::UNIFORM_BUFFER)
			{
				UBO uboCopy(param.uboData.GetData(), param.uboData.GetSize());
				desc.AddBuffer(param.name, std::move(uboCopy));
			}
		}
		desc.isTransparent = isTransparent;
		desc.materialAsset = materialAsset;
		return desc;
	}

	void MaterialPassDesc::SetParams(MaterialInstanceDesc& desc, MaterialInstanceParams& params)
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
							TextureHandle texture = { param.resource,param.resourceAsset };
							params.AddSampler(i, param.name, texture, b);
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
		LOG_BE_ERROR("[VulkanRenderPassDesc]Too many color attachments for render pass with name {0}", name);
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


	I32 Material::GetRenderPassIndexForType(RenderPassType type) const
	{
		for (int a = 0; a < desc->PassesCount(); ++a)
		{
			auto& pass = desc->GetMaterialPassDescAt(a);
			if (pass.passType == type)
			{
				return a;
			}
		}
		return -1;
	}

	DescriptorSetLayoutDesc::Binding* Material::GetBindingWithTag(DescriptorBindingTagType tag)
	{
		for (int a = 0; a < desc->passesContainer.passes.size(); ++a)
		{
			auto* binding = desc->passesContainer.passes[a].GetBindingWithTag(tag);
			if (binding != nullptr)
			{
				return binding;
			}
		}
		return nullptr;
	}


	void MaterialInstance::SetSampler(StringID& name, TextureHandle texture)
	{
		for (int a = 0; a < passesCount; ++a)
		{
			params[a].SetSampler(name, texture);
		}
	}

	void MaterialInstance::SetUniformBuffer(StringID& name, BufferHandle buffer)
	{
		for (int a = 0; a < passesCount; ++a)
		{
			params[a].SetUniformBuffer(name, buffer);
		}
	}

	TextureHandle MaterialInstance::GetTextureHandleForName(StringID& name)
	{
		for (int a = 0; a < passesCount; ++a)
		{
			auto handle = params[a].GetTextureHandleForName(name);
			if (HANDLE_IS_VALID(handle))
			{
				return handle;
			}
		}
	}


	void MeshData::ComputeAABB()
	{
		//compute aabb from positions
		aabb.min = Vec3(FLT_MAX);
		aabb.max = Vec3(-FLT_MAX);
		for (int a = 0; a < positions.size(); ++a)
		{
			aabb.min = glm::min(aabb.min, positions[a]);
			aabb.max = glm::max(aabb.max, positions[a]);
		}
	}

}