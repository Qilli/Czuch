#pragma once
#include"VulkanBase.h"
#include<vector>
#include<unordered_map>

namespace Czuch
{
	class DescriptorLayoutCache
	{
	public:
		void Init(VkDevice device);
		void CleanUp();

		VkDescriptorSetLayout CreateDescriptorLayout(VkDescriptorSetLayoutCreateInfo* info);

		struct DescriptorLayoutInfo
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			VkDescriptorSetLayout layout;
			int flags;
			bool operator==(const DescriptorLayoutInfo& other) const;
			size_t Hash() const;
		};
	private:
		struct DescriptorLayoutHash
		{
			std::size_t operator()(const DescriptorLayoutInfo& info) const
			{
				return info.Hash();
			}
		};

		Array<DescriptorLayoutInfo> m_LayoutCache;
		VkDevice m_Device;
	};
}


