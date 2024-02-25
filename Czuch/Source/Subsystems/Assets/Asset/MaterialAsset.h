#pragma once
#include "Asset.h"
#include "./Subsystems/Assets/SettingsPerType.h"

namespace Czuch
{
    class GraphicsDevice;

	class MaterialAsset: public Asset
	{
    public:
        MaterialAsset(const CzuchStr& path, GraphicsDevice* device, MaterialLoadSettings& loadSettings,AssetsManager* assetsManager);
        MaterialAsset(const CzuchStr& path, GraphicsDevice* device, MaterialCreateSettings& settings, AssetsManager* assetsManager);
        ~MaterialAsset();
        bool LoadAsset() override;
        bool UnloadAsset() override;
        bool CreateFromData() override;
        inline MaterialHandle GetMaterialAssetHandle() const { return m_MaterialAsset; }
    private:
        GraphicsDevice* m_Device;
        MaterialHandle m_MaterialAsset;
        MaterialCreateSettings m_MaterialCreateSettings;
        MaterialLoadSettings m_MaterialLoadSettings;
        std::vector<char> m_MaterialBuffer;
	};
}


