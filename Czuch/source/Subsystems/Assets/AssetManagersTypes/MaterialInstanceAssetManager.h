#pragma once
#include"../AssetsManager.h"
#include "../Asset/MaterialInstanceAsset.h"
#include "../Asset/MaterialAsset.h"

namespace Czuch
{

	/// <summary>
	/// It holds materials data for lighting. We want to keep all materials in one array and then use indices to access them.
	/// </summary>
	struct MaterialsDataContainer
	{
		Buffer* materialsBuffer;
		BufferHandle materialsBufferHandle;
		DescriptorBindingTagType dataTag;
		GraphicsDevice* device;
		U32 materialsCount;
		U32 elementSize;
		U32 AddMaterialData(void* data);
		void UpdateMaterialData(I32 index, MaterialCustomBufferData& data);
		void Init(GraphicsDevice* device, DescriptorBindingTagType dataBindingTag,U32 elementSize);
		void Release();
	};


    class MaterialInstanceAssetManager :public AssetManager
	{
	public:
		MaterialInstanceAssetManager(GraphicsDevice* device);
		~MaterialInstanceAssetManager() override;
		bool IsFormatSupported(const char* format)const override { return strcmp(format, ".mat") == 0; }
		void Init() override;
		int GetAssetType() const override { return (int)AssetType::MATERIAL_INSTANCE; }
		void RegisterDataContainer(DescriptorBindingTagType dataBindingTag,U32 elementSize, GraphicsDevice* device)
		{
			MaterialsDataContainer container;
			container.Init(device, dataBindingTag,elementSize);
			m_DataContainer.push_back(std::move(container));
		}

		BufferHandle GetStorageBufferHandleForDescriptorTag(DescriptorBindingTagType tag) const
		{
			for (const auto& container : m_DataContainer)
			{
				if (container.dataTag == tag)
				{
					return container.materialsBufferHandle;
				}
			}
			return BufferHandle();
		}

		void UpdateDataForGlobalStorage(I32 index,MaterialCustomBufferData& data);
		void DirtyAllMaterials();

	protected:
		Asset* CreateAsset(const CzuchStr& name, BaseCreateSettings& settings) override;
		Asset* CreateAsset(const CzuchStr& path, BaseLoadSettings& settings) override;
		Asset* CreateLoadableAsset(const CzuchStr& path) override;
	private:
		U32 AddDataForDescriptorWithTag(DescriptorBindingTagType tag, void* data, U32 size);
	private:
		GraphicsDevice* m_Device;
		Array<MaterialsDataContainer> m_DataContainer;
	};

	inline MaterialInstanceAssetManager::MaterialInstanceAssetManager(GraphicsDevice* device) :m_Device(device)
	{
	}

	inline MaterialInstanceAssetManager::~MaterialInstanceAssetManager()
	{
		for (auto& container : m_DataContainer)
		{
			container.Release();
		}
		m_DataContainer.clear();
	}

	inline void MaterialInstanceAssetManager::UpdateDataForGlobalStorage(I32 index,MaterialCustomBufferData& data)
	{
		for (auto& container : m_DataContainer)
		{
			if (container.dataTag == data.tagType)
			{
				container.UpdateMaterialData(index, data);
				return;
			}
		}
		CZUCH_BE_ASSERT(false, "MaterialInstanceAssetManager::UpdateDataForGlobalStorage: No container found for the given data tag.");
	}

	inline void MaterialInstanceAssetManager::DirtyAllMaterials()
	{
		ExecuteOnAllAssets([](Asset* asset) 
			{
				MaterialInstanceAsset* matInst = dynamic_cast<MaterialInstanceAsset*>(asset);
				if (matInst)
				{
					matInst->SetAsDirty();
				}
			});
	}

	inline Asset* MaterialInstanceAssetManager::CreateAsset(const CzuchStr& path, BaseCreateSettings& settings)
	{
		StringID strId = StringID::MakeStringID(path);
		MaterialInstanceCreateSettings& matSettings = (MaterialInstanceCreateSettings&)settings;
		MaterialInstanceAsset* matRes = new MaterialInstanceAsset(path,m_Device,matSettings,AssetsManager::GetPtr());
		RegisterAsset(strId, matRes);
		MaterialAsset* matAsset = AssetsManager::GetPtr()->GetAsset<MaterialAsset>(matSettings.desc.materialAsset, false);

		CZUCH_BE_ASSERT(matAsset != nullptr, "MaterialInstanceAssetManager::CreateAsset: Material asset is null for material instance asset creation.");

		if (matAsset->HasBindingWithTag(DescriptorBindingTagType::MATERIALS_LIGHTING_DATA))
		{
			// Add material data to the container
			auto info = matRes->GetInfoDescriptorTag(DescriptorBindingTagType::MATERIALS_LIGHTING_DATA,0);
			if (info.index >= 0)//if less than 0 then it means that material does not have this tag, so we do not need to add data
			{
				void* gpuData = matRes->GetParameterDataWithInfo(info);
				if (gpuData != nullptr)
				{

					U32 dataIndex = AddDataForDescriptorWithTag(DescriptorBindingTagType::MATERIALS_LIGHTING_DATA, gpuData, sizeof(MaterialObjectGPUData)); //this add data to the container and returns index of data in the container(one big buffer)
					BufferHandle handle = GetStorageBufferHandleForDescriptorTag(DescriptorBindingTagType::MATERIALS_LIGHTING_DATA);//global buffer handle for this tag
					matRes->SetGlobalIndexAndBufferForDescriptorAtIndex(DescriptorBindingTagType::MATERIALS_LIGHTING_DATA, dataIndex, handle);//set index and buffer handle for this tag in the material instance asset
				}
			}
		}

		return matRes;
	}

	inline Asset* MaterialInstanceAssetManager::CreateAsset(const CzuchStr& path, BaseLoadSettings& settings)
	{
		MaterialInstanceAsset* matRes = new MaterialInstanceAsset(path,m_Device, (MaterialInstanceLoadSettings&)settings,AssetsManager::GetPtr());
		return matRes;
	}

	inline Asset* MaterialInstanceAssetManager::CreateLoadableAsset(const CzuchStr& path)
	{
		MaterialInstanceLoadSettings settings;
		MaterialInstanceAsset* matRes = new MaterialInstanceAsset(path, m_Device, settings, AssetsManager::GetPtr());
		//MaterialAsset* matAsset = AssetsManager::GetPtr()->GetAsset<MaterialAsset>(settings., false);
		return matRes;
	}

	inline U32 MaterialInstanceAssetManager::AddDataForDescriptorWithTag(DescriptorBindingTagType tag, void* data, U32 size)
	{
		for (auto& container : m_DataContainer)
		{
			if (container.dataTag == tag)
			{
				return container.AddMaterialData(data);
			}
		}
		return -1;
	}

}

