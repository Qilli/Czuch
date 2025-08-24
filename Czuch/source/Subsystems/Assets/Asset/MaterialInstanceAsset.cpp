#include "czpch.h"
#include "MaterialInstanceAsset.h"
#include "./Renderer/GraphicsDevice.h"
#include "./Core/FileHelper.h"
#include "./Subsystems/Assets/AssetsManager.h"
#include "./Subsystems/Assets/Asset/TextureAsset.h"
#include "./Subsystems/Assets/Asset/MaterialAsset.h"
#include "./Subsystems/Assets/AssetManagersTypes/MaterialInstanceAssetManager.h"


namespace Czuch
{
	MaterialInstanceAsset::MaterialInstanceAsset(const CzuchStr& path, GraphicsDevice* device, MaterialInstanceLoadSettings& loadSettings, AssetsManager* assetsManager) : Asset(path, GetNameFromPath(path),assetsManager), m_Device(device)
	{
		m_AssetType = AssetModeType::LOADED_TYPE;
		m_MaterialLoadSettings = std::move(loadSettings);
	}

	MaterialInstanceAsset::MaterialInstanceAsset(const CzuchStr& path, GraphicsDevice* device, MaterialInstanceCreateSettings& settings, AssetsManager* assetsManager) : Asset(path, GetNameFromPath(path),assetsManager), m_Device(device)
	{
		m_AssetType = AssetModeType::CREATED_TYPE;
		m_MaterialCreateSettings = std::move(settings);
		CreateFromData();
	}

	MaterialInstanceAsset::~MaterialInstanceAsset()
	{
		m_ForceUnload = true;
		UnloadAsset();
	}

	MaterialAsset* MaterialInstanceAsset::GetMaterialAsset()
	{
		if (m_MaterialInstanceDesc.materialAsset.IsValid())
		{
			return AssetsManager::GetPtr()->GetAsset<MaterialAsset>(m_MaterialInstanceDesc.materialAsset, false);
		}
		return nullptr;
	}

	void MaterialInstanceAsset::UpdateSizeForTag(DescriptorBindingTagType tag, U32 size,int pass)
	{
		m_MaterialInstance->UpdateSizeForTag(tag, size,pass);
	}

	void MaterialInstanceAsset::SetAsDirty()
	{
		m_MaterialInstance->SetAsDirty();
	}


	bool MaterialInstanceAsset::LoadAsset()
	{
		if (Asset::LoadAsset())
		{
			return true;
		}

		if (m_AssetType == AssetModeType::CREATED_TYPE)
		{
			LoadDependencies();
			m_MaterialInstanceResource = m_Device->CreateMaterialInstance(m_MaterialInstanceDesc);
			m_MaterialInstance = m_Device->AccessMaterialInstance(m_MaterialInstanceResource);
			m_State = AssetInnerState::LOADED;
			m_RefCounter.Up();
			return true;
		}

		if (!ReadBinaryFile(AssetPath(), m_MaterialInstanceBuffer))
		{
			LOG_BE_ERROR("{0} Failed to load material instance resource at path {1}", "[MaterialInstanceResource]", AssetPath());
			return false;
		}

		//TODO Parse material from file and create material assedt

		if (!HANDLE_IS_VALID(m_MaterialInstanceResource))
		{
			return false;
		}
		m_RefCounter.Up();
		m_State = AssetInnerState::LOADED;
		return true;
	}

	bool MaterialInstanceAsset::UnloadAsset()
	{
		if (ShouldUnload() && HANDLE_IS_VALID(m_MaterialInstanceResource))
		{
			UnloadDependencies();
			m_Device->Release(m_MaterialInstanceResource);
			m_State = AssetInnerState::UNLOADED;
			return true;
		}
		return false;
	}

	bool MaterialInstanceAsset::CreateFromData()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			return true;
		}

		
		m_MaterialInstanceDesc = std::move(m_MaterialCreateSettings.desc);
		m_MaterialInstanceDesc.GetAllTexturesDependencies(m_Dependencies);
		LoadDependencies();
		m_MaterialInstanceResource = m_Device->CreateMaterialInstance(m_MaterialInstanceDesc);
		m_MaterialInstance = m_Device->AccessMaterialInstance(m_MaterialInstanceResource);
		m_State = AssetInnerState::LOADED;
		m_RefCounter.Up();
		return true;
	}

	ShortAssetInfo* MaterialInstanceAsset::GetShortAssetInfo()
	{
		if (m_ShortInfo.name == nullptr || m_ShortInfo.name->empty())
		{
			m_ShortInfo.name = &AssetName();
			m_ShortInfo.type = AssetType::MATERIAL;
			m_ShortInfo.asset = GetHandle();
		}

		m_ShortInfo.resource = m_MaterialInstanceResource.handle;

		return &m_ShortInfo;
	}
	CzuchStr MaterialInstanceAsset::GetAssetLoadInfo() const
	{
		return "MaterialInstanceAsset: " + AssetName()+" Ref count: "+m_RefCounter.GetCountString() + " State: " + (m_State == AssetInnerState::LOADED ? " Loaded" : "Unloaded");
	}

	MaterialParamType MaterialInstanceAsset::GetParameterAtIndexType(int index) const
	{
		CZUCH_BE_ASSERT(index < m_MaterialInstanceDesc.paramsDesc.size(), "Index out of range");
		auto descType=m_MaterialInstanceDesc.paramsDesc[index].type;

		switch (descType)
		{

		case DescriptorType::UNIFORM_BUFFER:
			return MaterialParamType::PARAM_BUFFER;
		case DescriptorType::SAMPLER:
			return MaterialParamType::PARAM_TEXTURE;
		default:	
			return MaterialParamType::PARAM_UNKNOWN;
		}
	}

	bool MaterialInstanceAsset::IsParameterAtIndexInternal(int index) const
	{
		CZUCH_BE_ASSERT(index < m_MaterialInstanceDesc.paramsDesc.size(), "Index out of range");
		return m_MaterialInstanceDesc.paramsDesc[index].isInternal;
	}

	const CzuchStr& MaterialInstanceAsset::GetParameterAtIndexName(int index) const
	{
		CZUCH_BE_ASSERT(index < m_MaterialInstanceDesc.paramsDesc.size(), "Index out of range");
		return m_MaterialInstanceDesc.paramsDesc[index].name;
	}

	std::tuple<AssetHandle, I32> MaterialInstanceAsset::GetTextureAssetAtIndex(int index) const
	{
		CZUCH_BE_ASSERT(index < m_MaterialInstanceDesc.paramsDesc.size(), "Index out of range");
		auto id = StringID::MakeStringID(m_MaterialInstanceDesc.paramsDesc[index].name);
		TextureHandle texHandle = m_MaterialInstance->GetTextureHandleForName(id);
		return { texHandle.assetHandle,texHandle.handle};
	}

	std::tuple<Buffer*, UBOLayout*> MaterialInstanceAsset::GetUBOBufferAtIndex(int index) const
	{
		CZUCH_BE_ASSERT(index < m_MaterialInstanceDesc.paramsDesc.size(), "Index out of range");
		auto &param = m_MaterialInstanceDesc.paramsDesc[index];


		auto id = StringID::MakeStringID(param.name);
		BufferHandle handle(param.resource);
		auto buffer = m_Device->AccessBuffer(handle);
		auto material = AssetsManager::GetPtr()->GetAsset<MaterialAsset>(m_MaterialInstanceDesc.materialAsset, false);
		auto layout = material->GetUBOLayoutForName(id);
		return { buffer,layout };
	}

	void MaterialInstanceAsset::UpdateUBOBufferAtIndex(int index)
	{
		CZUCH_BE_ASSERT(index < m_MaterialInstanceDesc.paramsDesc.size(), "Index out of range");
		auto& param = m_MaterialInstanceDesc.paramsDesc[index];
		auto id = StringID::MakeStringID(param.name);
		//copy data to ubo data from buffer to update it

		BufferHandle handle(param.resource);
		m_Device->UploadCurrentDataToBuffer(handle);
	}

	void MaterialInstanceAsset::SetTextureParameterAtIndex(int index, AssetHandle asset, I32 resource)
	{
		CZUCH_BE_ASSERT(index < m_MaterialInstanceDesc.paramsDesc.size(), "Index out of range");
		auto oldHandle = m_MaterialInstanceDesc.paramsDesc[index].resourceAsset;

		if (asset == oldHandle)
		{
			return;
		}

		if (oldHandle != Invalid_Handle_Id)
		{
			AssetsManager::GetPtr()->UnloadAsset<TextureAsset>(oldHandle);
		}

		if (resource == Invalid_Handle_Id)
		{
			//we need to load texture
			auto assetNew = AssetsManager::GetPtr()->GetAsset<TextureAsset>(asset, true);
			if (assetNew == nullptr || !assetNew->IsLoaded())
			{
				LOG_BE_ERROR("{0} Failed to load texture dependency in SetTextureParam: {1}", "[MaterialInstanceAsset]", asset.handle);
			}
			else
			{
				resource = assetNew->GetTextureResourceHandle().handle;
			}
		}

		m_MaterialInstanceDesc.paramsDesc[index].resourceAsset = asset.handle;
		m_MaterialInstanceDesc.paramsDesc[index].resource = resource;

		auto id = StringID::MakeStringID(m_MaterialInstanceDesc.paramsDesc[index].name);
		TextureHandle texHandle(resource, asset.handle);
		m_MaterialInstance->SetSampler(id,texHandle);

		if (asset == Invalid_Handle_Id)
		{
			return;
		}
		
		auto assetNew = AssetsManager::GetPtr()->GetAsset<TextureAsset>(asset, true);
		if (assetNew == nullptr || !assetNew->IsLoaded())
		{
			LOG_BE_ERROR("{0} Failed to load texture dependency in SetTextureParam: {1}", "[MaterialInstanceAsset]", asset.handle);
		}
	}

	StorageBufferTagInfo MaterialInstanceAsset::GetInfoDescriptorTag(DescriptorBindingTagType tag,int pass) const
	{
		return m_MaterialInstance->GetInfoForDescriptorTag(tag,pass);
	}

	void MaterialInstanceAsset::ChangeDataForDescriptorWithTag(DescriptorBindingTagType tag, void* data, U32 size)
	{
		I32 index=m_MaterialInstance->UpdateCustomDataWithTag(tag, data, size,0);
		if (index < 0)
		{
			LOG_BE_ERROR("[MaterialInstanceAsset] Failed to update data for descriptor tag: {0}", static_cast<int>(tag));
			return;
		}
		auto targetData = m_MaterialInstance->GetDataForTag(tag, 0);
		m_AssetManager->UpdateDataForGlobalStorage(index,*targetData);
	}

	U32 MaterialInstanceAsset::GetParametersCount() const
	{
		return m_MaterialInstanceDesc.paramsDesc.size();
	}

	void MaterialInstanceAsset::LoadDependencies()
	{
		for (auto& dep : m_Dependencies)
		{
			auto asset = AssetsManager::GetPtr()->GetAsset<TextureAsset>(dep.assetHandle,true);
			if (asset==nullptr || !asset->IsLoaded())
			{
				LOG_BE_ERROR("[MaterialInstanceAsset] Failed to load texture dependency: {0}",dep.assetHandle.handle);
			}
		}
	}
	void MaterialInstanceAsset::UnloadDependencies()
	{
		for (auto& dep : m_Dependencies)
		{
			AssetsManager::GetPtr()->UnloadAsset<TextureAsset>(dep.assetHandle);
		}

		for (auto& param : m_MaterialInstanceDesc.paramsDesc)
		{
			if (param.type == DescriptorType::UNIFORM_BUFFER && param.isInternal==false)
			{
				auto handle = BufferHandle(param.resource);
				m_Device->Release(handle);
				param.resource =Invalid_Handle_Id;
			}
		}
	}

	static int s_MaterialInstanceCounter = 0;
	MaterialInstanceAsset* MaterialInstanceAsset::CloneMaterialInstance()
	{
		MaterialInstanceCreateSettings settings;
		settings.desc = std::move(m_MaterialInstanceDesc.Clone());
		settings.materialInstanceName = AssetName() + "_Clone" + std::to_string(s_MaterialInstanceCounter++);
	
		auto asset = AssetsManager::GetPtr()->CreateMaterialInstance(settings);
		return (MaterialInstanceAsset*)AssetsManager::GetPtr()->GetAsset<MaterialInstanceAsset>(asset, true);
	}
	void* MaterialInstanceAsset::GetParameterDataWithInfo(StorageBufferTagInfo& info)
	{
		auto data = m_MaterialInstance->GetDataForTag(info.tag, 0);
		if (data != nullptr)
		{
			return data->dataRaw.data();
		}
		return nullptr;
	}
	void MaterialInstanceAsset::SetGlobalIndexAndBufferForDescriptorAtIndex(DescriptorBindingTagType tag, I32 index, BufferHandle handle)
	{
		m_MaterialInstance->SetIndexAndBufferForInternalBufferForTag(tag, index, handle,0);
	}
}
