#pragma once
#include "./Renderer/Graphics.h"

namespace Czuch
{
	struct BaseSettings
	{
		bool permamentAsset;

		BaseSettings()
		{
			permamentAsset = false;
		}
	};

	struct BaseLoadSettings: public BaseSettings
	{

	};

	struct BaseCreateSettings: public BaseSettings
	{

	};

	struct LoadSettingsDefault: BaseLoadSettings
	{

	};

	struct TextureLoadSettings: BaseLoadSettings
	{
		TextureDesc::Type type= TextureDesc::Type::TEXTURE_2D;
	};

	struct TextureCreateSettings: BaseCreateSettings
	{
		TextureDesc::Type type = TextureDesc::Type::TEXTURE_2D;
		U32 width = 128;
		U32 height = 128;
		U16 channels = 4;
		std::vector<unsigned char> colors;

		TextureCreateSettings() = default;

		TextureCreateSettings(TextureCreateSettings&& other) noexcept
		{
			std::cout << "In constructor(MemoryBlock&&). length = " << "." << std::endl;
			*this = std::move(other);
		}

		TextureCreateSettings& operator=(TextureCreateSettings&& other) noexcept
		{
			std::cout << "In operator=(MemoryBlock&&). length = " << "." << std::endl;
			if (&other != this)
			{
				this->channels = other.channels;
				this->height = other.height;
				this->width = other.width;
				this->type = other.type;
				this->colors = std::move(other.colors);
			}
			return *this;
		}
	};

	struct CZUCH_API MaterialCreateSettings : BaseCreateSettings
	{
		MaterialDesc desc;
	};

	struct MaterialLoadSettings : BaseLoadSettings
	{

	};

	struct CZUCH_API MaterialInstanceCreateSettings : BaseCreateSettings
	{
		MaterialInstanceDesc desc;
		CzuchStr materialInstanceName;

		MaterialInstanceCreateSettings()
		{
			desc.AddBuffer("SceneData", INVALID_HANDLE(BufferHandle));
		}
	};

	struct MaterialInstanceLoadSettings : BaseLoadSettings
	{

	};

	struct ShaderLoadSettings: BaseLoadSettings
	{

	};

	struct ShaderCreateSettings: BaseCreateSettings
	{
		ShaderStage stage;
		ShaderFormat format;
		std::vector<char> shaderCode;

		ShaderCreateSettings() = default;

		ShaderCreateSettings(ShaderCreateSettings& other) noexcept
		{
			*this = std::move(other);
		}

		ShaderCreateSettings& operator=(ShaderCreateSettings&& other) noexcept
		{
			if (&other != this)
			{
				this->format = other.format;
				this->stage = other.stage;
				this->shaderCode = std::move(other.shaderCode);
			}
			return *this;
		}
	};


	struct ModelLoadSettings : BaseLoadSettings
	{

	};

	struct ModelCreateSettings : BaseCreateSettings
	{
		std::vector<MeshData> meshesData;
		CzuchStr modelName;
	};


}
