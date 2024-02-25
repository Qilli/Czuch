#pragma once
#include "Asset.h"
#include "./Subsystems/Assets/SettingsPerType.h"

namespace Czuch
{
    class GraphicsDevice;

    class ModelAsset :
        public Asset
    {
    public:
        ModelAsset(const CzuchStr& path,ModelLoadSettings& settings, GraphicsDevice* device,AssetsManager* assetsManager);
        ModelAsset(const CzuchStr& path, ModelCreateSettings& settings, GraphicsDevice* device,AssetsManager* assetsManager);
        ~ModelAsset();
        bool LoadAsset() override;
        bool UnloadAsset() override;
        bool CreateFromData() override;
        inline MeshHandle GetMeshHandle(int index) const { return m_Meshes[index]; }
        inline U32 GetMeshesCount() const { return m_Meshes.size(); }
    private:
        GraphicsDevice* m_Device;
        ModelLoadSettings m_CurrentLoadSettings;
        ModelCreateSettings m_CreateSettings;
        std::vector<MeshHandle> m_Meshes;
        std::vector<AssetHandle> m_Materials;
    };

}
