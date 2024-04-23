#include "czpch.h"
#include "AssetsManager.h"
#include <typeinfo>
#include "./Subsystems/Logging.h"
#include "Asset/TextureAsset.h"
#include "Asset/MaterialInstanceAsset.h"
#include "Asset/MaterialAsset.h"


namespace Czuch
{

	void AssetsManager::Init()
	{

	}

	void AssetsManager::Shutdown()
	{
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

	TextureHandle AssetsManager::Load2DTexture(const CzuchStr& path)
	{
		TextureLoadSettings settings{};
		settings.type = TextureDesc::Type::TEXTURE_2D;
		auto texAsset = LoadAsset<TextureAsset, Czuch::TextureLoadSettings>(path,settings);
		CZUCH_BE_ASSERT(texAsset.IsValid(), "Texture asset is not valid, Load2DTexture failed");
		auto texResource = GetAsset<TextureAsset>(texAsset);
		CZUCH_BE_ASSERT(texResource, "Texture resource is not valid");
		return texResource->GetTextureResourceHandle();
	}

	MaterialInstanceHandle AssetsManager::CreateMaterialInstance(MaterialInstanceCreateSettings& settings)
	{
		auto asset = GetAsset<MaterialAsset>(settings.desc.materialAsset);
		settings.desc.SetTransparent(asset->IsTransparent());
		auto matInstanceHandle=CreateAsset<MaterialInstanceAsset>(settings.materialInstanceName, settings);
		auto instanceResource = GetAsset<Czuch::MaterialInstanceAsset>(matInstanceHandle);
		return instanceResource->GetMaterialInstanceResourceHandle();
	}

	MaterialInstanceHandle AssetsManager::CreateMaterialInstance(const CzuchStr& matName, AssetHandle materialSource)
	{
		Czuch::MaterialInstanceCreateSettings instanceCreateSettings{};
		instanceCreateSettings.materialInstanceName = matName;
		instanceCreateSettings.desc.materialAsset = materialSource;
		return CreateMaterialInstance(instanceCreateSettings);
	}

}
