#pragma once
#include "Asset.h"
#include "./Subsystems/Assets/LoadSettingsPerType.h"

namespace Czuch
{
    class GraphicsDevice;

    class TextureAsset :
        public Asset
    {
    public:
        TextureAsset(const CzuchStr& path,TextureLoadSettings* settings, GraphicsDevice* device);
        ~TextureAsset();
        bool LoadAsset() override;
        void UnloadAsset() override;
        inline TextureHandle GetTextureAsset() const { return m_TextureAsset; }
    private:
        GraphicsDevice* m_Device;
        TextureLoadSettings* m_Settings;
        TextureHandle m_TextureAsset;
    };

}
