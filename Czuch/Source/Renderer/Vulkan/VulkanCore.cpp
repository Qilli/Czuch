#include"czpch.h"
#define VMA_IMPLEMENTATION
#include"VulkanCore.h"
namespace Czuch
{
	void* Buffer_Vulkan::GetMappedData()
	{
		return allocation->GetMappedData();
	}
}