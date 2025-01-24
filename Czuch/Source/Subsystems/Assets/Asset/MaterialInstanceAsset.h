#pragma once
#include "Asset.h"
#include "./Subsystems/Assets/SettingsPerType.h"

namespace Czuch
{
    class GraphicsDevice;

	class CZUCH_API MaterialInstanceAsset: public Asset
	{
    public:
        MaterialInstanceAsset(const CzuchStr& path, GraphicsDevice* device, MaterialInstanceLoadSettings& loadSettings,AssetsManager* assetsManager);
        MaterialInstanceAsset(const CzuchStr& path, GraphicsDevice* device, MaterialInstanceCreateSettings& settings, AssetsManager* assetsManager);
        ~MaterialInstanceAsset();
        bool LoadAsset() override;
        bool UnloadAsset() override;
        bool CreateFromData() override;
        inline MaterialInstanceHandle GetMaterialInstanceResourceHandle() const { return m_MaterialInstanceResource; }
    public:
        ShortAssetInfo* GetShortAssetInfo() override;
        CzuchStr GetAssetLoadInfo() const override;
    private:
		void LoadDependencies();
		void UnloadDependencies();
    private:
        GraphicsDevice* m_Device;
		MaterialInstanceDesc m_MaterialInstanceDesc;
        MaterialInstanceHandle m_MaterialInstanceResource;
        MaterialInstanceCreateSettings m_MaterialCreateSettings;
        MaterialInstanceLoadSettings m_MaterialLoadSettings;
        Array<char> m_MaterialInstanceBuffer;
		Array<TextureHandle> m_Dependencies;
	};
}


