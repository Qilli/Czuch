#pragma once
#include "Asset.h"
#include "./Subsystems/Assets/SettingsPerType.h"

namespace Czuch
{
    class GraphicsDevice;

	class CZUCH_API MaterialAsset: public Asset
	{
    public:
        MaterialAsset(const CzuchStr& path, GraphicsDevice* device, MaterialLoadSettings& loadSettings,AssetsManager* assetsManager);
        MaterialAsset(const CzuchStr& path, GraphicsDevice* device, MaterialCreateSettings& settings, AssetsManager* assetsManager);
        ~MaterialAsset();
        bool LoadAsset() override;
        bool UnloadAsset() override;
        bool CreateFromData() override;
        inline MaterialHandle GetMaterialResourceHandle() const { return m_MaterialResource; }
        void CopyMaterialDescTo(MaterialDesc& desc);
        bool IsTransparent() const { return m_MaterialCreateSettings.desc.IsTransparent(); }
    private:
        GraphicsDevice* m_Device;
        MaterialHandle m_MaterialResource;
        MaterialCreateSettings m_MaterialCreateSettings;
        MaterialLoadSettings m_MaterialLoadSettings;
        std::vector<char> m_MaterialBuffer;
	};
}


