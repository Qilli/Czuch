#pragma once
#include "Asset.h"
#include "./Subsystems/Assets/SettingsPerType.h"

namespace Czuch
{
    class GraphicsDevice;

    class CZUCH_API TextureAsset :
        public Asset
    {
    public:
        TextureAsset(const CzuchStr& path,TextureLoadSettings& settings, GraphicsDevice* device, AssetsManager* assetsManager);
        TextureAsset(const CzuchStr& path, TextureCreateSettings& settings, GraphicsDevice* device, AssetsManager* assetsManager);
        ~TextureAsset();
        bool LoadAsset() override;
        bool UnloadAsset() override;
        bool CreateFromData() override;
        inline TextureHandle GetTextureResourceHandle() const { return m_TextureResource; }
    private:
        GraphicsDevice* m_Device;
        TextureLoadSettings m_CurrentLoadSettings;
        TextureCreateSettings m_CreateSettings;
        TextureHandle m_TextureResource;
    };

}
