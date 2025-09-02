#include"czpch.h"
#include "Graphics.h"
#include "GraphicsDevice.h"
#include"imgui.h"

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
	DescriptorSetLayoutDesc& DescriptorSetLayoutDesc::AddBinding(CzuchStr name, DescriptorType type, U32 bindingIndex, U32 count, U32 size, bool InternalParam, DescriptorBindingTagType tagType)
	{
		bindings[bindingsCount++] = { StringID::MakeStringID(name),type,tagType,size,(U16)bindingIndex, (U16)count,InternalParam };
		return *this;
	}

	DescriptorSetLayoutDesc& DescriptorSetLayoutDesc::SetUBOLayout(UBOLayout& layout)
	{
		uboLayout = std::move(layout);
		return *this;
	}

	UBOLayout* DescriptorSetLayoutDesc::GetUBOLayoutForBinding(const StringID& name)
	{
		for (I32 a = 0; a < bindingsCount; ++a)
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

	ShaderParamsSet& ShaderParamsSet::AddBuffer(CzuchStr name, BufferHandle buffer, U16 binding, I32 size)
	{
		if (descriptorsCount >= s_max_descriptors_per_set)
		{
			return *this;
		}
		descriptors[descriptorsCount].paramName = StringID::MakeStringID(name);
		descriptors[descriptorsCount].binding = binding;
		descriptors[descriptorsCount].resource = buffer.handle;
		descriptors[descriptorsCount].size = size;
		descriptors[descriptorsCount++].type = DescriptorType::UNIFORM_BUFFER;

		return *this;
	}


	ShaderParamsSet& ShaderParamsSet::AddSampler(CzuchStr name, TextureHandle color_texture, U16 binding, DescriptorBindingTagType tag)
	{
		if (descriptorsCount >= s_max_descriptors_per_set)
		{
			return *this;
		}

		descriptors[descriptorsCount].paramName = StringID::MakeStringID(name);
		descriptors[descriptorsCount].binding = binding;
		descriptors[descriptorsCount].resource = color_texture.handle;
		descriptors[descriptorsCount].assetHandle = color_texture.assetHandle.handle;
		descriptors[descriptorsCount].tag = tag;
		descriptors[descriptorsCount++].type = DescriptorType::SAMPLER;


		return *this;
	}

	ShaderParamsSet& ShaderParamsSet::AddStorageBuffer(CzuchStr name, BufferHandle buffer, U16 binding, DescriptorBindingTagType tag, I32 size)
	{
		if (descriptorsCount >= s_max_descriptors_per_set)
		{
			return *this;
		}
		descriptors[descriptorsCount].paramName = StringID::MakeStringID(name);
		descriptors[descriptorsCount].binding = binding;
		descriptors[descriptorsCount].resource = buffer.handle;
		descriptors[descriptorsCount].tag = tag;
		descriptors[descriptorsCount].size = size;
		descriptors[descriptorsCount++].type = DescriptorType::STORAGE_BUFFER;

		return *this;
	}

	ShaderParamsSet& ShaderParamsSet::AddStorageBufferWithData(CzuchStr name, MaterialCustomBufferData* customData, U16 binding, DescriptorBindingTagType tag)
	{
		if (descriptorsCount >= s_max_descriptors_per_set)
		{
			return *this;
		}
		descriptors[descriptorsCount].paramName = StringID::MakeStringID(name);
		descriptors[descriptorsCount].binding = binding;
		descriptors[descriptorsCount].customData = customData;
		descriptors[descriptorsCount].tag = tag;
		descriptors[descriptorsCount].size = customData->GetSize();
		descriptors[descriptorsCount++].type = DescriptorType::STORAGE_BUFFER;

		return *this;
	}

	/*void ShaderParamsSet::CheckForUpdateAndMakeDirty(Czuch::Handle handle, bool forceFullUpdate, I32 descriptor)
	{
		if (updateControl != nullptr && Invalid_Handle_Id != handle)
		{
			if (forceFullUpdate)
			{
				descriptors[descriptor].ClearDirtyAll();
			}
			else
			{
				descriptors[descriptor].SetDirtyExcept(updateControl->GetCurrentFrameIndex());
			}

			updateControl->UpdateDescriptorSet(descriptors[descriptor], forceFullUpdate);
		}
	}

	void ShaderParamsSet::UpdateDirtyParamsForSet()
	{
		/*if (updateControl != nullptr)
		{
			for (I32 i = 0; i < descriptorsCount; ++i)
			{
					if (descriptors[i].isDirty[updateControl->GetCurrentFrameIndex()])
					{
						descriptors[i].isDirty[updateControl->GetCurrentFrameIndex()] = false;
						updateControl->UpdateDescriptorSet(descriptors[i], false);
					}
				
			}
		}
	}*/

	void ShaderParamsSet::SetSampler(I32 descriptor, TextureHandle color_texture)
	{
		if (descriptor < descriptorsCount && descriptors[descriptor].type == DescriptorType::SAMPLER)
		{
			descriptors[descriptor].resource = color_texture.handle;
		}
	}


	bool ShaderParamsSet::TrySetSampler(StringID& name, TextureHandle texture)
	{
		for (I32 a = 0; a < descriptorsCount; ++a)
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
		for (I32 a = 0; a < descriptorsCount; ++a)
		{
			if (descriptors[a].paramName.Compare(name) == 0)
			{
				descriptors[a].resource = buffer.handle;
				return true;
			}
		}
		return false;
	}

	bool ShaderParamsSet::SetBuffer(DescriptorBindingTagType tag, BufferHandle buffer, I32 size)
	{
		for (I32 a = 0; a < descriptorsCount; ++a)
		{
			if (descriptors[a].tag == tag)
			{
				descriptors[a].resource = buffer.handle;
				if (size >= 0)
				{
					descriptors[a].size = size;
				}
				return true;
			}
		}
		return false;
	}

	bool ShaderParamsSet::SetBuffer(U32 descriptor, BufferHandle buffer, I32 size)
	{
		descriptors[descriptor].resource = buffer.handle;
		if (size >= 0)
		{
			descriptors[descriptor].size = size;
		}
		return true;
	}

	bool ShaderParamsSet::SetBufferSize(U32 descriptor, I32 size)
	{
		descriptors[descriptor].size = size;
		return true;
	}

	bool ShaderParamsSet::SetBufferSize(DescriptorBindingTagType tag, I32 size)
	{
		for(I32 a = 0; a < descriptorsCount; ++a)
		{
			if (descriptors[a].tag == tag)
			{
				descriptors[a].size = size;
				return true;
			}
		}
		return false;
	}

	TextureHandle ShaderParamsSet::GetTextureHandleForName(StringID& name)
	{
		for (I32 a = 0; a < descriptorsCount; ++a)
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
		if (HANDLE_IS_VALID(vertexBufferHandle))
		{
			device->Release(vertexBufferHandle);
		}

		if (HANDLE_IS_VALID(indicesHandle))
		{
			device->Release(indicesHandle);
		}
	}

	MaterialInstanceParams& MaterialInstanceParams::Reset()
	{
		for (I32 a = 0; a < setsCount; a++)
		{
			shaderParamsDesc[a].Reset();
		}
		setsCount = 0;
		return *this;
	}
	MaterialInstanceParams& MaterialInstanceParams::AddBuffer(I32 set,const CzuchStr& name, BufferHandle buffer, U16 binding, I32 size)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return *this;
		}
		setsCount = std::max(set + 1, this->setsCount);

		shaderParamsDesc[set].AddBuffer(name, buffer, binding,size);
		return *this;

	}
	MaterialInstanceParams& MaterialInstanceParams::AddStorageBuffer(I32 set,const CzuchStr& name, BufferHandle buffer, U16 binding, DescriptorBindingTagType tag, I32 size)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return *this;
		}
		setsCount = std::max(set + 1, this->setsCount);
		shaderParamsDesc[set].AddStorageBuffer(name, buffer, binding, tag,size);
		return *this;
	}
	MaterialInstanceParams& MaterialInstanceParams::AddStorageBufferWithData(I32 set, const CzuchStr& name, MaterialCustomBufferData* customData, U16 binding, DescriptorBindingTagType tag)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return *this;
		}
		setsCount = std::max(set + 1, this->setsCount);
		shaderParamsDesc[set].AddStorageBufferWithData(name, customData, binding, tag);
		return *this;
	}
	MaterialInstanceParams& MaterialInstanceParams::AddSampler(I32 set, const CzuchStr& name, TextureHandle color_texture, U16 binding)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return *this;
		}
		setsCount = std::max(set + 1, this->setsCount);

		shaderParamsDesc[set].AddSampler(name, color_texture, binding,DescriptorBindingTagType::NONE);
		return *this;
	}

	void MaterialInstanceParams::SetAsTextureBindlessSet(int set)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return;
		}
		shaderParamsDesc[set].isBindlessTexturesSet = true;
	}

	void MaterialInstanceParams::SetSampler(I32 set, TextureHandle color_texture, I32 descriptor)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return;
		}
		shaderParamsDesc[set].SetSampler(descriptor, color_texture);
	}

	void MaterialInstanceParams::SetSampler(StringID& name, TextureHandle texture)
	{
		for (I32 a = 0; a < setsCount; ++a)
		{
			if (shaderParamsDesc[a].TrySetSampler(name, texture) == true)
			{
				return;
			}
		}
	}

	void MaterialInstanceParams::SetUniformBuffer(StringID& name, BufferHandle buffer)
	{
		for (I32 a = 0; a < setsCount; ++a)
		{
			if (shaderParamsDesc[a].TrySetBuffer(name, buffer))
			{
				return;
			}
		}
	}

	void MaterialInstanceParams::SetUniformBuffer(I32 set, BufferHandle buffer, I32 descriptor, I32 size)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return;
		}
		if (descriptor < shaderParamsDesc[set].descriptorsCount)
		{
			shaderParamsDesc[set].SetBuffer(descriptor, buffer,size);
		}
		else
		{
			LOG_BE_ERROR("[MaterialInstanceParams] SetUniformBuffer: Descriptor index {0} is out of bounds for set {1}.", descriptor, set);
		}
	}

	void MaterialInstanceParams::SetStorageBuffer(I32 set, BufferHandle buffer, I32 descriptor, I32 size)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return;
		}
		if (descriptor < shaderParamsDesc[set].descriptorsCount)
		{
			shaderParamsDesc[set].SetBuffer(descriptor, buffer,size);
		}
		else
		{
			LOG_BE_ERROR("[MaterialInstanceParams] SetStorageBuffer: Descriptor index {0} is out of bounds for set {1}.", descriptor, set);
		}
	}

	void MaterialInstanceParams::SetStorageBufferWithData(I32 set, MaterialCustomBufferData* customData, I32 descriptor)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return;
		}
		if (descriptor < shaderParamsDesc[set].descriptorsCount)
		{
			shaderParamsDesc[set].descriptors[descriptor].customData = customData;
		}
		else
		{
			LOG_BE_ERROR("[MaterialInstanceParams] SetStorageBufferWithData: Descriptor index {0} is out of bounds for set {1}.", descriptor, set);
		}
	}

	void MaterialInstanceParams::UpdateBufferSize(I32 set, I32 descriptor, I32 size)
	{
		if (set >= k_max_descriptor_set_layouts)
		{
			return;
		}
		if (descriptor < shaderParamsDesc[set].descriptorsCount)
		{
			shaderParamsDesc[set].SetBufferSize(descriptor, size);
		}
		else
		{
			LOG_BE_ERROR("[MaterialInstanceParams] UpdateBufferSize: Descriptor index {0} is out of bounds for set {1}.", descriptor, set);
		}
	}

	void MaterialInstanceParams::UpdateBufferSize(DescriptorBindingTagType tag, I32 size)
	{
		for (I32 a = 0; a < setsCount; ++a)
		{
			if (shaderParamsDesc[a].SetBufferSize(tag, size))
			{
				return;
			}
		}
		LOG_BE_ERROR("[MaterialInstanceParams] UpdateBufferSize: Tag {0} not found in any set.", static_cast<int>(tag));
	}

	TextureHandle MaterialInstanceParams::GetTextureHandleForName(StringID& name)
	{
		for (I32 a = 0; a < setsCount; ++a)
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
	MaterialInstanceDesc& MaterialInstanceDesc::AddBuffer(const CzuchStr& name, MaterialCustomBufferData&& data)
	{
		paramsDesc.push_back({ .name = name,.data = std::move(data),.type = DescriptorType::UNIFORM_BUFFER,.resource = Invalid_Handle_Id,.isInternal = false });
		return *this;
	}

	MaterialInstanceDesc& MaterialInstanceDesc::AddStorageBufferSingleData(const CzuchStr& name, MaterialCustomBufferData&& data)
	{
		paramsDesc.push_back({ .name = name,.data = std::move(data),.type = DescriptorType::STORAGE_BUFFER_SINGLE_DATA,.resource = Invalid_Handle_Id,.isInternal = false });
		return *this;
	}

	MaterialInstanceDesc& MaterialInstanceDesc::AddBuffer(const CzuchStr& name, BufferHandle handle)
	{
		paramsDesc.push_back({ .name = name,.data = MaterialCustomBufferData(),.type = DescriptorType::UNIFORM_BUFFER,.resource = handle.handle,.isInternal = true });
		return *this;
	}
	MaterialInstanceDesc& MaterialInstanceDesc::AddStorageBuffer(const CzuchStr& name, BufferHandle handle)
	{
		paramsDesc.push_back({ .name = name,.data = MaterialCustomBufferData(),.type = DescriptorType::STORAGE_BUFFER,.resource = handle.handle,.isInternal = true });
		return *this;
	}
	MaterialInstanceDesc& MaterialInstanceDesc::AddSampler(const CzuchStr& name, TextureHandle color_texture, bool isInternal)
	{
		paramsDesc.push_back({ .name = name,.data = MaterialCustomBufferData(),.type = DescriptorType::SAMPLER,.resourceAsset = color_texture.assetHandle.handle,.resource = color_texture.handle,.isInternal = isInternal });
		return *this;
	}

	MaterialInstanceDesc MaterialInstanceDesc::Clone()
	{
		MaterialInstanceDesc desc;
		for (I32 a = 0; a < paramsDesc.size(); a++)
		{
			auto& param = paramsDesc[a];
			if (param.type == DescriptorType::SAMPLER)
			{
				desc.AddSampler(param.name, TextureHandle(param.resource, param.resourceAsset), param.isInternal);
			}
			else if (param.type == DescriptorType::UNIFORM_BUFFER)
			{
				MaterialCustomBufferData uboCopy(param.data.GetData(), param.data.GetSize(), DescriptorBindingTagType::NONE);
				desc.AddBuffer(param.name, std::move(uboCopy));
			}
		}
		desc.isTransparent = isTransparent;
		desc.materialAsset = materialAsset;
		return desc;
	}

	void MaterialPassDesc::SetParams(MaterialInstanceDesc& desc, MaterialInstanceParams& params)
	{
		for (I32 i = 0; i < layoutsCount; ++i)
		{
			auto& current = layouts[i];
			for (I32 b = 0; b < current.bindingsCount; ++b)
			{
				auto& binding = current.bindings[b];
				if (binding.type == DescriptorType::SAMPLER)
				{
					params.AddSampler(i, binding.bindingName.GetStrName(), TextureHandle(), b);
				}
				else if (binding.type == DescriptorType::UNIFORM_BUFFER)
				{
					params.AddBuffer(i, binding.bindingName.GetStrName(), BufferHandle(), b,binding.size);
				}
				else if (binding.type == DescriptorType::STORAGE_BUFFER)
				{
					params.AddStorageBuffer(i, binding.bindingName.GetStrName(), BufferHandle(), b, binding.tag,binding.size);
				}
				else if (binding.type == DescriptorType::STORAGE_BUFFER_SINGLE_DATA)
				{
					params.AddStorageBufferWithData(i, binding.bindingName.GetStrName(), nullptr, b, binding.tag);
				}
				else if (binding.type == DescriptorType::COMBINED_IMAGE_SAMPLER)
				{
					params.SetAsTextureBindlessSet(i);
				}
			}
		}

		for (I32 a = 0; a < desc.paramsDesc.size(); ++a)
		{
			auto& param = desc.paramsDesc[a];
			StringID nameId = StringID::MakeStringID(param.name);
			for (I32 i = 0; i < layoutsCount; ++i)
			{
				auto& current = layouts[i];
				for (I32 b = 0; b < current.bindingsCount; ++b)
				{
					auto& binding = current.bindings[b];
					if (binding.bindingName.Compare(nameId) == 0)
					{
						if (param.type == DescriptorType::SAMPLER)
						{
							params.SetSampler(i, TextureHandle(param.resource, param.resourceAsset), b);
						}
						else if (param.type == DescriptorType::UNIFORM_BUFFER)
						{
							params.SetUniformBuffer(i,BufferHandle(param.resource), b);
						}
						else if (param.type == DescriptorType::STORAGE_BUFFER)
						{
							params.SetStorageBuffer(i,BufferHandle(param.resource), b);
						}
						else if (param.type == DescriptorType::STORAGE_BUFFER_SINGLE_DATA)
						{
							params.SetStorageBufferWithData(i, &param.data, b);
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
		for (I32 a = 0; a < desc->PassesCount(); ++a)
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
		for (I32 a = 0; a < desc->passesContainer.passes.size(); ++a)
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
		for (I32 a = 0; a < passesCount; ++a)
		{
			params[a].SetSampler(name, texture);
		}
	}

	void MaterialInstance::SetUniformBuffer(StringID& name, BufferHandle buffer)
	{
		for (I32 a = 0; a < passesCount; ++a)
		{
			params[a].SetUniformBuffer(name, buffer);
		}
	}

	TextureHandle MaterialInstance::GetTextureHandleForName(StringID& name)
	{
		for (I32 a = 0; a < passesCount; ++a)
		{
			auto handle = params[a].GetTextureHandleForName(name);
			if (HANDLE_IS_VALID(handle))
			{
				return handle;
			}
		}
	}


	void MaterialInstance::UpdateSizeForTag(DescriptorBindingTagType tag, I32 size, int pass)
	{
		for (U32 j = 0; j < params[pass].setsCount; j++)
		{
			auto& set = params[pass].shaderParamsDesc[j];
			for (U32 k = 0; k < set.descriptorsCount; k++)
			{
				if (set.descriptors[k].tag == tag)
				{
					set.descriptors[k].size = size;
					/*if (set.updateControl != nullptr)
					{
						set.updateControl->UpdateDescriptorSet(set.descriptors[k],false);
					}*/
					return;
				}
			}
		}
		LOG_BE_ERROR("[MaterialInstance] UpdateSizeForTag: No descriptor found for tag {0} in pass {1}.", static_cast<int>(tag), pass);
	}

	void MaterialInstance::SetAsDirty()
	{
		isDirty = true;
		for (int a = 0; a < k_max_render_passes; ++a)
		{
			auto& current = params[a];
			for (int b = 0; b < current.setsCount; b++)
			{
				current.shaderParamsDesc[b].currentDescriptor = nullptr;
			}
		}
	}
	
	MaterialCustomBufferData* MaterialInstance::GetDataForTag(DescriptorBindingTagType tag, I32 pass)
	{
		for (U32 j = 0; j < params[pass].setsCount; j++)
		{
			auto& set = params[pass].shaderParamsDesc[j];
			for (U32 k = 0; k < set.descriptorsCount; k++)
			{
				if (set.descriptors[k].tag == tag)
				{
					return set.descriptors[k].customData ? set.descriptors[k].customData : nullptr;
				}
			}
		}
		return nullptr;
	}

	I32 MaterialInstance::UpdateCustomDataWithTag(DescriptorBindingTagType tag, void* data, U32 size, I32 pass)
	{
		for (U32 j = 0; j < params[pass].setsCount; j++)
		{
			auto& set = params[pass].shaderParamsDesc[j];
			for (U32 k = 0; k < set.descriptorsCount; k++)
			{
				if (set.descriptors[k].tag == tag)
				{
					auto current = set.descriptors[k].customData;
					if (current == nullptr)
					{
						LOG_BE_ERROR("[MaterialInstance] UpdateCustomDataWithTag: No custom data found in pass {0}.", pass);
						return -1;
					}
					else
					{
						current->SetData(data, size);
						return set.descriptors[k].assetHandle; // Return the index of the buffer in the storage buffer container
					}
				}
			}
		}
		LOG_BE_ERROR("[MaterialInsance] UpdateCustomDataWithTag: No descriptor found in pass {0}.", pass);
		return -1;
	}

	I32 MaterialInstance::GetIndexForInternalBufferForTag(DescriptorBindingTagType tag, I32 pass) const
	{
		for (U32 j = 0; j < params[pass].setsCount; j++)
		{
			auto& set = params[pass].shaderParamsDesc[j];
			for (U32 k = 0; k < set.descriptorsCount; k++)
			{
				if (set.descriptors[k].tag == tag)
				{
					return set.descriptors[k].assetHandle;
				}
			}
		}
		return -1;
	}

	void MaterialInstance::SetIndexAndBufferForInternalBufferForTag(DescriptorBindingTagType tag, I32 index, BufferHandle buffer, I32 pass)
	{
		for (U32 j = 0; j < params[pass].setsCount; j++)
		{
			auto& set = params[pass].shaderParamsDesc[j];
			for (U32 k = 0; k < set.descriptorsCount; k++)
			{
				if (set.descriptors[k].tag == tag)
				{
					set.descriptors[k].resource = buffer.handle;
					set.descriptors[k].assetHandle = index;
					return;
				}
			}
		}
	}

	StorageBufferTagInfo MaterialInstance::GetInfoForDescriptorTag(DescriptorBindingTagType tag, I32 pass)
	{
		StorageBufferTagInfo info;
		info.tag = tag;
		info.descriptorIndex = -1;
		info.index = -1;


		for (U32 j = 0; j < params[pass].setsCount; j++)
		{
			auto& set = params[pass].shaderParamsDesc[j];
			for (U32 k = 0; k < set.descriptorsCount; k++)
			{
				if (set.descriptors[k].tag == tag)
				{
					info.tag = tag;
					info.descriptorIndex = j;
					info.index = k;
					return info;
				}
			}
		}

		return info;
	}


	void MeshData::ComputeAABB()
	{
		//compute aabb from positions
		aabb.min = Vec3(FLT_MAX);
		aabb.max = Vec3(-FLT_MAX);
		for (I32 a = 0; a < vertices.size(); ++a)
		{
			aabb.min = glm::min(aabb.min, vertices[a].position);
			aabb.max = glm::max(aabb.max, vertices[a].position);
		}
	}

	BufferHandle MultiplerBufferContainer::GetBuffer(const BufferDesc& desc)
	{
		//check if buffer with this desc already exists
		for (auto& buffer : multipleBuffers)
		{
			if (buffer.desc.bufferType == desc.bufferType && buffer.desc.format == desc.format && buffer.desc.stride == desc.stride && buffer.desc.usage == desc.usage && (buffer.currentOffset + desc.size) < buffer.desc.size)
			{
				return buffer.AddNewBufferPart(desc);
			}
		}

		return BufferHandle();
	}

	bool MultiplerBufferContainer::Release(BufferHandle handle)
	{
		for (auto& buffer : multipleBuffers)
		{
			if (buffer.handle.handle == handle.handle)
			{
				return buffer.ReleasePart(handle);
			}
		}
		return false;
	}

	BufferHandle MultiplerBufferContainer::CreateBuffer(const BufferDesc& desc, GraphicsDevice* device, U32 capacity)
	{
		BufferDesc mDesc;
		mDesc.bind_flags = desc.bind_flags;
		mDesc.usage = desc.usage;
		mDesc.format = desc.format;
		mDesc.stride = desc.stride;
		mDesc.size = capacity;
		mDesc.persistentMapped = desc.persistentMapped;
		mDesc.initData = nullptr;
		mDesc.exclusiveBuffer = true;
		mDesc.bufferType = desc.bufferType;

		BufferHandle handle = device->CreateBuffer(&mDesc);

		if (HANDLE_IS_VALID(handle))
		{
			MultipleBuffer newBuffer;
			newBuffer.handle = handle;
			newBuffer.desc = mDesc;
			newBuffer.currentOffset = 0;
			newBuffer.parts = 0;
			multipleBuffers.push_back(std::move(newBuffer));
			return GetBuffer(desc);
		}
		LOG_BE_ERROR("[MultiplerBufferContainer]Failed to create buffer");
		return BufferHandle();
	}

	void MultiplerBufferContainer::DrawDebugWindow()
	{
		windowName = "Multiple Buffer Statistics";
		ImGui::Begin("Multiple Buffer Statistics",&windowOpen);

		ImGui::Text("Total Multiple Buffers: %zu", multipleBuffers.size());
		ImGui::Separator();

		U32 totalSubAllocations = 0;
		U32 totalSpaceOccupied = 0;

		if (ImGui::CollapsingHeader("Individual Buffers"))
		{
			for (size_t i = 0; i < multipleBuffers.size(); ++i)
			{
				MultipleBuffer& mb = multipleBuffers[i];
				totalSubAllocations += mb.parts;
				totalSpaceOccupied += mb.currentOffset;

				ImGui::PushID(i); // Unique ID for each buffer in the loop

				float spaceUsedPercent = (mb.desc.size > 0) ? (static_cast<float>(mb.currentOffset) / mb.desc.size) * 100.0f : 0.0f;

				ImGui::Text("Buffer %zu (Handle ID: %u, Offset: %u)", i, mb.handle.handle, mb.handle.offset);
				ImGui::Indent();
				ImGui::Text("Capacity: %u bytes", mb.desc.size);
				ImGui::Text("Used Space: %u bytes", mb.currentOffset);
				ImGui::Text("Sub-allocations: %u", mb.parts);
				ImGui::Text("Space Used: %.2f%%", spaceUsedPercent);
				ImGui::Unindent();
				ImGui::Separator();
				ImGui::PopID();
			}
		}

		ImGui::Separator();
		ImGui::Text("Overall Statistics:");
		ImGui::Text("Total Sub-allocations Across All Buffers: %u", totalSubAllocations);
		ImGui::Text("Total Space Currently Occupied (Sum of currentOffset): %u bytes", totalSpaceOccupied);

		ImGui::End();
	}

}