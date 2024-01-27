#pragma once
#include"VulkanBase.h"
#include<vector>
#include<deque>

namespace Czuch
{
	struct DescriptorSet;
	struct DescriptorSetDesc;
	struct DescriptorSetLayout;
	enum DescriptorType;
	struct Buffer;

	struct DescriptorWriter
	{
		void WriteBuffer(int binding, Buffer* buffer, size_t size, size_t offset, DescriptorType type);
		void Clear();
		void UpdateSet(VkDevice device,DescriptorSet* descriptorSet);

		std::deque<VkDescriptorImageInfo> imageInfos;
		std::deque<VkDescriptorBufferInfo> bufferInfos;
		std::vector<VkWriteDescriptorSet> writes;
	};

	class DescriptorAllocator
	{
	public:
		struct PoolSizes
		{
			std::vector<std::pair<VkDescriptorType, float>> sizes =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
			};
		};


		void ResetPools();
		DescriptorSet* Allocate(DescriptorSetDesc& desc,DescriptorSetLayout* layout);
		void Init(VkDevice device);
		void CleanUp();
	private:
		DescriptorWriter m_Writer;
		VkDescriptorPool GrabPool();
		VkDevice m_Device;
		VkDescriptorPool m_CurrentPool{ VK_NULL_HANDLE };
		PoolSizes m_DescriptorSizes;
		std::vector<VkDescriptorPool> m_UsedPools;
		std::vector<VkDescriptorPool> m_FreePools;
		std::vector<DescriptorSet*> m_Descriptors;
	};

}

