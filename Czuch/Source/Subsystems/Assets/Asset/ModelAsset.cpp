#include "czpch.h"
#include "ModelAsset.h"
#include "./Renderer/GraphicsDevice.h"
#include"../AssetsManager.h"
#include"MaterialAsset.h"
#include"MaterialInstanceAsset.h"
#include "TextureAsset.h"
#include "./Subsystems/Assets/BuildInAssets.h"


namespace Czuch
{


	ModelAsset::ModelAsset(const CzuchStr& path, ModelLoadSettings& loadSettings, GraphicsDevice* device, AssetsManager* assetsManager):Asset(path, GetNameFromPath(path),assetsManager),m_Device(device)
	{
		m_AssetType = AssetModeType::LOADED_TYPE;
		m_CurrentLoadSettings = std::move(loadSettings);
		LOG_BE_INFO("Created new model asset with unloaded state at path: {0}", AssetPath());
	}

	ModelAsset::ModelAsset(const CzuchStr& path, ModelCreateSettings& settings, GraphicsDevice* device, AssetsManager* assetsManager) :Asset(path,path,assetsManager), m_Device(device)
	{
		m_AssetType = AssetModeType::CREATED_TYPE;
		m_CreateSettings = std::move(settings);
		CreateFromData();
	}

	ModelAsset::~ModelAsset()
	{
		m_ForceUnload = true;
		UnloadAsset();
	}

	const CzuchStr* ModelAsset::GetMeshName(MeshHandle handle) const
	{
		Mesh* mesh = m_Device->AccessMesh(handle);
		if (mesh == nullptr)
		{
			return nullptr;
		}
		return &mesh->data->meshName;
	}

	bool ModelAsset::LoadAsset()
	{
		if (Asset::LoadAsset())
		{
			return true;
		}

		if (m_AssetType == AssetModeType::CREATED_TYPE || m_MeshData.inited)
		{
			for (auto& meshData : m_MeshData.meshesData)
			{
				auto meshHandle = m_Device->CreateMesh(meshData.meshData);
				Mesh* mesh = m_Device->AccessMesh(meshHandle);

				m_AssetsMgr->LoadAsset<MaterialInstanceAsset>(meshData.meshData.materialInstanceAssetHandle);
				auto matInstance = m_AssetsMgr->GetAsset<MaterialInstanceAsset>(meshData.meshData.materialInstanceAssetHandle)->GetMaterialInstanceResourceHandle();

				meshData.meshHandle = meshHandle;
				meshData.materialInstanceHandle = matInstance;
			}
			return true;
		}


		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(AssetPath(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG_BE_ERROR("ASSIMP ERROR: {0}", importer.GetErrorString());
			return false;
		}

		m_LoadedMaterials.clear();
		m_MeshData.Clear();
		m_MeshData.inited = true;
		m_MeshData.Reserve(scene->mNumMeshes);
		auto child=m_MeshData.meshesHierarchy.AddChild({-1});
		ProcessNode(scene->mRootNode, scene,child);
		LOG_BE_INFO("Loaded model {0} with {1} meshes and {2} materials", AssetName(), scene->mNumMeshes, scene->mNumMaterials);
		m_RefCounter.Up();
		m_State = AssetInnerState::LOADED;

		return true;
	}

	bool ModelAsset::UnloadAsset()
	{
		if (ShouldUnload())
		{
			for (auto &m : m_MeshData.meshesData)
			{
				m_Device->Release(m.meshHandle);
			}

			for (auto &m : m_MeshData.meshesData)
			{
				m_AssetsMgr->UnloadAsset<MaterialInstanceAsset>(m.meshData.materialInstanceAssetHandle);
			}
			m_State = AssetInnerState::UNLOADED;
			return true;
		}
		return false;
	}

	bool ModelAsset::CreateFromData()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			return true;
		}

		if (m_AssetType == AssetModeType::LOADED_TYPE)
		{
			return false;
		}

		m_MeshData.Clear();
		m_MeshData.Reserve(m_CreateSettings.meshesData.size());
		m_MeshData.inited = true;
		auto child = m_MeshData.meshesHierarchy.AddChild({ -1 });
		for (auto &meshData : m_CreateSettings.meshesData)
		{
			U32 index=m_MeshData.AddMeshData({ meshData,MeshHandle(),MaterialInstanceHandle()}, child);
			auto &currentMeshData = m_MeshData.meshesData[index];
			auto meshHandle= m_Device->CreateMesh(currentMeshData.meshData);
			Mesh* mesh = m_Device->AccessMesh(meshHandle);
			auto matInstance=m_AssetsMgr->GetAsset<MaterialInstanceAsset>(currentMeshData.meshData.materialInstanceAssetHandle)->GetMaterialInstanceResourceHandle();

			currentMeshData.meshHandle = meshHandle;
			currentMeshData.materialInstanceHandle = matInstance;
		}

		m_State = AssetInnerState::LOADED;
		m_RefCounter.Up();
		return true;
	}

	ShortAssetInfo* ModelAsset::GetShortAssetInfo()
	{
		if (m_ShortInfo.name == nullptr || m_ShortInfo.name->empty())
		{
			m_ShortInfo.name = &AssetName();
			m_ShortInfo.type = AssetType::MESH;
			m_ShortInfo.asset = GetHandle();
			m_ShortInfo.resource = Invalid_Handle_Id;

			ShortAssetInfo** last = &m_ShortInfo.next;

			m_AdditionalMeshesInfo.reserve(m_MeshData.meshesData.size());

			for (auto& meshData : m_MeshData.meshesData) {
				m_AdditionalMeshesInfo.emplace_back();
				ShortAssetInfo& meshInfo = m_AdditionalMeshesInfo.back();

				meshInfo.name = &meshData.meshData.meshName;
				meshInfo.type = AssetType::MESH;
				meshInfo.asset = GetHandle();
				meshInfo.resource = meshData.meshHandle.handle;
				meshInfo.next = nullptr;

				*last = &meshInfo;
				last = &meshInfo.next;
			}
			*last = nullptr;
		}

		return &m_ShortInfo;
	}

	CzuchStr ModelAsset::GetAssetLoadInfo() const
	{
		return "ModelAsset: " + AssetName() + " Ref count: "+m_RefCounter.GetCountString() + " State: " + (m_State == AssetInnerState::LOADED ? " Loaded" : "Unloaded");
	}

	void ModelAsset::ProcessNode(aiNode* node, const aiScene* scene, TreeNode<Array<I32>>* currentNode)
	{
		for (U32 i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			auto index = ProcessMesh(mesh, scene);
			currentNode->GetData().push_back(index);
		}

		for (U32 i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, currentNode->AddChild({}));
		}
	}

	U32 ModelAsset::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		MeshData meshData;
		meshData.Reserve(mesh->mNumVertices);
		meshData.meshName = mesh->mName.C_Str();
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			meshData.positions.push_back({ mesh->mVertices[i].x,mesh->mVertices[i].y,mesh->mVertices[i].z });
			meshData.normals.push_back({ mesh->mNormals[i].x,mesh->mNormals[i].y,mesh->mNormals[i].z });

			if (mesh->mTextureCoords[0])
			{
				meshData.uvs0.push_back({ mesh->mTextureCoords[0][i].x,mesh->mTextureCoords[0][i].y,0,0 });
			}
			else
			{
				meshData.uvs0.push_back({ 0.0f,0.0f,0.0f,0.0f });

			}
		}

		meshData.indices.reserve(mesh->mNumFaces * 3);

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace &face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				meshData.indices.push_back(face.mIndices[j]);
			}
		}

		MaterialInfo materialInfo = {};
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			materialInfo = ProcessMaterial(material);
			meshData.materialInstanceAssetHandle = materialInfo.materialAsset;
		}

		U32 index= m_MeshData.AddMeshData({ std::move(meshData),MeshHandle(),materialInfo.materialInstanceHandle}, nullptr);
		auto &currentMeshData = m_MeshData.meshesData[index];
		auto meshHandle = m_Device->CreateMesh(currentMeshData.meshData);
		m_MeshData.meshesData[index].meshHandle = meshHandle;

		return index;
	}

	AssetHandle ModelAsset::CheckAndLoadTexture(aiMaterial* material, aiTextureType type, U32 index)
	{
		aiString str;
		material->GetTexture(type, index, &str);
		LOG_BE_INFO("Checking if texture {0} already exist and is loaded", str.C_Str());
		auto texPath = GetRelativePath().parent_path().string() +"\\"+ str.C_Str();
		return AssetsManager::GetPtr()->LoadAsset<TextureAsset>(texPath);
	}

	MaterialInfo ModelAsset::ProcessMaterial(aiMaterial* material)
	{
		MaterialInfo info;

		if (m_LoadedMaterials.find(material) != m_LoadedMaterials.end())
		{
			return m_LoadedMaterials[material];
		}

		U32 diffuseCount=material->GetTextureCount(aiTextureType_DIFFUSE);

		if (diffuseCount == 0)
		{
			LOG_BE_ERROR("Material has no diffuse texture");
			info.materialInstanceHandle = DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE;
			info.materialAsset = DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET;
			m_LoadedMaterials[material] = info;
			return info;
		}
		else
		{
			AssetHandle diffuseTextureHandle = CheckAndLoadTexture(material, aiTextureType_DIFFUSE, 0);
			if (diffuseTextureHandle.IsValid())
			{
				const char* nameMat = material->GetName().C_Str();
				CzuchStr name = "DefaultMaterial_0_";
				name += nameMat;

				TextureHandle textureHandle = AssetsManager::GetPtr()->GetAsset<TextureAsset>(diffuseTextureHandle)->GetTextureResourceHandle();
			
				MaterialInstanceCreateSettings instanceCreateSettings{};
				instanceCreateSettings.materialInstanceName = std::move(name);
				instanceCreateSettings.desc.AddSampler("MainTexture", textureHandle);
				instanceCreateSettings.desc.materialAsset = DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET;
				instanceCreateSettings.desc.isTransparent = false;

				AssetHandle instanceAssetHandle = m_AssetsMgr->CreateAsset<MaterialInstanceAsset, MaterialInstanceCreateSettings>(instanceCreateSettings.materialInstanceName, instanceCreateSettings);
				MaterialInstanceAsset* instanceAsset = m_AssetsMgr->GetAsset<MaterialInstanceAsset>(instanceAssetHandle);
				
				info.materialInstanceHandle = instanceAsset->GetMaterialInstanceResourceHandle();
				info.materialAsset = instanceAssetHandle;
				m_LoadedMaterials[material] = info;
				return info;
			}
			else
			{
				LOG_BE_ERROR("Failed to load diffuse texture");
				info.materialInstanceHandle = DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE;
				info.materialAsset = DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET;
				m_LoadedMaterials[material] = info;
				return info;
			}

		}

		//[TODO] add support for multiple textures, by texture existence we also select proper material type
		/*U32 specularCount = material->GetTextureCount(aiTextureType_SPECULAR);
		U32 normalCount = material->GetTextureCount(aiTextureType_NORMALS);
		U32 heightCount = material->GetTextureCount(aiTextureType_HEIGHT);
		U32 ambientCount = material->GetTextureCount(aiTextureType_AMBIENT);
		U32 emissiveCount = material->GetTextureCount(aiTextureType_EMISSIVE);
		U32 shininessCount = material->GetTextureCount(aiTextureType_SHININESS);
		U32 opacityCount = material->GetTextureCount(aiTextureType_OPACITY);
		U32 displacementCount = material->GetTextureCount(aiTextureType_DISPLACEMENT);
		U32 lightmapCount = material->GetTextureCount(aiTextureType_LIGHTMAP);
		U32 reflectionCount = material->GetTextureCount(aiTextureType_REFLECTION);*/

	}
}
