#include "czpch.h"
#include "BuildInAssets.h"
#include"./Renderer/GraphicsDevice.h"
#include"AssetsManager.h"
#include"Asset/TextureAsset.h"
#include"Asset/MaterialAsset.h"
#include"Asset/ModelAsset.h"
#include"Asset/ShaderAsset.h"


namespace Czuch
{
	AssetHandle DefaultAssets::CUBE_ASSET;
	AssetHandle DefaultAssets::PLANE_ASSET;
	AssetHandle DefaultAssets::PLANE_GRID_ASSET;

	MaterialHandle DefaultAssets::DEFAULT_SIMPLE_MATERIAL;
	AssetHandle DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET;

	TextureHandle DefaultAssets::WHITE_TEXTURE;
	AssetHandle DefaultAssets::WHITE_TEXTURE_ASSET;

	BuildInAssets::BuildInAssets(GraphicsDevice* device, AssetsManager* mgr) :m_Device(device), m_AssetsMgr(mgr)
	{

	}

	void BuildInAssets::BuildAndLoad()
	{
		CreateDefaultTextures();
		CreateDefaultMaterials();
		CreateDefaultModels();
	}


	void BuildInAssets::CreateDefaultTextures()
	{
		std::vector<U8> colors1;
		colors1.reserve(128 * 128 * 4);

		for (size_t i = 0; i < 128; i++)
		{
			for (size_t j = 0; j < 128; j++)
			{
				colors1.push_back(255);
				colors1.push_back(255);
				colors1.push_back(255);
				colors1.push_back(255);
			}
		}

		TextureCreateSettings createTexSet;
		createTexSet.channels = 4;
		createTexSet.height = 128;
		createTexSet.width = 128;
		createTexSet.colors = colors1;

		DefaultAssets::WHITE_TEXTURE_ASSET = (m_AssetsMgr->CreateAsset<TextureAsset, TextureCreateSettings>("White", createTexSet));
		DefaultAssets::WHITE_TEXTURE = m_AssetsMgr->GetAsset<TextureAsset>(DefaultAssets::WHITE_TEXTURE_ASSET)->GetTextureAssetHandle();
	}

	void BuildInAssets::CreateDefaultMaterials()
	{
		auto handleVS = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("/Shaders/vertShader.vert", {});
		auto handlePS = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("/Shaders/fragShader.frag", {});

		PipelineStateDesc desc;
		desc.vs = handleVS;
		desc.ps = handlePS;
		desc.pt = PrimitiveTopology::TRIANGLELIST;
		desc.rs.cull_mode = CullMode::BACK;
		desc.rs.fill_mode = PolygonMode::SOLID;
		desc.dss.depth_enable = true;
		desc.dss.depth_func = CompFunc::LESS_EQUAL;
		desc.dss.depth_write_mask = DepthWriteMask::ZERO;
		desc.dss.stencil_enable = false;

		desc.il.AddStream({ .binding = 0,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });
		desc.il.AddStream({ .binding = 1,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });
		desc.il.AddStream({ .binding = 2,.stride = sizeof(float) * 2,.input_rate = InputClassification::PER_VERTEX_DATA });

		desc.il.AddAttribute({ .location = 0,.binding = 0,.offset = 0,.format = Format::R32G32B32_FLOAT });
		desc.il.AddAttribute({ .location = 1,.binding = 1,.offset = 0,.format = Format::R32G32B32_FLOAT });
		desc.il.AddAttribute({ .location = 2,.binding = 2,.offset = 0,.format = Format::R32G32_FLOAT });


		MaterialDesc matDesc;
		matDesc.pipelineDesc = std::move(desc);
		matDesc.materialName = "DefaultMaterial";

		DescriptorSetLayoutDesc desc_SceneData{};
		desc_SceneData.shaderStage = (U32)ShaderStage::PS | (U32)ShaderStage::VS;
		desc_SceneData.AddBinding("SceneData", DescriptorType::UNIFORM_BUFFER, 0, 1, sizeof(SceneData));

		DescriptorSetLayoutDesc desc_tex{};
		desc_tex.shaderStage = (U32)ShaderStage::PS;
		desc_tex.AddBinding("MainTexture", DescriptorType::SAMPLER, 0, 1, 0);

		DescriptorSetDesc setDesc_SceneData;
		DescriptorSetDesc setDesc_Tex;

		setDesc_SceneData.AddBuffer(INVALID_HANDLE(BufferHandle), 0);
		setDesc_Tex.AddSampler(DefaultAssets::WHITE_TEXTURE, 0);

		matDesc.AddLayoutWithDescriptor(desc_SceneData, setDesc_SceneData);
		matDesc.AddLayoutWithDescriptor(desc_tex, setDesc_Tex);

		MaterialCreateSettings createSettings;
		createSettings.desc = std::move(matDesc);

		DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET = m_AssetsMgr->CreateAsset<MaterialAsset, MaterialCreateSettings>(createSettings.desc.materialName, createSettings);
		auto materialAsset = m_AssetsMgr->GetAsset<MaterialAsset>(DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET);
		materialAsset->SetPersistentStatus(true);
		DefaultAssets::DEFAULT_SIMPLE_MATERIAL = materialAsset->GetMaterialAssetHandle();
	}

	std::vector<Vec3> GetCubeMeshPositions(float size) {

		return {
			{-size, -size, size},
			{size, -size, size},
			{size, size, size},
			{-size, size, size},
			{-size, -size, -size},
			{-size, size, -size},
			{size, size, -size},
			{size, -size, -size},
			{-size, size, -size},
			{-size, size, size},
			{size, size, size},
			{size, size, -size},
			{-size, -size, -size},
			{size, -size, -size},
			{size, -size, size},
			{-size, -size, size},
			{size, -size, -size},
			{size, size, -size},
			{size, size, size},
			{size, -size, size},
			{-size, -size, -size},
			{-size, -size, size},
			{-size, size, size},
			{-size, size, -size},
		};
	}


	std::vector<Vec4> GetCubeMeshUvs()
	{
		return {
			  {0.0f, 0.0f,0,0}, {1.0f, 0.0f,0,0}, {1.0f, 1.0f,0,0}, {0.0f, 1.0f,0,0}, // Front face
			  {0.0f, 0.0f,0,0}, {1.0f, 0.0f,0,0}, {1.0f, 1.0f,0,0}, {0.0f, 1.0f,0,0}, // Back face
			  {0.0f, 0.0f,0,0}, {1.0f, 0.0f,0,0}, {1.0f, 1.0f,0,0}, {0.0f, 1.0f,0,0}, // Top face
			  {0.0f, 0.0f,0,0}, {1.0f, 0.0f,0,0}, {1.0f, 1.0f,0,0}, {0.0f, 1.0f,0,0}, // Bottom face
			  {0.0f, 0.0f,0,0}, {1.0f, 0.0f,0,0}, {1.0f, 1.0f,0,0}, {0.0f, 1.0f,0,0}, // Right face
			  {0.0f, 0.0f,0,0}, {1.0f, 0.0f,0,0}, {1.0f, 1.0f,0,0}, {0.0f, 1.0f,0,0}  // Left face
		};
	}

	std::vector<Vec3> GetCubeMeshNormals()
	{
		return {
			// Front face
			{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
			// Back face
			{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
			// Top face
			{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
			// Bottom face
			{0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},
			// Right face
			{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
			// Left face
			{-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}
		};
	}

	std::vector<Vec4> GetCubeMeshColors() {
		return {
			{1.0f, 0.0f, 0.0f,1}, {1.0f, 0.0f, 0.0f,1}, {1.0f, 0.0f, 0.0f,1}, {1.0f, 0.0f, 0.0f,1}, // Front face (red)
			{0.0f, 1.0f, 0.0f,1}, {0.0f, 1.0f, 0.0f,1}, {0.0f, 1.0f, 0.0f,1}, {0.0f, 1.0f, 0.0f,1}, // Back face (green)
			{0.0f, 0.0f, 1.0f,1}, {0.0f, 0.0f, 1.0f,1}, {0.0f, 0.0f, 1.0f,1}, {0.0f, 0.0f, 1.0f,1}, // Top face (blue)
			{1.0f, 1.0f, 0.0f,1}, {1.0f, 1.0f, 0.0f,1}, {1.0f, 1.0f, 0.0f,1}, {1.0f, 1.0f, 0.0f,1}, // Bottom face (yellow)
			{1.0f, 0.0f, 1.0f,1}, {1.0f, 0.0f, 1.0f,1}, {1.0f, 0.0f, 1.0f,1}, {1.0f, 0.0f, 1.0f,1}, // Right face (magenta)
			{0.0f, 1.0f, 1.0f,1}, {0.0f, 1.0f, 1.0f,1}, {0.0f, 1.0f, 1.0f,1}, {0.0f, 1.0f, 1.0f,1}  // Left face (cyan)
		};
	}

	std::vector<U32> getCubeIndices() {
		return {
			// Front face
			0, 1, 2,
			2, 3, 0,

			// Back face
			4, 5, 6,
			6, 7, 4,

			// Top face
			8, 9, 10,
			10, 11, 8,

			// Bottom face
			12, 13, 14,
			14, 15, 12,

			// Right face
			16, 17, 18,
			18, 19, 16,

			// Left face
			20, 21, 22,
			22, 23, 20
		};
	}

	void BuildInAssets::CreateDefaultModels()
	{
		//Plane model simple
		const std::vector<Vec3> positions = {
			{-0.5f, 0.0f,0.5f},
			{0.5f, 0.0f,-0.5f},
			{0.5f, 0.0f,0.5f},
			{-0.5f, 0.0f,-0.5f},
		};

		const std::vector<Vec3> normals = {
			{0.0f, 1.0f,0.0f},
			{0.0f, 1.0f,0.0f},
			{0.0f, 1.0f,0.0f},
			{0.0f, 1.0f,0.0f},
		};

		std::vector<Vec4> colors = {
			{1.0f,1.0f, 1.0f,1.0f},
			{1.0f, 1.0f, 1.0f,1.0f},
			{1.0f, 1.0f, 1.0f,1.0f},
			{1.0f, 1.0f, 1.0f,1.0f},
		};


		const std::vector<Vec4> uvs = {
			{0.0f, 1.0f,0,0},
			{1.0f, 0.0f,0,0},
			{1.0f, 1.0f,0,0},
			{0.0f, 0.0f,0,0},
		};

		const std::vector<U32> indices = { 0, 1, 2, 3, 1, 0 };

		MeshData planeData;
		planeData.colors = std::move(colors);
		planeData.indices = std::move(indices);
		planeData.positions = std::move(positions);
		planeData.normals = std::move(normals);
		planeData.uvs0 = std::move(uvs);
		planeData.meshName = "PlaneMeshSimple";
		planeData.material = DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET;

		ModelCreateSettings createSettings{};
		createSettings.modelName = "PlaneModel";
		createSettings.permamentAsset = true;
		createSettings.meshesData.push_back(std::move(planeData));


		DefaultAssets::PLANE_ASSET = m_AssetsMgr->CreateAsset<ModelAsset>("Plane", createSettings);

		//Cube mesh
		MeshData cubeData;
		cubeData.colors = std::move(GetCubeMeshColors());
		cubeData.indices = std::move(getCubeIndices());
		cubeData.positions = std::move(GetCubeMeshPositions(0.5f));
		cubeData.normals = std::move(GetCubeMeshNormals());
		cubeData.uvs0 = std::move(GetCubeMeshUvs());
		cubeData.meshName = "CubeMeshSimple";
		cubeData.material = DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET;
		

		ModelCreateSettings createSettingsCube{};
		createSettingsCube.modelName = "CubeModel";
		createSettingsCube.permamentAsset = true;
		createSettingsCube.meshesData.push_back(std::move(cubeData));
		

		DefaultAssets::CUBE_ASSET = m_AssetsMgr->CreateAsset<ModelAsset>("Cube", createSettingsCube);

	}
}