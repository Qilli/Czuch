#include "czpch.h"
#include "DescriptorLayoutCache.h"
#include<algorithm>
#include<functional>

namespace Czuch
{
	void DescriptorLayoutCache::Init(VkDevice device)
	{
		m_Device = device;
	}

	void DescriptorLayoutCache::CleanUp()
	{
		for (auto pair : m_LayoutCache)
		{
			vkDestroyDescriptorSetLayout(m_Device,pair.second,nullptr);
		}
		m_LayoutCache.clear();
	}

	VkDescriptorSetLayout DescriptorLayoutCache::CreateDescriptorLayout(VkDescriptorSetLayoutCreateInfo* info)
	{
		DescriptorLayoutInfo layoutInfo;
		layoutInfo.bindings.reserve(info->bindingCount);
		bool isSorted = true;
		int lastBinding = -1;

		for (int i = 0; i < info->bindingCount; ++i)
		{
			layoutInfo.bindings.push_back(info->pBindings[i]);

			if (info->pBindings[i].binding > lastBinding)
			{
				lastBinding = info->pBindings[i].binding;
			}
			else
			{
				isSorted = false;
			}
		}

		if (!isSorted)
		{
			std::sort(layoutInfo.bindings.begin(), layoutInfo.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b)
				{
					return a.binding < b.binding;
				});
		}

		auto it = m_LayoutCache.find(layoutInfo);
		if (it != m_LayoutCache.end())
		{
			return it->second;
		}
		else
		{
			VkDescriptorSetLayout layout;
			vkCreateDescriptorSetLayout(m_Device, info, nullptr, &layout);
			m_LayoutCache[layoutInfo] = layout;
			return layout;
		}
	}

	bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
	{
		if (other.bindings.size() != bindings.size())
		{
			return false;
		}

		for (int a = 0; a < bindings.size(); ++a)
		{
			if (other.bindings[a].binding != bindings[a].binding)
			{
				return false;
			}

			if (other.bindings[a].descriptorType != bindings[a].descriptorType)
			{
				return false;
			}

			if (other.bindings[a].descriptorCount != bindings[a].descriptorCount)
			{
				return false;
			}

			if (other.bindings[a].stageFlags != bindings[a].stageFlags)
			{
				return false;
			}
		}
		return true;
	}

	size_t DescriptorLayoutCache::DescriptorLayoutInfo::Hash() const
	{
		using std::size_t;
		using std::hash;

		size_t result = hash<size_t>()(bindings.size());

		for (const VkDescriptorSetLayoutBinding& b : bindings)
		{
			size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;
			//xor with main hash
			result ^= hash<size_t>()(binding_hash);
		}

		return result;
	}
}
