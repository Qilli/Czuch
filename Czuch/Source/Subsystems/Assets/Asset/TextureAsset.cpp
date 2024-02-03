#include "czpch.h"
#include "TextureAsset.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "./Renderer/GraphicsDevice.h"

namespace Czuch
{
	TextureAsset::TextureAsset(const CzuchStr& path, TextureLoadSettings* loadSettings, GraphicsDevice* device):Asset(path, GetNameFromPath(path)),m_Device(device),m_Settings(loadSettings)
	{
		LoadAsset();
	}

	TextureAsset::~TextureAsset()
	{
		UnloadAsset();
	}

	bool TextureAsset::LoadAsset()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			return true;
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

		if (m_Settings==nullptr||m_Settings->type == TextureDesc::Type::TEXTURE_2D)
		{
			desc.format = Format::R8G8B8A8_UNORM_SRGB;
		}

		m_TextureAsset = m_Device->CreateTexture(&desc);
		stbi_image_free(pixels);

		if (!HANDLE_IS_VALID(m_TextureAsset))
		{
			return false;
		}

		m_State = AssetInnerState::LOADED;
		
		return true;
	}

	void TextureAsset::UnloadAsset()
	{
		if (m_State == AssetInnerState::LOADED && HANDLE_IS_VALID(m_TextureAsset))
		{
			m_Device->Release(m_TextureAsset);
			INVALIDATE_HANDLE(m_TextureAsset)
			m_State = AssetInnerState::UNLOADED;
		}
	}
}
