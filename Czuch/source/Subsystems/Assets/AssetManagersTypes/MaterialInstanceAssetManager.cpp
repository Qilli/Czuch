#include "czpch.h"
#include "MaterialInstanceAssetManager.h"
#include "../BuildInAssets.h"
#include "Renderer/GraphicsDevice.h"

namespace Czuch
{
	void MaterialInstanceAssetManager::Init()
	{
		m_DefaultAsset = GetAsset(DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET);
		RegisterDataContainer(DescriptorBindingTagType::MATERIALS_LIGHTING_DATA, sizeof(MaterialObjectGPUData),m_Device);
	}


	U32 MaterialsDataContainer::AddMaterialData(void* data)
	{
		CZUCH_BE_ASSERT(materialsBuffer != nullptr, "Materials buffer is not initialized. Call Init() before adding data.");
		CZUCH_BE_ASSERT(materialsCount < MATERIAL_DATA_CAPACITY, "Materials data capacity exceeded. Increase MATERIAL_DATA_CAPACITY if needed.");

		U32 index = materialsCount++;
		void* mappedDataPtr = device->GetMappedBufferDataPtr(materialsBufferHandle);
		
		if (mappedDataPtr)
		{
			uint8_t* byteData = static_cast<uint8_t*>(mappedDataPtr);
			memcpy(byteData + index * elementSize, data, elementSize);
		}
		else
		{
			CZUCH_BE_ASSERT(false, "Failed to get mapped data pointer for materials buffer.");
		}
		return index;
	}

	void MaterialsDataContainer::UpdateMaterialData(I32 index, MaterialCustomBufferData& data)
	{
		void* mappedDataPtr = device->GetMappedBufferDataPtr(materialsBufferHandle);
		if (mappedDataPtr)
		{
			uint8_t* byteData = static_cast<uint8_t*>(mappedDataPtr);
			memcpy(byteData + index * elementSize, data.dataRaw.data(), data.GetSize());
		}
		else
		{
			CZUCH_BE_ASSERT(false, "UpdateMaterialData: Failed to get mapped data pointer for materials buffer.");
		}
	}

	void MaterialsDataContainer::Init(GraphicsDevice* dev, DescriptorBindingTagType dataBindingTag, U32 elemSize)
	{
		BufferDesc desc{};
		desc.persistentMapped = true;
		desc.elementsCount = 0;
		desc.bind_flags = BindFlag::STORAGE_BUFFER;
		desc.size = elemSize * MATERIAL_DATA_CAPACITY;
		desc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;
		materialsBufferHandle = dev->CreateBuffer(&desc);
		materialsBuffer = dev->AccessBuffer(materialsBufferHandle);
		materialsCount = 0;
		device = dev;
		elementSize = elemSize;
		dataTag = dataBindingTag;
	}

	void MaterialsDataContainer::Release()
	{
		if (HANDLE_IS_VALID(materialsBufferHandle))
		{
			device->Release(materialsBufferHandle);
			materialsBufferHandle = BufferHandle();
			materialsBuffer = nullptr;
		}
		materialsCount = 0;
	}
}
