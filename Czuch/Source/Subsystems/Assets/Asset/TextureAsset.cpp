#include "czpch.h"
#include "TextureAsset.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "./Renderer/GraphicsDevice.h"

namespace Czuch
{

	TextureAsset::TextureAsset(const CzuchStr& path, TextureLoadSettings& loadSettings, GraphicsDevice* device, AssetsManager* assetsManager):Asset(path, GetNameFromPath(path),assetsManager),m_Device(device)
	{
		//stbi_set_flip_vertically_on_load(true);
		m_AssetType = AssetModeType::LOADED_TYPE;
		m_CurrentLoadSettings = std::move(loadSettings);
		LOG_BE_INFO("Created new texture asset with unloaded state at path: {0}", AssetPath());
	}

	TextureAsset::TextureAsset(const CzuchStr& path, TextureCreateSettings& settings, GraphicsDevice* device, AssetsManager* assetsManager) :Asset(path,path,assetsManager), m_Device(device)
	{
		m_AssetType = AssetModeType::CREATED_TYPE;
		m_CreateSettings = std::move(settings);
		if (CreateFromData())
		{
			LOG_BE_INFO("Created new texture from data with name: {0}", AssetName());
		}
		else
		{
			LOG_BE_ERROR("Failed to create new texture from data with name: {0}", AssetName());
		}
	}


	TextureAsset::~TextureAsset()
	{
		m_ForceUnload = true;
		UnloadAsset();
	}

	bool TextureAsset::LoadAsset()
	{
		if (Asset::LoadAsset())
		{
			return true;
		}

		if (m_AssetType == AssetModeType::CREATED_TYPE)
		{
			return false;
		}

		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(AssetPath().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		U32 imageSize = texWidth * texHeight * 4;

		if (!pixels)
		{
			LOG_BE_ERROR("{0} Failed to load texture resource at path {1}", "[TextureResource]", AssetPath());
			return false;
		}
		TextureDesc desc;
		desc.width = texWidth;
		desc.height = texHeight;
		desc.texData = pixels;

		if (m_CurrentLoadSettings.type == TextureDesc::Type::TEXTURE_2D)
		{
			desc.format = Format::R8G8B8A8_UNORM_SRGB;
		}
		else
		{
			//TODO: Implement other texture types
			m_CurrentLoadSettings.type = TextureDesc::Type::TEXTURE_2D;
			desc.format = Format::R8G8B8A8_UNORM_SRGB;
		}

		m_TextureResource = m_Device->CreateTexture(&desc,false);
		stbi_image_free(pixels);

		if (!HANDLE_IS_VALID(m_TextureResource))
		{
			return false;
		}

		m_TextureResource.assetHandle = GetHandle();

		m_State = AssetInnerState::LOADED;

		if (m_CurrentLoadSettings.isUITexture)
		{
			InitUITexturePtr();
		}

		m_RefCounter.Up();

		LOG_BE_INFO("Loaded new texture with path: {0}", AssetPath());
		
		return true;
	}

	bool TextureAsset::UnloadAsset()
	{
		if (ShouldUnload() && HANDLE_IS_VALID(m_TextureResource))
		{
			m_Device->Release(m_TextureResource);
			INVALIDATE_HANDLE(m_TextureResource)
			m_State = AssetInnerState::UNLOADED;
			return true;
		}
		return false;
	}

	bool TextureAsset::CreateFromData()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			return true;
		}

		if (m_AssetType == AssetModeType::LOADED_TYPE)
		{
			return false;
		}

		U32 texWidth=m_CreateSettings.width, texHeight= m_CreateSettings.height, texChannels = m_CreateSettings.channels;
		stbi_uc* pixels = m_CreateSettings.colors.data();
		U32 imageSize = texWidth * texHeight * 4;

		if (!pixels)
		{
			LOG_BE_ERROR("{0} Failed to create texture resource from data", "[TextureResource]");
			return false;
		}

		TextureDesc desc;
		desc.width = texWidth;
		desc.height = texHeight;
		desc.texData = pixels;

		if (m_CurrentLoadSettings.type == TextureDesc::Type::TEXTURE_2D)
		{
			desc.format = Format::R8G8B8A8_UNORM_SRGB;
		}

		m_TextureResource = m_Device->CreateTexture(&desc,false);

		if (!HANDLE_IS_VALID(m_TextureResource))
		{
			return false;
		}

		m_TextureResource.assetHandle = GetHandle();

		m_State = AssetInnerState::LOADED;

		if (m_CreateSettings.isUITexture)
		{
			InitUITexturePtr();
		}
		m_RefCounter.Up();

		return true;
	}

	ShortAssetInfo* TextureAsset::GetShortAssetInfo()
	{
		if (m_ShortInfo.name == nullptr || m_ShortInfo.name->empty())
		{
			m_ShortInfo.name = &AssetName();
			m_ShortInfo.type = AssetType::TEXTURE;
			m_ShortInfo.asset = GetHandle();
			m_ShortInfo.resource = m_TextureResource.handle;
		}

		return &m_ShortInfo;
	}

	CzuchStr TextureAsset::GetAssetLoadInfo() const
	{
		return "TextureAsset: " + AssetName() + " Ref count: " + m_RefCounter.GetCountString() + " State: " + (m_State == AssetInnerState::LOADED ? " Loaded" : "Unloaded");
	}
	void TextureAsset::InitUITexturePtr()
	{
		m_UITextureIDPtr = m_Device->CreatePointerForUITexture(m_TextureResource);
	}
}
