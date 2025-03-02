#pragma once
#include "Asset.h"
#include "./Subsystems/Assets/SettingsPerType.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Core/TreeNode.h"
#include <unordered_map>

namespace Czuch
{
    class GraphicsDevice;
	class Scene;
    struct Entity;

    Mat4x4 ConvertMatrix(const aiMatrix4x4& m);

    class CZUCH_API ModelAsset :
        public Asset
    {
    public:
        ModelAsset(const CzuchStr& path,ModelLoadSettings& settings, GraphicsDevice* device,AssetsManager* assetsManager);
        ModelAsset(const CzuchStr& path, ModelCreateSettings& settings, GraphicsDevice* device,AssetsManager* assetsManager);
        ~ModelAsset();
        bool LoadAsset() override;
        bool UnloadAsset() override;
        bool CreateFromData() override;
        inline MeshHandle GetMeshHandle(int index) const { return m_MeshData.GetMeshHandleAt(index); }
		inline MeshHandle GetMeshHandle(const CzuchStr& name) const { return m_MeshData.GetMeshHandleWithName(name); }
        inline TreeNode<MeshTreeNodeElement>& GetHierarchy() { return m_MeshData.meshesHierarchy; }
        inline U32 GetMeshesCount() const { return m_MeshData.GetMeshesCount(); }
		const CzuchStr* GetMeshName(MeshHandle handle) const;
        Entity AddModelToScene(Scene* scene,Entity parent);
    public:
        ShortAssetInfo* GetShortAssetInfo() override;
        CzuchStr GetAssetLoadInfo() const override;
	private:
		void ProcessNode(aiNode* node, const aiScene* scene, TreeNode<MeshTreeNodeElement>* currentNode);
		U32 ProcessMesh(aiMesh* mesh, const aiScene* scene);
        MaterialInfo ProcessMaterial(aiMaterial* material);
        AssetHandle CheckAndLoadTexture(aiMaterial* material, aiTextureType type, U32 index);
        Entity AddMeshObjectRecursive(Scene* scene, Entity parent, TreeNode<MeshTreeNodeElement>* currentNode);
    private:
        GraphicsDevice* m_Device;
        ModelLoadSettings m_CurrentLoadSettings;
        ModelCreateSettings  m_CreateSettings;
		MeshDataContainer m_MeshData;
		Array<ShortAssetInfo> m_AdditionalMeshesInfo;
		std::unordered_map<aiMaterial*,MaterialInfo> m_LoadedMaterials;
    };

}
