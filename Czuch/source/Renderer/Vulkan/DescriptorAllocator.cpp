#include "czpch.h"
#include "DescriptorAllocator.h"
#include "VulkanCore.h"
#include "VulkanDevice.h"

namespace Czuch
{

	VkDescriptorPool CreatePool(VkDevice device, const DescriptorAllocator::PoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags);
	void DescriptorAllocator::ResetPools()
	{
		for (auto p : m_UsedPools)
		{
			vkResetDescriptorPool(m_Device->GetNativeDevice(), p, 0);
			m_FreePools.push_back(p);
		}

		m_UsedPools.clear();

		for (auto d : m_Descriptors)
		{
			delete d;
		}
		m_Descriptors.clear();
		m_CurrentPool = VK_NULL_HANDLE;
	}

	void WriteDescriptor(DescriptorWriter& writer,VulkanDevice* device, DescriptorSet* descriptor)
	{
		writer.Clear();
		for (int i = 0; i < descriptor->desc->descriptorsCount; ++i)
		{
			auto& current = descriptor->desc->descriptors[i];
			if (current.type == DescriptorType::UNIFORM_BUFFER || current.type == DescriptorType::UNIFORM_BUFFER_DYNAMIC || current.type == DescriptorType::STORAGE_BUFFER || current.type == DescriptorType::STORAGE_BUFFER_DYNAMIC)
			{
				auto handle = BufferHandle(current.resource);
				if (!HANDLE_IS_VALID(handle))
				{
					return;
				}
				Buffer* buffer = (Buffer*)device->AccessBuffer(BufferHandle(current.resource));
				if (buffer == nullptr)
				{
					LOG_BE_ERROR("[DescriptorAllocator] Failed to access buffer for descriptor set with id: {0}", current.resource);
					continue;
				}
				writer.WriteBuffer(current.binding, buffer, buffer->desc.size, 0, current.type);
			}
			else
			{
				Texture* tex = (Texture*)device->AccessTexture({current.resource,AssetHandle()});
				if (tex != nullptr)
				{
					writer.WriteTexture(current.binding, tex, current.type);
				}
				else
				{
					return;
					//LOG_BE_ERROR("[DescriptorAllocator] Failed to access texture for descriptor set with id: {0}", current.resource);
				}
			}
		}

		writer.UpdateSet(device, descriptor);
	}

	DescriptorSet* DescriptorAllocator::Allocate(ShaderParamsSet& desc, DescriptorSetLayout* layout)
	{
		if (m_CurrentPool == VK_NULL_HANDLE)
		{
			m_CurrentPool = GrabPool();
			m_UsedPools.push_back(m_CurrentPool);
		}

		auto  vkLayout = Internal_to_DescriptorSetLayout(layout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.pSetLayouts = &vkLayout->layout;
		allocInfo.descriptorPool = m_CurrentPool;
		allocInfo.descriptorSetCount = 1;

		DescriptorSet *descSet = new DescriptorSet();;
		descSet->descriptorLayout = layout;
		descSet->desc = &desc;
		m_Descriptors.push_back(descSet);

		VkResult result = vkAllocateDescriptorSets(m_Device->GetNativeDevice(), &allocInfo, &descSet->descriptorSet);

		CZUCH_BE_ASSERT(result == VK_SUCCESS || result == VK_ERROR_FRAGMENTED_POOL || result == VK_ERROR_OUT_OF_POOL_MEMORY, "Failed to allocate descriptor set!");

		switch (result)
		{
		case VK_SUCCESS:
			WriteDescriptor(m_Writer,m_Device, descSet);
			return descSet;
		case VK_ERROR_FRAGMENTED_POOL:
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			break;
		default:
			return {};
		}

		m_CurrentPool = GrabPool();
		m_UsedPools.push_back(m_CurrentPool);

		result=vkAllocateDescriptorSets(m_Device->GetNativeDevice(), &allocInfo, &descSet->descriptorSet);

		if (result != VK_SUCCESS)
		{
			LOG_BE_ERROR("[DescriptorAllocator] Failed to allocate new descriptor set");
		}

		WriteDescriptor(m_Writer, m_Device,descSet);
		return descSet;
	}

	void DescriptorAllocator::Init(VulkanDevice* device)
	{
		m_Device = device;
	}

	void DescriptorAllocator::CleanUp()
	{
		for (auto d : m_Descriptors)
		{
			delete d;
		}
		m_Descriptors.clear();

		for (auto d : m_FreePools)
		{
			vkDestroyDescriptorPool(m_Device->GetNativeDevice(), d, nullptr);
		}

		for (auto d2 : m_UsedPools)
		{
			vkDestroyDescriptorPool(m_Device->GetNativeDevice(), d2, nullptr);
		}


		m_FreePools.clear();
		m_UsedPools.clear();
	}

	VkDescriptorPool DescriptorAllocator::GrabPool()
	{
		if (m_FreePools.size() > 0)
		{
			VkDescriptorPool pool = m_FreePools.back();
			m_FreePools.pop_back();
			return pool;
		}
		else
		{
			return CreatePool(m_Device->GetNativeDevice(), m_DescriptorSizes, 1000, VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);
		}
	}

#pragma region Helpers

	VkDescriptorPool CreatePool(VkDevice device, const DescriptorAllocator::PoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags)
	{
		std::vector<VkDescriptorPoolSize> sizes;
		sizes.reserve(poolSizes.sizes.size());
		for (auto sz : poolSizes.sizes)
		{
			sizes.push_back({ sz.first,U32(sz.second * count) });
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = flags;
		poolInfo.maxSets = count;
		poolInfo.poolSizeCount = U32(sizes.size());
		poolInfo.pPoolSizes = sizes.data();

		VkDescriptorPool newPool;
		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &newPool) != VK_SUCCESS)
		{
			return nullptr;
		}

		return newPool;
	}

#pragma endregion

#pragma region descriptor writer

	void DescriptorWriter::WriteBuffer(int binding, Buffer* buffer, size_t size, size_t offset, DescriptorType type)
	{
		VkDescriptorBufferInfo& info = bufferInfos.emplace_back(VkDescriptorBufferInfo{
			.buffer=Internal_to_Buffer(buffer)->buffer,
			.offset=offset,
			.range=size});

		VkWriteDescriptorSet write = {.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

		write.dstSet = VK_NULL_HANDLE;
		write.descriptorCount = 1;
		write.dstBinding = binding;
		write.descriptorType = ConvertDescriptorType(type);
		write.pBufferInfo = &info;
		writes.push_back(write);
	}

	void DescriptorWriter::WriteTexture(int binding, Texture* color_texture, DescriptorType type)
	{
		if (color_texture == nullptr)
		{
			LOG_BE_ERROR("[DescriptorWriter] Failed to write texture to descriptor set");
			return;
		}

		auto vulkanTex = Internal_to_Texture(color_texture);
		VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back(VkDescriptorImageInfo {
			.sampler = vulkanTex->sampler,
			.imageView = vulkanTex->imageView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		});
	
		VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

		write.dstSet = VK_NULL_HANDLE;
		write.descriptorCount = 1;
		write.dstBinding = binding;
		write.descriptorType = ConvertDescriptorType(type);
		write.pImageInfo = &imageInfo;
		writes.push_back(write);
	}

	void DescriptorWriter::Clear()
	{
		imageInfos.clear();
		bufferInfos.clear();
		writes.clear();
	}

	void DescriptorWriter::UpdateSet(VulkanDevice* device, DescriptorSet* descriptorSet)
	{
		for (VkWriteDescriptorSet& set : writes)
		{
			set.dstSet = descriptorSet->descriptorSet;
		}
		vkUpdateDescriptorSets(device->GetNativeDevice(), (uint32_t)writes.size(), writes.data(), 0, nullptr);
	}

#pragma endregion

}
