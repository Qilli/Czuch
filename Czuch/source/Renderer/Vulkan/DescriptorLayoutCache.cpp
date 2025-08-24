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
		for (auto layout : m_LayoutCache)
		{
			vkDestroyDescriptorSetLayout(m_Device, layout.layout, nullptr);
		}
		m_LayoutCache.clear();
	}

	VkDescriptorSetLayout DescriptorLayoutCache::CreateDescriptorLayout(VkDescriptorSetLayoutCreateInfo* info)
	{
		DescriptorLayoutInfo layoutInfo;
		layoutInfo.bindings.reserve(info->bindingCount);
		layoutInfo.flags = info->flags;
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


		/*VkDescriptorSetLayout layout;
		vkCreateDescriptorSetLayout(m_Device, info, nullptr, &layout);
		//m_LayoutCache[layoutInfo] = layout;
		m_Layouts.push_back(layout);
		return layout;*/
		DescriptorLayoutInfo* infoPtr = nullptr;
		for (auto& layout : m_LayoutCache)
		{
			if (layout == layoutInfo)
			{
				infoPtr = &layout;
				break;
			}
		}

		if (infoPtr != nullptr)
		{
			return infoPtr->layout;
		}
		else
		{
			VkDescriptorSetLayout layout;
			vkCreateDescriptorSetLayout(m_Device, info, nullptr, &layout);
			layoutInfo.layout = layout;
			m_LayoutCache.push_back(std::move(layoutInfo));
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

			if(other.flags != flags)
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

		// A better way to combine hashes
		auto hash_combine = [](size_t& seed, const size_t& v) {
			seed ^= v + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			};

		size_t result = hash<size_t>()(bindings.size());

		for (const VkDescriptorSetLayoutBinding& b : bindings)
		{
			// Hash each individual member
			size_t binding_hash = hash<uint32_t>()(b.binding);
			hash_combine(binding_hash, hash<VkDescriptorType>()(b.descriptorType));
			hash_combine(binding_hash, hash<uint32_t>()(b.descriptorCount));
			hash_combine(binding_hash, hash<VkShaderStageFlags>()(b.stageFlags));

			// Combine the individual binding hash with the total result
			hash_combine(result, binding_hash);
		}

		return result;
	}
}
