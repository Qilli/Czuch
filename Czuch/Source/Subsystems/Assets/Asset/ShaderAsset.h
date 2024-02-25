#pragma once
#include"Asset.h"
#include"Renderer/Graphics.h"
#include"Subsystems/Assets/SettingsPerType.h"
#include<vector>


namespace Czuch
{
    class Shader;
    class GraphicsDevice;
    struct ShaderCreateSettings;

    class ShaderAsset :public Asset
    {
    public:
        ShaderAsset(const CzuchStr& path, GraphicsDevice* device, ShaderLoadSettings &loadSettings, AssetsManager* assetsManager);
        ShaderAsset(const CzuchStr& path, GraphicsDevice* device, ShaderCreateSettings& settings, AssetsManager* assetsManager);
        ~ShaderAsset();
        bool LoadAsset() override;
        bool UnloadAsset() override;
        bool CreateFromData() override;
        inline ShaderHandle GetShaderAssetHandle() const { return m_ShaderAsset; }
    private:
        GraphicsDevice* m_Device;
        ShaderHandle m_ShaderAsset;
        ShaderCreateSettings m_ShaderCreateSettings;
        ShaderLoadSettings m_ShaderLoadSettings;
        std::vector<char> m_ShaderCode;
    };

}

