#include"czpch.h"
#define VMA_IMPLEMENTATION
#include"VulkanCore.h"
#include"../GraphicsDevice.h"
#include"../Renderer.h"
namespace Czuch
{
	void* Buffer_Vulkan::GetMappedData()
	{
		return allocation->GetMappedData();
	}

	class VulkanDevice;
	void ParamSetVulkanUpdateControl::UpdateDescriptorSet(ShaderParamInfo& param, bool forceFullUpdate)
	{
		if (forceFullUpdate)
		{
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				U32 frame = i;
				auto binding = descriptorSet[frame]->descriptorLayout->desc.bindings[param.binding];
				if (binding.type == DescriptorType::UNIFORM_BUFFER)
				{
					writer.WriteBuffer(binding.index, device->AccessBuffer(BufferHandle(param.resource)), param.size, 0, binding.type);
				}
				else if (binding.type == DescriptorType::SAMPLER)
				{
					writer.WriteTexture(binding.index, device->AccessTexture({ param.resource,AssetHandle() }), DescriptorType::SAMPLER);
				}
				else if (binding.type == DescriptorType::STORAGE_BUFFER)
				{
					if (param.resource != -1)
					{
						auto buffer = device->AccessBuffer(BufferHandle(param.resource));
						if (buffer != nullptr)
						{
							writer.WriteBuffer(binding.index, buffer, param.size, 0, binding.type);
						}
					}

				}
				writer.UpdateSet((VulkanDevice*)device, descriptorSet[frame]);
			}
		}
		else
		{
			U32 frame = GetCurrentFrameIndex();
			auto binding = descriptorSet[frame]->descriptorLayout->desc.bindings[param.binding];
			if (binding.type == DescriptorType::UNIFORM_BUFFER)
			{
				writer.WriteBuffer(binding.index, device->AccessBuffer(BufferHandle(param.resource)), binding.size, 0, binding.type);
			}
			else if (binding.type == DescriptorType::SAMPLER)
			{
				writer.WriteTexture(binding.index, device->AccessTexture({ param.resource,AssetHandle() }), DescriptorType::SAMPLER);
			}
			else if (binding.type == DescriptorType::STORAGE_BUFFER)
			{
				if (param.resource != -1)
				{
					auto buffer = device->AccessBuffer(BufferHandle(param.resource));
					if (buffer != nullptr)
					{
						writer.WriteBuffer(binding.index, buffer, binding.size, 0, binding.type);
					}
				}

			}
			writer.UpdateSet((VulkanDevice*)device, descriptorSet[frame]);
		}
	}

	U32 ParamSetVulkanUpdateControl::GetCurrentFrameIndex() const
	{
		return device->GetCurrentFrameIndex();
	}

	ParamSetVulkanUpdateControl::~ParamSetVulkanUpdateControl()
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			descriptorSet[i] = nullptr;
		}
		if (device != nullptr)
		{
			device = nullptr;
		}
		writer.Clear();
		writer.writes.clear();
		writer.imageInfos.clear();
		writer.bufferInfos.clear();
	}
}