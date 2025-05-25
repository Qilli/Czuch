#include "czpch.h"
#include "AssetsManager.h"
#include <typeinfo>
#include "./Subsystems/Logging.h"
#include "Asset/TextureAsset.h"
#include "Asset/MaterialInstanceAsset.h"
#include "Asset/MaterialAsset.h"
#include "./Subsystems/Assets/AssetManagersTypes/MaterialAssetManager.h"


namespace Czuch
{
	std::string AssetsManager::m_StartPath;

	void AssetsManager::Init(EngineSettings* settings)
	{
		BaseSubsystem::Init(settings);
		m_StartPath = settings->GetStartPath();
		m_AssetsInfoTemp.reserve(100);
	}

	void AssetsManager::Shutdown()
	{
		m_isDuringShutdown = true;
		for (const auto& [key, value] : m_AssetsMgrs)
		{
			value->UnloadAll();
		}

		for (const auto& [key, value] : m_AssetsMgrs)
		{
			delete value;
		}

		m_AssetsMgrs.clear();
	}

	void AssetsManager::Update(TimeDelta timeDelta)
	{
	}

	void AssetsManager::RegisterManager(AssetManager* newMgr, std::type_index type)
	{
		m_AssetsMgrs.insert({type,newMgr});
	}

	std::filesystem::path AssetsManager::GetAssetPath(AssetHandle handle)
	{
		Asset* asset=GetAsset<TextureAsset>(handle,false);
		if (asset)
		{
			return asset->AssetPath();
		}
		return std::filesystem::path();
	}

	void AssetsManager::UnloadAndRemoveAsset(const CzuchStr& path)
	{
		StringID strId = StringID::MakeStringID(path);
		AssetHandle handle = { strId.GetGuid()};
		for (const auto& [key, value] : m_AssetsMgrs)
		{
			if (value->GetAsset(handle))
			{
				value->RemoveAsset(handle);
				return;
			}
		}
	}

	void AssetsManager::InitManagers()
	{
		for (const auto& [key, value] : m_AssetsMgrs)
		{
			value->Init();
		}
	}

	const Array<ShortAssetInfo*>& AssetsManager::GetAssetsOfTypes(int assetsFilter)
	{
		m_AssetsInfoTemp.clear();
		for (const auto& [key, value] : m_AssetsMgrs)
		{
			if (assetsFilter == value->GetAssetType())
			{
				for (auto it = value->GetAssetIterator(); it != value->GetAssetEnd(); ++it)
				{
					ShortAssetInfo *assetInfo = (*it).second->GetShortAssetInfo();
					if (assetInfo->hiddenInEditor==false)
					{
						m_AssetsInfoTemp.push_back(assetInfo);
					}

					ShortAssetInfo* nextElem = assetInfo->next;
					while (nextElem != nullptr)
					{
						m_AssetsInfoTemp.push_back(nextElem);
						nextElem = nextElem->next;
					}
				}
			}
		}
		return m_AssetsInfoTemp;
	}

	bool AssetsManager::IsFormatSupported(const char* format) const
	{
		for (const auto& [key, value] : m_AssetsMgrs)
		{
			if (value->IsFormatSupported(format))
			{
				return true;
			}
		}
		return false;
	}

	bool AssetsManager::IsFormatAssetOfType(const char* format, AssetType type) const
	{
		for (const auto& [key, value] : m_AssetsMgrs)
		{
			if (value->GetAssetType() == (int)type)
			{
				if (value->IsFormatSupported(format))
				{
					return true;
				}
			}
		}
		return false;
	}

	void AssetsManager::CheckIfAssetExistsAndIfNotCreate(const std::filesystem::path& pathRelative) const
	{
		if (m_isDuringShutdown)
		{
			return;
		}

		for (const auto& [key, value] : m_AssetsMgrs)
		{
			if (value->IsFormatSupported(pathRelative.extension().string().c_str()))
			{
				std::string path = pathRelative.string();
				value->TryCreateAssetWithUnloadState(path);
				return;
			}
		}
	}

	TextureHandle AssetsManager::Load2DTexture(const CzuchStr& path)
	{
		if (m_isDuringShutdown)
		{
			return TextureHandle{};
		}

		TextureLoadSettings settings{};
		settings.type = TextureDesc::Type::TEXTURE_2D;
		auto texAsset = LoadAsset<TextureAsset, Czuch::TextureLoadSettings>(path,settings);
		CZUCH_BE_ASSERT(texAsset.IsValid(), "Texture asset is not valid, Load2DTexture failed");
		auto texResource = GetAsset<TextureAsset>(texAsset);
		CZUCH_BE_ASSERT(texResource, "Texture resource is not valid");
		return texResource->GetTextureResourceHandle();
	}

	AssetHandle AssetsManager::CreateMaterialInstance(MaterialInstanceCreateSettings& settings)
	{
		if (m_isDuringShutdown)
		{
			return AssetHandle{};
		}

		auto asset = GetAsset<MaterialAsset>(settings.desc.materialAsset);
		settings.desc.SetTransparent(asset->IsTransparent());
		return CreateAsset<MaterialInstanceAsset>(settings.materialInstanceName, settings);
	}

	AssetHandle AssetsManager::CreateMaterialInstance(const CzuchStr& matName, AssetHandle materialSource)
	{
		if (m_isDuringShutdown)
		{
			return AssetHandle{};
		}
		Czuch::MaterialInstanceCreateSettings instanceCreateSettings{};
		instanceCreateSettings.materialInstanceName = matName;
		instanceCreateSettings.desc.materialAsset = materialSource;
		return CreateMaterialInstance(instanceCreateSettings);
	}

	//here we update all materials which are from lighting pass, so we have correct amount of lights for storage buffers
	void AssetsManager::UpdateLightingMaterialsLightInfo(U32 lightsCount, U32 lightsIndexRangesCount, U32 lightTilesCount)
	{
		//find all material with lighting pass
		MaterialAssetManager* mgr = GetManagerOfType<MaterialAssetManager,MaterialAsset>();

		if (mgr == nullptr)
		{
			LOG_BE_ERROR("{0} Failed to find material asset manager", "[AssetsManager]");
			return;
		}

		auto &filtered=mgr->GetAllAssetsWithFilter([](Asset* asset)-> bool {
			MaterialAsset* mat = static_cast<MaterialAsset*>(asset);
			if(mat->HasPassType(RenderPassType::ForwardLighting)|| mat->HasPassType(RenderPassType::ForwardLightingTransparent))
			{
				return true;
			}
			});

		for (auto asset : filtered)
		{
			MaterialAsset* mat = static_cast<MaterialAsset*>(asset);
			auto* bindingLightsContainer = mat->GetBindingWithTag(DescriptorBindingTagType::LIGHTS_CONTAINER);
			auto* bindingLightsIndexRanges = mat->GetBindingWithTag(DescriptorBindingTagType::LIGHTS_INDEXES);
			auto* bindingLightsTiles = mat->GetBindingWithTag(DescriptorBindingTagType::LIGHTS_TILES);

			if (bindingLightsContainer)
			{
				bindingLightsContainer->size = lightsCount * sizeof(LightData);
			}

			if (bindingLightsIndexRanges)
			{
				bindingLightsIndexRanges->size = lightsIndexRangesCount * sizeof(U32);
			}

			if (bindingLightsTiles)
			{
				bindingLightsTiles->size = lightTilesCount * sizeof(LightsTileData);
			}
		}
	}

	void AssetsManager::UpdateRenderObjectsInfo(U32 renderObjectsCount)
	{
		MaterialAssetManager* mgr = GetManagerOfType<MaterialAssetManager, MaterialAsset>();

		if (mgr == nullptr)
		{
			LOG_BE_ERROR("{0} Failed to find material asset manager", "[AssetsManager]");
			return;
		}

		auto& filtered = mgr->GetAllAssetsWithFilter([](Asset* asset)-> bool {
			MaterialAsset* mat = static_cast<MaterialAsset*>(asset);
			if (mat->HasPassType(RenderPassType::ForwardLighting) || mat->HasPassType(RenderPassType::ForwardLightingTransparent))
			{
				return true;
			}
			});

		for (auto asset : filtered)
		{
			MaterialAsset* mat = static_cast<MaterialAsset*>(asset);
			auto* bindingRenderObjectsContainer = mat->GetBindingWithTag(DescriptorBindingTagType::RENDER_OBJECTS);

			if (bindingRenderObjectsContainer)
			{
				bindingRenderObjectsContainer->size = renderObjectsCount * sizeof(RenderObjectGPUData);
			}
		}
	}

}
