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
    private:
        GraphicsDevice* m_Device;
        MaterialInstanceHandle m_MaterialInstanceResource;
        MaterialInstanceCreateSettings m_MaterialCreateSettings;
        MaterialInstanceLoadSettings m_MaterialLoadSettings;
        std::vector<char> m_MaterialInstanceBuffer;
	};
}


