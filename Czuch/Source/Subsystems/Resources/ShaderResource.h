#pragma once
#include"Resource.h"
#include<vector>

namespace Czuch
{
    class Shader;
    class GraphicsDevice;

    class ShaderResource :public Resource
    {
    public:
        ShaderResource(const CzuchStr& path, GraphicsDevice* device);
        ~ShaderResource();
        bool LoadResource() override;
        void UnloadResource() override;
        inline Shader* GetShaderAsset() const { return m_ShaderAsset; }
    private:
        GraphicsDevice* m_Device;
        Shader* m_ShaderAsset;
        std::vector<char> m_ShaderCode;
    };

}

