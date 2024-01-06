#include"czpch.h"
#include "Graphics.h"

namespace Czuch
{
	DescriptorSetLayoutDesc& DescriptorSetLayoutDesc::Reset()
	{
		bindingsCount = 0;
		setIndex = 0;
		return *this;
	}
	DescriptorSetLayoutDesc& DescriptorSetLayoutDesc::AddBinding(DescriptorType type, U32 index, U32 count)
	{
		bindings[bindingsCount++] = { type, (U16)index, (U16)count };
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
}