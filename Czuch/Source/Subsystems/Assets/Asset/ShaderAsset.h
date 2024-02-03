#pragma once
#include"Asset.h"
#include"Renderer/Graphics.h"
#include<vector>


namespace Czuch
{
    class Shader;
    class GraphicsDevice;

    class ShaderAsset :public Asset
    {
    public:
        ShaderAsset(const CzuchStr& path, GraphicsDevice* device);
        ~ShaderAsset();
        bool LoadAsset() override;
        void UnloadAsset() override;
        inline ShaderHandle GetShaderAsset() const { return m_ShaderAsset; }
    private:
        GraphicsDevice* m_Device;
        ShaderHandle m_ShaderAsset;
        std::vector<char> m_ShaderCode;
    };

}

