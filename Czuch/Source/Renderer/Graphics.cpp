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
	DescriptorSetLayoutDesc& DescriptorSetLayoutDesc::AddBinding(CzuchStr name,DescriptorType type, U32 bindingIndex, U32 count, U32 size)
	{
		bindings[bindingsCount++] = { name,type,size,(U16)bindingIndex, (U16)count};
		return *this;
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

	DescriptorSetDesc& DescriptorSetDesc::Reset()
	{
		descriptorsCount = 0;
		return *this;
	}

	DescriptorSetDesc& DescriptorSetDesc::AddBuffer(BufferHandle buffer, U16 binding)
	{
		if (descriptorsCount >= s_max_descriptors_per_set)
		{
			return *this;
		}

		descriptors[descriptorsCount].binding = binding;
		descriptors[descriptorsCount].resource = buffer.handle;
		descriptors[descriptorsCount++].type = DescriptorType::UNIFORM_BUFFER;

		return *this;
	}


	DescriptorSetDesc& DescriptorSetDesc::AddSampler(TextureHandle color_texture, U16 binding)
	{
		if (descriptorsCount >= s_max_descriptors_per_set)
		{
			return *this;
		}

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
}