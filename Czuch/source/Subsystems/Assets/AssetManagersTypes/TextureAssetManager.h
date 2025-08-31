#pragma once
#include"../AssetsManager.h"
#include "../Asset/TextureAsset.h"
namespace Czuch
{
	class GraphicsDevice;
    class TextureAssetManager :public AssetManager
	{

		struct DynamicGlobalTexturesDebugWindow : public DrawDebugBaseGuiWindow
		{
			TextureAssetManager* mgr;
			void DrawDebugWindow() override;
		};

	public:
		TextureAssetManager(GraphicsDevice* device);
		bool IsFormatSupported(const char* format)const override { return strcmp(format, ".png") == 0 || strcmp(format, ".jpg") == 0 || strcmp(format, ".jpeg") == 0 || strcmp(format, "bmp") == 0 || strcmp(format, "tga") == 0; }
		void Init() override;
		int GetAssetType() const override { return (int)AssetType::TEXTURE; }
	protected:
		Asset* CreateAsset(const CzuchStr& name, BaseCreateSettings& settings) override;
		Asset* CreateAsset(const CzuchStr& path, BaseLoadSettings& settings) override;
		Asset* CreateLoadableAsset(const CzuchStr& path) override;
	private:
		GraphicsDevice* m_Device;
		DynamicGlobalTexturesDebugWindow m_DebugWindow;
	};


	inline Asset* TextureAssetManager::CreateAsset(const CzuchStr& path, BaseLoadSettings& settings)
	{
		auto& settingsTyped = static_cast<TextureLoadSettings&>(settings);
		TextureAsset* texRes = new TextureAsset(path, settingsTyped, m_Device,AssetsManager::GetPtr());
		StringID strId = StringID::MakeStringID(path);
		RegisterAsset(strId, texRes);
		return texRes;
	}

	inline Asset* TextureAssetManager::CreateLoadableAsset(const CzuchStr& path)
	{
		TextureLoadSettings settings;
		TextureAsset* texRes = new TextureAsset(path, settings, m_Device, AssetsManager::GetPtr());
		StringID strId = StringID::MakeStringID(path);
		RegisterAsset(strId, texRes);
		return texRes;
	}

}

