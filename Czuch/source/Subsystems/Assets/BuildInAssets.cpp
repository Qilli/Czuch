#include "czpch.h"
#include "BuildInAssets.h"
#include"./Renderer/GraphicsDevice.h"
#include"AssetsManager.h"
#include"Asset/TextureAsset.h"
#include"Asset/MaterialAsset.h"
#include"Asset/ModelAsset.h"
#include"Asset/ShaderAsset.h"
#include"Asset/MaterialInstanceAsset.h"
#include"Renderer/Graphics.h"
#include"Renderer/DebugDraw.h"

namespace Czuch
{
	AssetHandle DefaultAssets::CUBE_ASSET;
	MeshHandle DefaultAssets::CUBE_HANDLE;
	AssetHandle DefaultAssets::PLANE_ASSET;
	MeshHandle DefaultAssets::PLANE_HANDLE;
	AssetHandle DefaultAssets::PLANE_GRID_ASSET;
	MeshHandle DefaultAssets::PLANE_GRID_HANDLE;

	MaterialHandle DefaultAssets::DEFAULT_SIMPLE_MATERIAL;
	MaterialInstanceHandle DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE;
	AssetHandle DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET;
	AssetHandle DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET;

	MaterialHandle DefaultAssets::DEFAULT_SIMPLE_TRANSPARENT_MATERIAL;
	MaterialInstanceHandle DefaultAssets::DEFAULT_SIMPLE_TRANSPARENT_MATERIAL_INSTANCE;
	AssetHandle DefaultAssets::DEFAULT_SIMPLE_TRANSPARENT_MATERIAL_INSTANCE_ASSET;
	AssetHandle DefaultAssets::DEFAULT_SIMPLE_TRANSPARENT_MATERIAL_ASSET;

	MaterialHandle DefaultAssets::DEBUG_DRAW_MATERIAL;
	AssetHandle DefaultAssets::DEBUG_DRAW_MATERIAL_ASSET;
	MaterialInstanceHandle DefaultAssets::DEBUG_DRAW_MATERIAL_INSTANCE;
	AssetHandle DefaultAssets::DEBUG_DRAW_MATERIAL_INSTANCE_ASSET;
	AssetHandle DefaultAssets::DEBUG_DRAW_LIGHT_MATERIAL_INSTANCE_ASSET;
	MaterialInstanceHandle DefaultAssets::DEBUG_DRAW_LIGHT_MATERIAL_INSTANCE;


	AssetHandle DefaultAssets::DEBUG_DRAW_LINES_MATERIAL_ASSET;
	MaterialInstanceHandle DefaultAssets::DEBUG_DRAW_LINES_MATERIAL_INSTANCE;

	AssetHandle DefaultAssets::DEBUG_DRAW_TRIANGLES_MATERIAL_ASSET;
	MaterialInstanceHandle DefaultAssets::DEBUG_DRAW_TRIANGLES_MATERIAL_INSTANCE;

	AssetHandle DefaultAssets::DEBUG_DRAW_POINTS_MATERIAL_ASSET;
	MaterialInstanceHandle DefaultAssets::DEBUG_DRAW_POINTS_MATERIAL_INSTANCE;

	MaterialHandle DefaultAssets::DEPTH_PREPASS_MATERIAL;
	MaterialInstanceHandle DefaultAssets::DEPTH_PREPASS_MATERIAL_INSTANCE;
	AssetHandle DefaultAssets::DEPTH_PREPASS_MATERIAL_INSTANCE_ASSET;
	AssetHandle DefaultAssets::DEPTH_PREPASS_MATERIAL_ASSET;

	MaterialHandle DefaultAssets::FINAL_PASS_MATERIAL;
	MaterialInstanceHandle DefaultAssets::FINAL_PASS_MATERIAL_INSTANCE;
	AssetHandle DefaultAssets::FINAL_PASS_MATERIAL_INSTANCE_ASSET;
	AssetHandle DefaultAssets::FINAL_PASS_MATERIAL_ASSET;

	MaterialHandle DefaultAssets::DEPTH_LINEAR_PREPASS_MATERIAL;
	MaterialInstanceHandle DefaultAssets::DEPTH_LINEAR_PREPASS_MATERIAL_INSTANCE;
	AssetHandle DefaultAssets::DEPTH_LINEAR_PREPASS_MATERIAL_INSTANCE_ASSET;
	AssetHandle DefaultAssets::DEPTH_LINEAR_PREPASS_MATERIAL_ASSET;

	TextureHandle DefaultAssets::WHITE_TEXTURE;
	AssetHandle DefaultAssets::WHITE_TEXTURE_ASSET;

	TextureHandle DefaultAssets::PINK_TEXTURE;
	AssetHandle DefaultAssets::PINK_TEXTURE_ASSET;

	AssetHandle DefaultAssets::EDITOR_ICON_TRANSLATE;
	AssetHandle DefaultAssets::EDITOR_ICON_ROTATE;
	AssetHandle DefaultAssets::EDITOR_ICON_SCALE;
	AssetHandle DefaultAssets::EDITOR_ICON_FILE;
	AssetHandle DefaultAssets::EDITOR_ICON_FOLDER;
	AssetHandle DefaultAssets::EDITOR_ICON_MODEL;
	AssetHandle DefaultAssets::EDITOR_ICON_MATERIAL;
	AssetHandle DefaultAssets::EDITOR_ICON_TEXTURE;
	AssetHandle DefaultAssets::EDITOR_ICON_SHADER;
	AssetHandle DefaultAssets::EDITOR_ICON_MATERIAL_INSTANCE;

	AssetHandle DefaultAssets::DEFAULT_VS_SHADER_ASSET;
	AssetHandle DefaultAssets::DEFAULT_PS_SHADER_ASSET;

	AssetHandle DefaultAssets::DEBUG_DRAW_VS_SHADER_ASSET;
	AssetHandle DefaultAssets::DEBUG_DRAW_PS_SHADER_ASSET;
	AssetHandle DefaultAssets::DEFAULT_SIMPLE_COLOR_PS_ASSET;
	AssetHandle DefaultAssets::DEBUG_DRAW_VS_INSTANCED_LINES_ASSET;
	AssetHandle DefaultAssets::DEBUG_DRAW_VS_INSTANCED_TRIANGLES_ASSET;
	AssetHandle DefaultAssets::DEBUG_DRAW_VS_INSTANCED_POINTS_ASSET;


	BuildInAssets::BuildInAssets(GraphicsDevice* device, AssetsManager* mgr, EngineMode mode) :m_Device(device), m_AssetsMgr(mgr)
	{
		m_Mode = mode;
	}

	void BuildInAssets::BuildAndLoad()
	{
		CreateDefaultTextures();
		CreateDefaultMaterials();
		CreateDefaultModels();
		LoadUIAssets();
	}


	void BuildInAssets::CreateDefaultTextures()
	{
		//White texture
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
		DefaultAssets::WHITE_TEXTURE = m_AssetsMgr->GetAsset<TextureAsset>(DefaultAssets::WHITE_TEXTURE_ASSET)->GetTextureResourceHandle();
		DefaultAssets::WHITE_TEXTURE.assetHandle = DefaultAssets::WHITE_TEXTURE_ASSET;

		//Pink texture
		std::vector<U8> colorsP;
		colors1.reserve(128 * 128 * 4);

		for (size_t i = 0; i < 128; i++)
		{
			for (size_t j = 0; j < 128; j++)
			{
				colorsP.push_back(255);
				colorsP.push_back(0);
				colorsP.push_back(255);
				colorsP.push_back(255);
			}
		}

		TextureCreateSettings createTexSetP;
		createTexSetP.channels = 4;
		createTexSetP.height = 128;
		createTexSetP.width = 128;
		createTexSetP.colors = colorsP;

		DefaultAssets::PINK_TEXTURE_ASSET = (m_AssetsMgr->CreateAsset<TextureAsset, TextureCreateSettings>("Pink", createTexSetP));
		DefaultAssets::PINK_TEXTURE = m_AssetsMgr->GetAsset<TextureAsset>(DefaultAssets::PINK_TEXTURE_ASSET)->GetTextureResourceHandle();
		DefaultAssets::PINK_TEXTURE.assetHandle = DefaultAssets::PINK_TEXTURE_ASSET;
	}

	void BuildInAssets::CreateDefaultMaterials()
	{
		//Simple material
		auto handleVS = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\vertShader.vert", {});
		auto handlePS = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\fragShader.frag", {});
		//debug draw shader
		DefaultAssets::DEBUG_DRAW_PS_SHADER_ASSET = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\DebugDrawFragmentShader.frag", {});

		DefaultAssets::DEFAULT_VS_SHADER_ASSET = handleVS;
		DefaultAssets::DEFAULT_PS_SHADER_ASSET = handlePS;

		MaterialPassDesc desc;
		desc.vs = handleVS;
		desc.ps = handlePS;
		desc.pt = PrimitiveTopology::TRIANGLELIST;
		desc.rs.cull_mode = CullMode::BACK;
		desc.rs.fill_mode = PolygonMode::SOLID;
		desc.dss.depth_enable = true;
		desc.dss.depth_func = CompFunc::EQUAL;
		desc.dss.depth_write_mask = DepthWriteMask::ZERO;
		desc.dss.depth_write_enable = false;
		desc.dss.stencil_enable = false;
		desc.bindPoint = BindPoint::BIND_POINT_GRAPHICS;
		desc.passType = RenderPassType::ForwardLighting;

		desc.il.AddStream({ .binding = 0,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });
		desc.il.AddStream({ .binding = 1,.stride = sizeof(float) * 4,.input_rate = InputClassification::PER_VERTEX_DATA });
		desc.il.AddStream({ .binding = 2,.stride = sizeof(float) * 4,.input_rate = InputClassification::PER_VERTEX_DATA });
		desc.il.AddStream({ .binding = 3,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });

		desc.il.AddAttribute({ .location = 0,.binding = 0,.offset = 0,.format = Format::R32G32B32_FLOAT });
		desc.il.AddAttribute({ .location = 1,.binding = 1,.offset = 0,.format = Format::R32G32B32A32_FLOAT });
		desc.il.AddAttribute({ .location = 2,.binding = 2,.offset = 0,.format = Format::R32G32B32A32_FLOAT });
		desc.il.AddAttribute({ .location = 3,.binding = 3,.offset = 0,.format = Format::R32G32B32_FLOAT });

		DescriptorSetLayoutDesc desc_SceneData{};
		desc_SceneData.shaderStage = (U32)ShaderStage::PS | (U32)ShaderStage::VS;
		desc_SceneData.AddBinding("SceneData", DescriptorType::UNIFORM_BUFFER, 0, 1, sizeof(SceneData), true);
		desc_SceneData.AddBinding("RenderObjectsData", DescriptorType::STORAGE_BUFFER, 1, INIT_MAX_RENDER_OBJECTS, sizeof(RenderObjectGPUData), true, DescriptorBindingTagType::RENDER_OBJECTS);

		DescriptorSetLayoutDesc desc_LightBuffers{};
		desc_LightBuffers.shaderStage = (U32)ShaderStage::PS;
		desc_LightBuffers.AddBinding("LightBuffer", DescriptorType::STORAGE_BUFFER, 0, 1, sizeof(LightData), true, DescriptorBindingTagType::LIGHTS_CONTAINER);
		desc_LightBuffers.AddBinding("LightIndexBuffer", DescriptorType::STORAGE_BUFFER, 1, 1, sizeof(LightsTileData), true, DescriptorBindingTagType::LIGHTS_INDEXES);
		desc_LightBuffers.AddBinding("TileDataBuffer", DescriptorType::STORAGE_BUFFER, 2, 1, sizeof(U32), true, DescriptorBindingTagType::LIGHTS_TILES);


		DescriptorSetLayoutDesc desc_tex{};
		desc_tex.shaderStage = (U32)ShaderStage::PS;
		desc_tex.AddBinding("MainTexture", DescriptorType::SAMPLER, 0, 1, 0, false);
		desc_tex.AddBinding("Color", DescriptorType::UNIFORM_BUFFER, 1, 1, sizeof(ColorUBO), false);


		UBOLayout uboLayout{};
		uboLayout.AddElement(0, sizeof(ColorUBO), UBOElementType::ColorType, StringID::MakeStringID("Color"));
		desc_tex.SetUBOLayout(uboLayout);

		desc.AddLayout(desc_SceneData);
		desc.AddLayout(desc_LightBuffers);
		desc.AddLayout(desc_tex);

		MaterialDefinitionDesc matDesc(1);
		matDesc.EmplacePass(desc);
		matDesc.materialName = "DefaultMaterial";


		MaterialCreateSettings createSettings;
		createSettings.desc = std::move(matDesc);

		DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET = m_AssetsMgr->CreateAsset<MaterialAsset, MaterialCreateSettings>(createSettings.desc.materialName, createSettings);
		auto materialAsset = m_AssetsMgr->GetAsset<MaterialAsset>(DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET);
		materialAsset->SetPersistentStatus(true);
		DefaultAssets::DEFAULT_SIMPLE_MATERIAL = materialAsset->GetMaterialResourceHandle();

		MaterialInstanceCreateSettings instanceCreateSettings{};
		instanceCreateSettings.materialInstanceName = "DefaultMaterialInstance";
		instanceCreateSettings.desc.AddSampler("MainTexture", DefaultAssets::WHITE_TEXTURE, false);

		ColorUBO colorUbo;
		colorUbo.color = Vec4(1.0f, 1.0f, 1.0f, 1);

		instanceCreateSettings.desc.AddBuffer("Color", Czuch::UBO((void*)&colorUbo, sizeof(ColorUBO)));
		instanceCreateSettings.desc.materialAsset = DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET;
		instanceCreateSettings.desc.isTransparent = false;

		AssetHandle instanceAssetHandle = m_AssetsMgr->CreateAsset<MaterialInstanceAsset, MaterialInstanceCreateSettings>(instanceCreateSettings.materialInstanceName, instanceCreateSettings);
		DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET = instanceAssetHandle;
		MaterialInstanceAsset* instanceAsset = m_AssetsMgr->GetAsset<MaterialInstanceAsset>(instanceAssetHandle);
		DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE = instanceAsset->GetMaterialInstanceResourceHandle();

		CreateDefaultSimpleTransparentMaterial();
		CreateDepthPrePassMaterial();
		CreateFinalPassMaterial();
		CreateDepthLinearPrePassMaterial();
		CreateDebugDrawMaterials();
	}

	std::vector<Vec3> GetCubeMeshPositions(float size) {

		float halfSize = size / 2.0f;
		return {
			// Front face
			{ -halfSize, halfSize, halfSize },
			{ halfSize, halfSize, halfSize },
			{-halfSize, -halfSize, halfSize },
			{ halfSize, -halfSize, halfSize },

			// Back face
			{ halfSize, -halfSize, -halfSize },
			{ halfSize, halfSize, -halfSize },
			{ -halfSize, halfSize, -halfSize },
			{ -halfSize, -halfSize, -halfSize },

			// Top face
			{ -halfSize, halfSize, halfSize },
			{ -halfSize, halfSize, -halfSize },
			{ halfSize, halfSize, -halfSize },
			{ halfSize, halfSize, halfSize },

			// Bottom face
			{ -halfSize, -halfSize, halfSize },
			{  halfSize, -halfSize, halfSize },
			{ halfSize, -halfSize, -halfSize },
			{ -halfSize, -halfSize, -halfSize },

			// Right face
			{ halfSize, -halfSize, halfSize },
			{ halfSize, halfSize, halfSize },
			{ halfSize, halfSize, -halfSize },
			{ halfSize, -halfSize, -halfSize },

			// Left face
			{ -halfSize, -halfSize, halfSize },
			{ -halfSize, -halfSize, -halfSize },
			{ -halfSize, halfSize, -halfSize },
			{ -halfSize, halfSize, halfSize }
		};
	}

	std::vector<Vec4> GetCubeMeshUvs()
	{
		return {
			  {0.0f, 1.0f,0,0}, {1.0f, 1.0f,0,0}, {0.0f, 0.0f,0,0}, {1.0f, 0.0f,0,0}, // Front face
			  {0.0f, 0.0f,0,0}, {0.0f, 1.0f,0,0}, {1.0f, 1.0f,0,0}, {1.0f, 0.0f,0,0}, // Back face
			  {0.0f, 0.0f,0,0}, {1.0f, 0.0f,0,0}, {1.0f, 1.0f,0,0}, {0.0f, 1.0f,0,0}, // Top face
			  {0.0f, 0.0f,0,0}, {0.0f, 1.0f,0,0}, {1.0f, 1.0f,0,0}, {1.0f, 0.0f,0,0}, // Bottom face
			  {0.0f, 0.0f,0,0}, {0.0f, 1.0f,0,0}, {1.0f, 1.0f,0,0}, {1.0f, 0.0f,0,0}, // Right face
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
			{0.0f, 0.0f, 0.0f,1}, {0.0f, 0.0f, 0.0f,1}, {0.0f, 0.0f, 0.0f,1}, {0.0f, 0.0f, 0.0f,1}, // Back face (green)
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
			2, 1, 3,

			// Back face
			4, 5, 6,
			4, 6, 7,

			// Top face
			8, 9, 10,
			8, 10, 11,

			// Bottom face
			12, 13, 14,
			12, 14, 15,

			// Right face
			16, 17, 18,
			16, 18, 19,

			// Left face
			20, 21, 22,
			20, 22, 23
		};
	}

	void BuildInAssets::CreateDefaultModels()
	{
		//Plane model simple
		std::vector<Vec3> positions = {
		   {-0.5f, 0.0f,-0.5f},
		   {0.5f, 0.0f,-0.5f},
		   {0.5f, 0.0f,0.5f},
		   {-0.5f, 0.0f,0.5f},
		};

		std::vector<Vec3> normals = {
			{0.0f, 1.0f,0.0f},
			{0.0f, 1.0f,0.0f},
			{0.0f, 1.0f,0.0f},
			{0.0f, 1.0f,0.0f},
		};

		std::vector<Vec4> colors = {
			{1.0f,0.0f, 0.0f,1.0f},
			{1.0f, 0.0f, 0.0f,1.0f},
			{0.0f, 0.0f, 1.0f,1.0f},
			{0.0f, 1.0f, 0.0f,1.0f},
		};


		std::vector<Vec4> uvs = {
			{0.0f, 1.0f,0,0},
			{1.0f, 1.0f,0,0},
			{1.0f, 0.0f,0,0},
			{0.0f, 0.0f,0,0},
		};


		std::vector<U32> indices = { 0, 1, 2, 3, 0, 2 };

		MeshData planeData;
		planeData.colors = std::move(colors);
		planeData.indices = std::move(indices);
		planeData.positions = std::move(positions);
		planeData.normals = std::move(normals);
		planeData.uvs0 = std::move(uvs);
		planeData.meshName = "PlaneMeshSimple";
		planeData.materialInstanceAssetHandle = DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET;

		ModelCreateSettings createSettings{};
		createSettings.modelName = "PlaneModel";
		createSettings.permamentAsset = true;
		createSettings.meshesData.push_back(std::move(planeData));


		DefaultAssets::PLANE_ASSET = m_AssetsMgr->CreateAsset<ModelAsset>("Plane", createSettings);
		auto planeModel = m_AssetsMgr->GetAsset<ModelAsset>(DefaultAssets::PLANE_ASSET);
		DefaultAssets::PLANE_HANDLE = planeModel->GetMeshHandle(0);

		//Cube mesh
		MeshData cubeData;
		cubeData.colors = std::move(GetCubeMeshColors());
		cubeData.indices = std::move(getCubeIndices());
		cubeData.positions = std::move(GetCubeMeshPositions(0.5f));
		cubeData.normals = std::move(GetCubeMeshNormals());
		cubeData.uvs0 = std::move(GetCubeMeshUvs());
		cubeData.meshName = "CubeMeshSimple";
		cubeData.materialInstanceAssetHandle = DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET;


		ModelCreateSettings createSettingsCube{};
		createSettingsCube.modelName = "CubeModel";
		createSettingsCube.permamentAsset = true;
		createSettingsCube.meshesData.push_back(std::move(cubeData));


		DefaultAssets::CUBE_ASSET = m_AssetsMgr->CreateAsset<ModelAsset>("Cube", createSettingsCube);
		auto cubeModel = m_AssetsMgr->GetAsset<ModelAsset>(DefaultAssets::CUBE_ASSET);
		DefaultAssets::CUBE_HANDLE = cubeModel->GetMeshHandle(0);


	}

	void BuildInAssets::LoadUIAssets()
	{
		if (m_Mode == EngineMode::Editor)
		{
			//Transform icons for editor
			DefaultAssets::EDITOR_ICON_TRANSLATE = m_AssetsMgr->LoadAsset<TextureAsset, TextureLoadSettings>("Editor\\Icons\\Editor_TranslateIcon.png", { .type = TextureDesc::Type::TEXTURE_2D,.isUITexture = true });
			DefaultAssets::EDITOR_ICON_ROTATE = m_AssetsMgr->LoadAsset<TextureAsset, TextureLoadSettings>("Editor\\Icons\\Editor_RotateIcon.png", { .type = TextureDesc::Type::TEXTURE_2D,.isUITexture = true });
			DefaultAssets::EDITOR_ICON_SCALE = m_AssetsMgr->LoadAsset<TextureAsset, TextureLoadSettings>("Editor\\Icons\\Editor_ScaleIcon.png", { .type = TextureDesc::Type::TEXTURE_2D,.isUITexture = true });
			DefaultAssets::EDITOR_ICON_FILE = m_AssetsMgr->LoadAsset<TextureAsset, TextureLoadSettings>("Editor\\Icons\\Editor_FileIcon.png", { .type = TextureDesc::Type::TEXTURE_2D,.isUITexture = true });
			DefaultAssets::EDITOR_ICON_FOLDER = m_AssetsMgr->LoadAsset<TextureAsset, TextureLoadSettings>("Editor\\Icons\\Editor_FolderIcon.png", { .type = TextureDesc::Type::TEXTURE_2D,.isUITexture = true });
			DefaultAssets::EDITOR_ICON_MODEL = m_AssetsMgr->LoadAsset<TextureAsset, TextureLoadSettings>("Editor\\Icons\\Editor_ModelIcon.png", { .type = TextureDesc::Type::TEXTURE_2D,.isUITexture = true });
			DefaultAssets::EDITOR_ICON_MATERIAL = m_AssetsMgr->LoadAsset<TextureAsset, TextureLoadSettings>("Editor\\Icons\\Editor_MaterialIcon.png", { .type = TextureDesc::Type::TEXTURE_2D,.isUITexture = true });
			DefaultAssets::EDITOR_ICON_TEXTURE = m_AssetsMgr->LoadAsset<TextureAsset, TextureLoadSettings>("Editor\\Icons\\Editor_TextureIcon.png", { .type = TextureDesc::Type::TEXTURE_2D,.isUITexture = true });
			DefaultAssets::EDITOR_ICON_SHADER = m_AssetsMgr->LoadAsset<TextureAsset, TextureLoadSettings>("Editor\\Icons\\Editor_ShaderIcon.png", { .type = TextureDesc::Type::TEXTURE_2D,.isUITexture = true });
			DefaultAssets::EDITOR_ICON_MATERIAL_INSTANCE = m_AssetsMgr->LoadAsset<TextureAsset, TextureLoadSettings>("Editor\\Icons\\Editor_MaterialInstanceIcon.png", { .type = TextureDesc::Type::TEXTURE_2D,.isUITexture = true });
		}
	}
	void BuildInAssets::CreateDepthPrePassMaterial()
	{
		//Depth prepass material
		auto depthVS = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\DepthPrepassShader.vert", {});
		auto depthPS = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\EmptyFragmentShader.frag", {});

		MaterialPassDesc desc;
		desc.vs = depthVS;
		desc.ps = depthPS;
		desc.pt = PrimitiveTopology::TRIANGLELIST;
		desc.rs.cull_mode = CullMode::BACK;
		desc.rs.fill_mode = PolygonMode::SOLID;
		desc.dss.depth_enable = true;
		desc.dss.depth_func = CompFunc::LESS_EQUAL;
		desc.dss.depth_write_mask = DepthWriteMask::ZERO;
		desc.dss.stencil_enable = false;
		desc.bindPoint = BindPoint::BIND_POINT_GRAPHICS;
		desc.passType = RenderPassType::DepthPrePass;

		desc.il.AddStream({ .binding = 0,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });

		desc.il.AddAttribute({ .location = 0,.binding = 0,.offset = 0,.format = Format::R32G32B32_FLOAT });


		MaterialDefinitionDesc matDesc(1);
		matDesc.EmplacePass(desc);
		matDesc.materialName = "DepthPrePassMaterial";

		MaterialCreateSettings createSettings;
		createSettings.desc = std::move(matDesc);

		DefaultAssets::DEPTH_PREPASS_MATERIAL_ASSET = m_AssetsMgr->CreateAsset<MaterialAsset, MaterialCreateSettings>(createSettings.desc.materialName, createSettings);
		auto materialAsset = m_AssetsMgr->GetAsset<MaterialAsset>(DefaultAssets::DEPTH_PREPASS_MATERIAL_ASSET);
		materialAsset->SetPersistentStatus(true);
		DefaultAssets::DEPTH_PREPASS_MATERIAL = materialAsset->GetMaterialResourceHandle();

		MaterialInstanceCreateSettings instanceCreateSettings{};
		instanceCreateSettings.materialInstanceName = "DepthPrePassMaterialInstance";
		instanceCreateSettings.desc.materialAsset = DefaultAssets::DEPTH_PREPASS_MATERIAL_ASSET;
		instanceCreateSettings.desc.isTransparent = false;

		AssetHandle instanceAssetHandle = m_AssetsMgr->CreateAsset<MaterialInstanceAsset, MaterialInstanceCreateSettings>(instanceCreateSettings.materialInstanceName, instanceCreateSettings);
		DefaultAssets::DEPTH_PREPASS_MATERIAL_INSTANCE_ASSET = instanceAssetHandle;
		MaterialInstanceAsset* instanceAsset = m_AssetsMgr->GetAsset<MaterialInstanceAsset>(instanceAssetHandle);
		DefaultAssets::DEPTH_PREPASS_MATERIAL_INSTANCE = instanceAsset->GetMaterialInstanceResourceHandle();
	}

	void BuildInAssets::CreateFinalPassMaterial()
	{
		// Final pass material
		auto finalVS = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\VertexFinalPassShader.vert", {});
		auto finalPS = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\FragmentFinalPassShader.frag", {});

		MaterialPassDesc desc;
		desc.vs = finalVS;
		desc.ps = finalPS;
		desc.pt = PrimitiveTopology::TRIANGLELIST;
		desc.rs.cull_mode = CullMode::NONE;
		desc.rs.fill_mode = PolygonMode::SOLID;
		desc.dss.depth_enable = true;
		desc.dss.depth_func = CompFunc::ALWAYS;
		desc.dss.depth_write_mask = DepthWriteMask::ZERO;
		desc.dss.stencil_enable = false;
		desc.bindPoint = BindPoint::BIND_POINT_GRAPHICS;
		desc.passType = RenderPassType::Final;

		DescriptorSetLayoutDesc desc_tex{};
		desc_tex.shaderStage = (U32)ShaderStage::PS;
		desc_tex.AddBinding("MainTexture", DescriptorType::SAMPLER, 0, 1, 0, false);

		desc.AddLayout(desc_tex);

		MaterialDefinitionDesc matDesc(1);
		matDesc.EmplacePass(desc);
		matDesc.materialName = "FinalPassMaterial";

		MaterialCreateSettings createSettings;
		createSettings.desc = std::move(matDesc);

		DefaultAssets::FINAL_PASS_MATERIAL_ASSET = m_AssetsMgr->CreateAsset<MaterialAsset, MaterialCreateSettings>(createSettings.desc.materialName, createSettings);
		auto materialAsset = m_AssetsMgr->GetAsset<MaterialAsset>(DefaultAssets::FINAL_PASS_MATERIAL_ASSET);
		materialAsset->SetPersistentStatus(true);
		DefaultAssets::FINAL_PASS_MATERIAL = materialAsset->GetMaterialResourceHandle();

		MaterialInstanceCreateSettings instanceCreateSettings{};
		instanceCreateSettings.materialInstanceName = "FinalPassMaterialInstance";
		instanceCreateSettings.desc.AddSampler("MainTexture", DefaultAssets::WHITE_TEXTURE, false);
		instanceCreateSettings.desc.materialAsset = DefaultAssets::FINAL_PASS_MATERIAL_ASSET;
		instanceCreateSettings.desc.isTransparent = false;

		AssetHandle instanceAssetHandle = m_AssetsMgr->CreateAsset<MaterialInstanceAsset, MaterialInstanceCreateSettings>(instanceCreateSettings.materialInstanceName, instanceCreateSettings);
		DefaultAssets::FINAL_PASS_MATERIAL_INSTANCE_ASSET = instanceAssetHandle;
		MaterialInstanceAsset* instanceAsset = m_AssetsMgr->GetAsset<MaterialInstanceAsset>(instanceAssetHandle);
		DefaultAssets::FINAL_PASS_MATERIAL_INSTANCE = instanceAsset->GetMaterialInstanceResourceHandle();
	}

	void BuildInAssets::CreateDepthLinearPrePassMaterial()
	{
		// Final pass material
		auto fullscreenVS = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\VertexFinalPassShader.vert", {});
		auto depthLinearPS = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\DepthLinearPrepassShader.frag", {});

		MaterialPassDesc desc;
		desc.vs = fullscreenVS;
		desc.ps = depthLinearPS;
		desc.pt = PrimitiveTopology::TRIANGLELIST;
		desc.rs.cull_mode = CullMode::BACK;
		desc.rs.fill_mode = PolygonMode::SOLID;
		desc.dss.depth_enable = true;
		desc.dss.depth_func = CompFunc::ALWAYS;
		desc.dss.depth_write_mask = DepthWriteMask::ZERO;
		desc.dss.stencil_enable = false;
		desc.bindPoint = BindPoint::BIND_POINT_GRAPHICS;
		desc.passType = RenderPassType::DepthLinearPrePass;

		DescriptorSetLayoutDesc desc_layout{};
		desc_layout.shaderStage = (U32)ShaderStage::PS;
		desc_layout.AddBinding("Depth", DescriptorType::SAMPLER, 0, 1, 0, false);
		desc_layout.AddBinding("CameraPlanesData", DescriptorType::UNIFORM_BUFFER, 1, 1, sizeof(CameraPlanesData), true);

		desc.AddLayout(desc_layout);

		MaterialDefinitionDesc matDesc(1);
		matDesc.EmplacePass(desc);
		matDesc.materialName = "DepthLinearMaterial";

		MaterialCreateSettings createSettings;
		createSettings.desc = std::move(matDesc);

		DefaultAssets::DEPTH_LINEAR_PREPASS_MATERIAL_ASSET = m_AssetsMgr->CreateAsset<MaterialAsset, MaterialCreateSettings>(createSettings.desc.materialName, createSettings);
		auto materialAsset = m_AssetsMgr->GetAsset<MaterialAsset>(DefaultAssets::DEPTH_LINEAR_PREPASS_MATERIAL_ASSET);
		materialAsset->SetPersistentStatus(true);
		DefaultAssets::DEPTH_LINEAR_PREPASS_MATERIAL = materialAsset->GetMaterialResourceHandle();

		MaterialInstanceCreateSettings instanceCreateSettings{};
		instanceCreateSettings.materialInstanceName = "DepthLinearPrePassMaterialInstance";
		instanceCreateSettings.desc.AddSampler("Depth", DefaultAssets::WHITE_TEXTURE, true);
		instanceCreateSettings.desc.AddBuffer("CameraPlanesData", BufferHandle{ Invalid_Handle_Id });
		instanceCreateSettings.desc.materialAsset = DefaultAssets::DEPTH_LINEAR_PREPASS_MATERIAL_ASSET;
		instanceCreateSettings.desc.isTransparent = false;

		AssetHandle instanceAssetHandle = m_AssetsMgr->CreateAsset<MaterialInstanceAsset, MaterialInstanceCreateSettings>(instanceCreateSettings.materialInstanceName, instanceCreateSettings);
		DefaultAssets::DEPTH_LINEAR_PREPASS_MATERIAL_INSTANCE_ASSET = instanceAssetHandle;
		MaterialInstanceAsset* instanceAsset = m_AssetsMgr->GetAsset<MaterialInstanceAsset>(instanceAssetHandle);
		DefaultAssets::DEPTH_LINEAR_PREPASS_MATERIAL_INSTANCE = instanceAsset->GetMaterialInstanceResourceHandle();
	}

	void BuildInAssets::CreateDefaultSimpleTransparentMaterial()
	{
		MaterialPassDesc desc;
		desc.vs = DefaultAssets::DEFAULT_VS_SHADER_ASSET;
		desc.ps = DefaultAssets::DEFAULT_PS_SHADER_ASSET;
		desc.pt = PrimitiveTopology::TRIANGLELIST;
		desc.rs.cull_mode = CullMode::BACK;
		desc.rs.fill_mode = PolygonMode::SOLID;
		desc.dss.depth_enable = true;
		desc.dss.depth_func = CompFunc::LESS_EQUAL;
		desc.dss.depth_write_mask = DepthWriteMask::ZERO;
		desc.dss.depth_write_enable = false;
		desc.dss.stencil_enable = false;
		desc.bindPoint = BindPoint::BIND_POINT_GRAPHICS;
		desc.passType = RenderPassType::ForwardLightingTransparent;
		desc.bs.SetAlphaBlend();

		desc.il.AddStream({ .binding = 0,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });
		desc.il.AddStream({ .binding = 1,.stride = sizeof(float) * 4,.input_rate = InputClassification::PER_VERTEX_DATA });
		desc.il.AddStream({ .binding = 2,.stride = sizeof(float) * 4,.input_rate = InputClassification::PER_VERTEX_DATA });
		desc.il.AddStream({ .binding = 3,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });

		desc.il.AddAttribute({ .location = 0,.binding = 0,.offset = 0,.format = Format::R32G32B32_FLOAT });
		desc.il.AddAttribute({ .location = 1,.binding = 1,.offset = 0,.format = Format::R32G32B32A32_FLOAT });
		desc.il.AddAttribute({ .location = 2,.binding = 2,.offset = 0,.format = Format::R32G32B32A32_FLOAT });
		desc.il.AddAttribute({ .location = 3,.binding = 3,.offset = 0,.format = Format::R32G32B32_FLOAT });

		DescriptorSetLayoutDesc desc_SceneData{};
		desc_SceneData.shaderStage = (U32)ShaderStage::PS | (U32)ShaderStage::VS;
		desc_SceneData.AddBinding("SceneData", DescriptorType::UNIFORM_BUFFER, 0, 1, sizeof(SceneData), true);
		desc_SceneData.AddBinding("RenderObjectsData", DescriptorType::STORAGE_BUFFER, 1, 1, sizeof(RenderObjectGPUData), true, DescriptorBindingTagType::RENDER_OBJECTS);

		DescriptorSetLayoutDesc desc_LightBuffers{};
		desc_LightBuffers.shaderStage = (U32)ShaderStage::PS;
		desc_LightBuffers.AddBinding("LightBuffer", DescriptorType::STORAGE_BUFFER, 0, 1, sizeof(LightData), true, DescriptorBindingTagType::LIGHTS_CONTAINER);
		desc_LightBuffers.AddBinding("LightIndexBuffer", DescriptorType::STORAGE_BUFFER, 1, 1, sizeof(LightsTileData), true, DescriptorBindingTagType::LIGHTS_INDEXES);
		desc_LightBuffers.AddBinding("TileDataBuffer", DescriptorType::STORAGE_BUFFER, 2, 1, sizeof(U32), true, DescriptorBindingTagType::LIGHTS_TILES);

		DescriptorSetLayoutDesc desc_tex{};
		desc_tex.shaderStage = (U32)ShaderStage::PS;
		desc_tex.AddBinding("MainTexture", DescriptorType::SAMPLER, 0, 1, 0, false);
		desc_tex.AddBinding("Color", DescriptorType::UNIFORM_BUFFER, 1, 1, sizeof(ColorUBO), false);


		UBOLayout uboLayout{};
		uboLayout.AddElement(0, sizeof(ColorUBO), UBOElementType::ColorType, StringID::MakeStringID("Color"));
		desc_tex.SetUBOLayout(uboLayout);

		desc.AddLayout(desc_SceneData);
		desc.AddLayout(desc_LightBuffers);
		desc.AddLayout(desc_tex);


		MaterialDefinitionDesc matDesc(1);
		matDesc.EmplacePass(desc);
		matDesc.materialName = "DefaultTransparentMaterial";


		MaterialCreateSettings createSettings;
		createSettings.desc = std::move(matDesc);

		DefaultAssets::DEFAULT_SIMPLE_TRANSPARENT_MATERIAL_ASSET = m_AssetsMgr->CreateAsset<MaterialAsset, MaterialCreateSettings>(createSettings.desc.materialName, createSettings);
		auto materialAsset = m_AssetsMgr->GetAsset<MaterialAsset>(DefaultAssets::DEFAULT_SIMPLE_TRANSPARENT_MATERIAL_ASSET);
		materialAsset->SetPersistentStatus(true);
		DefaultAssets::DEFAULT_SIMPLE_TRANSPARENT_MATERIAL = materialAsset->GetMaterialResourceHandle();

		MaterialInstanceCreateSettings instanceCreateSettings{};
		instanceCreateSettings.materialInstanceName = "DefaultTransparentMaterialInstance";
		instanceCreateSettings.desc.AddSampler("MainTexture", DefaultAssets::WHITE_TEXTURE, false);

		ColorUBO colorUbo;
		colorUbo.color = Vec4(1.0f, 1.0f, 1.0f, 1);

		instanceCreateSettings.desc.AddBuffer("Color", Czuch::UBO((void*)&colorUbo, sizeof(ColorUBO)));
		instanceCreateSettings.desc.materialAsset = DefaultAssets::DEFAULT_SIMPLE_TRANSPARENT_MATERIAL_ASSET;
		instanceCreateSettings.desc.isTransparent = true;

		AssetHandle instanceAssetHandle = m_AssetsMgr->CreateAsset<MaterialInstanceAsset, MaterialInstanceCreateSettings>(instanceCreateSettings.materialInstanceName, instanceCreateSettings);
		DefaultAssets::DEFAULT_SIMPLE_TRANSPARENT_MATERIAL_INSTANCE_ASSET = instanceAssetHandle;
		MaterialInstanceAsset* instanceAsset = m_AssetsMgr->GetAsset<MaterialInstanceAsset>(instanceAssetHandle);
		DefaultAssets::DEFAULT_SIMPLE_TRANSPARENT_MATERIAL_INSTANCE = instanceAsset->GetMaterialInstanceResourceHandle();
	}

	void BuildInAssets::CreateDebugDrawMaterials()
	{
		DefaultAssets::DEBUG_DRAW_VS_SHADER_ASSET = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\DebugDrawVertexShader.vert", {});

		//Debug material for standard debug meshes
		MaterialPassDesc desc;
		desc.vs = DefaultAssets::DEBUG_DRAW_VS_SHADER_ASSET;
		desc.ps = DefaultAssets::DEBUG_DRAW_PS_SHADER_ASSET;
		desc.pt = PrimitiveTopology::TRIANGLELIST;
		desc.rs.cull_mode = CullMode::BACK;
		desc.rs.fill_mode = PolygonMode::SOLID;
		desc.dss.depth_enable = true;
		desc.dss.depth_func = CompFunc::ALWAYS;
		desc.dss.depth_write_mask = DepthWriteMask::ZERO;
		desc.dss.depth_write_enable = false;
		desc.dss.stencil_enable = false;
		desc.bindPoint = BindPoint::BIND_POINT_GRAPHICS;
		desc.passType = RenderPassType::DebugDraw;

		desc.il.AddStream({ .binding = 0,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });
		desc.il.AddStream({ .binding = 1,.stride = sizeof(float) * 4,.input_rate = InputClassification::PER_VERTEX_DATA });
		desc.il.AddStream({ .binding = 2,.stride = sizeof(float) * 4,.input_rate = InputClassification::PER_VERTEX_DATA });
		desc.il.AddStream({ .binding = 3,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });

		desc.il.AddAttribute({ .location = 0,.binding = 0,.offset = 0,.format = Format::R32G32B32_FLOAT });
		desc.il.AddAttribute({ .location = 1,.binding = 1,.offset = 0,.format = Format::R32G32B32A32_FLOAT });
		desc.il.AddAttribute({ .location = 2,.binding = 2,.offset = 0,.format = Format::R32G32B32A32_FLOAT });
		desc.il.AddAttribute({ .location = 3,.binding = 3,.offset = 0,.format = Format::R32G32B32_FLOAT });

		DescriptorSetLayoutDesc desc_SceneData{};
		desc_SceneData.shaderStage = (U32)ShaderStage::PS | (U32)ShaderStage::VS;
		desc_SceneData.AddBinding("SceneData", DescriptorType::UNIFORM_BUFFER, 0, 1, sizeof(SceneData), true);

		DescriptorSetLayoutDesc desc_tex{};
		desc_tex.shaderStage = (U32)ShaderStage::PS;
		desc_tex.AddBinding("MainTexture", DescriptorType::SAMPLER, 0, 1, 0, false);
		desc_tex.AddBinding("Color", DescriptorType::UNIFORM_BUFFER, 1, 1, sizeof(ColorUBO), false);


		UBOLayout uboLayout{};
		uboLayout.AddElement(0, sizeof(ColorUBO), UBOElementType::ColorType, StringID::MakeStringID("Color"));
		desc_tex.SetUBOLayout(uboLayout);

		desc.AddLayout(desc_SceneData);
		desc.AddLayout(desc_tex);

		MaterialDefinitionDesc matDesc(1);
		matDesc.EmplacePass(desc);
		matDesc.materialName = "DefaultDebugMaterial";


		MaterialCreateSettings createSettings;
		createSettings.desc = std::move(matDesc);

		DefaultAssets::DEBUG_DRAW_MATERIAL_ASSET = m_AssetsMgr->CreateAsset<MaterialAsset, MaterialCreateSettings>(createSettings.desc.materialName, createSettings);
		auto materialAsset = m_AssetsMgr->GetAsset<MaterialAsset>(DefaultAssets::DEBUG_DRAW_MATERIAL_ASSET);
		materialAsset->SetPersistentStatus(true);
		DefaultAssets::DEBUG_DRAW_MATERIAL = materialAsset->GetMaterialResourceHandle();

		//Create Instance for debug draw material
		MaterialInstanceCreateSettings instanceCreateSettings{};
		instanceCreateSettings.materialInstanceName = "DefaultDebugMaterialInstance";
		instanceCreateSettings.desc.AddSampler("MainTexture", DefaultAssets::WHITE_TEXTURE, false);

		ColorUBO colorUbo;
		colorUbo.color = Vec4(1.0f, 1.0f, 1.0f, 1);

		instanceCreateSettings.desc.AddBuffer("Color", Czuch::UBO((void*)&colorUbo, sizeof(ColorUBO)));
		instanceCreateSettings.desc.materialAsset = DefaultAssets::DEBUG_DRAW_MATERIAL_ASSET;
		instanceCreateSettings.desc.isTransparent = false;

		AssetHandle instanceAssetHandle = m_AssetsMgr->CreateAsset<MaterialInstanceAsset, MaterialInstanceCreateSettings>(instanceCreateSettings.materialInstanceName, instanceCreateSettings);
		DefaultAssets::DEBUG_DRAW_MATERIAL_INSTANCE_ASSET = instanceAssetHandle;
		MaterialInstanceAsset* instanceAsset = m_AssetsMgr->GetAsset<MaterialInstanceAsset>(instanceAssetHandle);
		DefaultAssets::DEBUG_DRAW_MATERIAL_INSTANCE = instanceAsset->GetMaterialInstanceResourceHandle();

		//create instance for lights
		MaterialInstanceCreateSettings instanceLightCreateSettings{};
		instanceLightCreateSettings.materialInstanceName = "DefaultDebugLightMaterialInstance";
		instanceLightCreateSettings.desc.AddSampler("MainTexture", DefaultAssets::WHITE_TEXTURE, false);

		ColorUBO colorLightUbo;
		colorLightUbo.color = Vec4(1.0f, 1.0f, 1.0f, 1);

		instanceLightCreateSettings.desc.AddBuffer("Color", Czuch::UBO((void*)&colorLightUbo, sizeof(ColorUBO)));
		instanceLightCreateSettings.desc.materialAsset = DefaultAssets::DEBUG_DRAW_MATERIAL_ASSET;
		instanceLightCreateSettings.desc.isTransparent = false;

		AssetHandle instanceLightAssetHandle = m_AssetsMgr->CreateAsset<MaterialInstanceAsset, MaterialInstanceCreateSettings>(instanceLightCreateSettings.materialInstanceName, instanceLightCreateSettings);
		DefaultAssets::DEBUG_DRAW_LIGHT_MATERIAL_INSTANCE_ASSET = instanceLightAssetHandle;
		MaterialInstanceAsset* instanceLightAsset = m_AssetsMgr->GetAsset<MaterialInstanceAsset>(instanceLightAssetHandle);
		DefaultAssets::DEBUG_DRAW_LIGHT_MATERIAL_INSTANCE = instanceLightAsset->GetMaterialInstanceResourceHandle();

		//Debug material for standard debug lines/points/tris
		CreateDebugLinesMaterial();
		CreateDebugTrianglesMaterial();
		CreateDebugPointsMaterial();
	}
	void BuildInAssets::CreateDebugLinesMaterial()
	{
		DefaultAssets::DEBUG_DRAW_VS_INSTANCED_LINES_ASSET = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\DebugDrawIndirectLinesVertexShader.vert", {});
		DefaultAssets::DEFAULT_SIMPLE_COLOR_PS_ASSET = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\SimpleColorFragmentShader.frag", {});

		//Debug material for standard debug lines/points/tris
		MaterialPassDesc descDebug;
		descDebug.vs = DefaultAssets::DEBUG_DRAW_VS_INSTANCED_LINES_ASSET;
		descDebug.ps = DefaultAssets::DEFAULT_SIMPLE_COLOR_PS_ASSET;
		descDebug.pt = PrimitiveTopology::LINELIST;
		descDebug.rs.cull_mode = CullMode::NONE;
		descDebug.rs.fill_mode = PolygonMode::SOLID;
		descDebug.dss.depth_enable = true;
		descDebug.dss.depth_func = CompFunc::LESS_EQUAL;
		descDebug.dss.depth_write_mask = DepthWriteMask::ZERO;
		descDebug.dss.depth_write_enable = false;
		descDebug.dss.stencil_enable = false;
		descDebug.bindPoint = BindPoint::BIND_POINT_GRAPHICS;
		descDebug.passType = RenderPassType::DebugDraw;

		descDebug.il.AddStream({ .binding = 0,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });
		descDebug.il.AddAttribute({ .location = 0,.binding = 0,.offset = 0,.format = Format::R32G32B32_FLOAT });

		DescriptorSetLayoutDesc desc_SceneData{};
		desc_SceneData.shaderStage = (U32)ShaderStage::VS;
		desc_SceneData.AddBinding("SceneData", DescriptorType::UNIFORM_BUFFER, 0, 1, sizeof(SceneData), true);

		DescriptorSetLayoutDesc desc_data{};
		desc_data.shaderStage = (U32)ShaderStage::VS;
		desc_data.AddBinding("LineInstances", DescriptorType::STORAGE_BUFFER, 0, 1, sizeof(LineInstanceData), true, DescriptorBindingTagType::DEBUG_LINES_INSTANCE_DATA);

		descDebug.AddLayout(desc_SceneData);
		descDebug.AddLayout(desc_data);

		MaterialDefinitionDesc matDesc(1);
		matDesc.EmplacePass(descDebug);
		matDesc.materialName = "DefaultLinesDebugMaterial";


		MaterialCreateSettings createLinesSettings;
		createLinesSettings.desc = std::move(matDesc);

		DefaultAssets::DEBUG_DRAW_LINES_MATERIAL_ASSET = m_AssetsMgr->CreateAsset<MaterialAsset, MaterialCreateSettings>(createLinesSettings.desc.materialName, createLinesSettings);
		auto materialDebugAsset = m_AssetsMgr->GetAsset<MaterialAsset>(DefaultAssets::DEBUG_DRAW_LINES_MATERIAL_ASSET);
		materialDebugAsset->SetPersistentStatus(true);
		auto materialDebugHandle = materialDebugAsset->GetMaterialResourceHandle();

		MaterialInstanceCreateSettings instanceLinesCreateSettings{};
		instanceLinesCreateSettings.materialInstanceName = "DefaultDebugLinesMaterialInstance";
		instanceLinesCreateSettings.desc.materialAsset = DefaultAssets::DEBUG_DRAW_LINES_MATERIAL_ASSET;
		instanceLinesCreateSettings.desc.isTransparent = false;
		instanceLinesCreateSettings.desc.AddStorageBuffer("LineInstances", BufferHandle{ Invalid_Handle_Id });

		AssetHandle instanceAssetHandle = m_AssetsMgr->CreateAsset<MaterialInstanceAsset, MaterialInstanceCreateSettings>(instanceLinesCreateSettings.materialInstanceName, instanceLinesCreateSettings);
		MaterialInstanceAsset* instanceLinesAsset = m_AssetsMgr->GetAsset<MaterialInstanceAsset>(instanceAssetHandle);
		DefaultAssets::DEBUG_DRAW_LINES_MATERIAL_INSTANCE = instanceLinesAsset->GetMaterialInstanceResourceHandle();
	}
	void BuildInAssets::CreateDebugTrianglesMaterial()
	{
		DefaultAssets::DEBUG_DRAW_VS_INSTANCED_TRIANGLES_ASSET = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\DebugDrawIndirectTrianglesVertexShader.vert", {});

		//Debug material for standard debug lines/points/tris
		MaterialPassDesc descDebug;
		descDebug.vs = DefaultAssets::DEBUG_DRAW_VS_INSTANCED_TRIANGLES_ASSET;
		descDebug.ps = DefaultAssets::DEFAULT_SIMPLE_COLOR_PS_ASSET;
		descDebug.pt = PrimitiveTopology::TRIANGLELIST;
		descDebug.rs.cull_mode = CullMode::NONE;
		descDebug.rs.fill_mode = PolygonMode::SOLID;
		descDebug.dss.depth_enable = true;
		descDebug.dss.depth_func = CompFunc::LESS_EQUAL;
		descDebug.dss.depth_write_mask = DepthWriteMask::ZERO;
		descDebug.dss.depth_write_enable = false;
		descDebug.dss.stencil_enable = false;
		descDebug.bindPoint = BindPoint::BIND_POINT_GRAPHICS;
		descDebug.passType = RenderPassType::DebugDraw;
		descDebug.bs.SetAlphaBlend();

		descDebug.il.AddStream({ .binding = 0,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });
		descDebug.il.AddAttribute({ .location = 0,.binding = 0,.offset = 0,.format = Format::R32G32B32_FLOAT });

		DescriptorSetLayoutDesc desc_SceneData{};
		desc_SceneData.shaderStage = (U32)ShaderStage::VS;
		desc_SceneData.AddBinding("SceneData", DescriptorType::UNIFORM_BUFFER, 0, 1, sizeof(SceneData), true);

		DescriptorSetLayoutDesc desc_data{};
		desc_data.shaderStage = (U32)ShaderStage::VS;
		desc_data.AddBinding("TrianglesInstances", DescriptorType::STORAGE_BUFFER, 0, 1, sizeof(TriangleInstanceData), true, DescriptorBindingTagType::DEBUG_TRIANGLES_INSTANCE_DATA);

		descDebug.AddLayout(desc_SceneData);
		descDebug.AddLayout(desc_data);

		MaterialDefinitionDesc matDesc(1);
		matDesc.EmplacePass(descDebug);
		matDesc.materialName = "DefaultTrianglesDebugMaterial";


		MaterialCreateSettings createTrianglesSettings;
		createTrianglesSettings.desc = std::move(matDesc);

		DefaultAssets::DEBUG_DRAW_TRIANGLES_MATERIAL_ASSET = m_AssetsMgr->CreateAsset<MaterialAsset, MaterialCreateSettings>(createTrianglesSettings.desc.materialName, createTrianglesSettings);
		auto materialDebugAsset = m_AssetsMgr->GetAsset<MaterialAsset>(DefaultAssets::DEBUG_DRAW_TRIANGLES_MATERIAL_ASSET);
		materialDebugAsset->SetPersistentStatus(true);
		auto materialDebugHandle = materialDebugAsset->GetMaterialResourceHandle();

		MaterialInstanceCreateSettings instanceTrianglesCreateSettings{};
		instanceTrianglesCreateSettings.materialInstanceName = "DefaultDebugTrianglesMaterialInstance";
		instanceTrianglesCreateSettings.desc.materialAsset = DefaultAssets::DEBUG_DRAW_TRIANGLES_MATERIAL_ASSET;
		instanceTrianglesCreateSettings.desc.isTransparent = false;
		instanceTrianglesCreateSettings.desc.AddStorageBuffer("TrianglesInstances", BufferHandle{ Invalid_Handle_Id });

		AssetHandle instanceAssetHandle = m_AssetsMgr->CreateAsset<MaterialInstanceAsset, MaterialInstanceCreateSettings>(instanceTrianglesCreateSettings.materialInstanceName, instanceTrianglesCreateSettings);
		MaterialInstanceAsset* instanceLinesAsset = m_AssetsMgr->GetAsset<MaterialInstanceAsset>(instanceAssetHandle);
		DefaultAssets::DEBUG_DRAW_TRIANGLES_MATERIAL_INSTANCE = instanceLinesAsset->GetMaterialInstanceResourceHandle();
	}
	void BuildInAssets::CreateDebugPointsMaterial()
	{
		DefaultAssets::DEBUG_DRAW_VS_INSTANCED_POINTS_ASSET = m_AssetsMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\DebugDrawIndirectPointsVertexShader.vert", {});

		//Debug material for standard debug lines/points/tris
		MaterialPassDesc descDebug;
		descDebug.vs = DefaultAssets::DEBUG_DRAW_VS_INSTANCED_POINTS_ASSET;
		descDebug.ps = DefaultAssets::DEFAULT_SIMPLE_COLOR_PS_ASSET;
		descDebug.pt = PrimitiveTopology::POINTLIST;
		descDebug.rs.cull_mode = CullMode::NONE;
		descDebug.rs.fill_mode = PolygonMode::SOLID;
		descDebug.dss.depth_enable = true;
		descDebug.dss.depth_func = CompFunc::LESS_EQUAL;
		descDebug.dss.depth_write_mask = DepthWriteMask::ZERO;
		descDebug.dss.depth_write_enable = false;
		descDebug.dss.stencil_enable = false;
		descDebug.bindPoint = BindPoint::BIND_POINT_GRAPHICS;
		descDebug.passType = RenderPassType::DebugDraw;

		descDebug.il.AddStream({ .binding = 0,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });
		descDebug.il.AddAttribute({ .location = 0,.binding = 0,.offset = 0,.format = Format::R32G32B32_FLOAT });

		DescriptorSetLayoutDesc desc_SceneData{};
		desc_SceneData.shaderStage = (U32)ShaderStage::VS;
		desc_SceneData.AddBinding("SceneData", DescriptorType::UNIFORM_BUFFER, 0, 1, sizeof(SceneData), true);

		DescriptorSetLayoutDesc desc_data{};
		desc_data.shaderStage = (U32)ShaderStage::VS;
		desc_data.AddBinding("PointsInstances", DescriptorType::STORAGE_BUFFER, 0, 1, sizeof(PointInstanceData), true, DescriptorBindingTagType::DEBUG_POINTS_INSTANCE_DATA);

		descDebug.AddLayout(desc_SceneData);
		descDebug.AddLayout(desc_data);

		MaterialDefinitionDesc matDesc(1);
		matDesc.EmplacePass(descDebug);
		matDesc.materialName = "DefaultPointsDebugMaterial";


		MaterialCreateSettings createPointsSettings;
		createPointsSettings.desc = std::move(matDesc);

		DefaultAssets::DEBUG_DRAW_POINTS_MATERIAL_ASSET = m_AssetsMgr->CreateAsset<MaterialAsset, MaterialCreateSettings>(createPointsSettings.desc.materialName, createPointsSettings);
		auto materialDebugAsset = m_AssetsMgr->GetAsset<MaterialAsset>(DefaultAssets::DEBUG_DRAW_POINTS_MATERIAL_ASSET);
		materialDebugAsset->SetPersistentStatus(true);
		auto materialDebugHandle = materialDebugAsset->GetMaterialResourceHandle();

		MaterialInstanceCreateSettings instancePointsCreateSettings{};
		instancePointsCreateSettings.materialInstanceName = "DefaultDebugPointsMaterialInstance";
		instancePointsCreateSettings.desc.materialAsset = DefaultAssets::DEBUG_DRAW_POINTS_MATERIAL_ASSET;
		instancePointsCreateSettings.desc.isTransparent = false;
		instancePointsCreateSettings.desc.AddStorageBuffer("PointsInstances", BufferHandle{ Invalid_Handle_Id });

		AssetHandle instanceAssetHandle = m_AssetsMgr->CreateAsset<MaterialInstanceAsset, MaterialInstanceCreateSettings>(instancePointsCreateSettings.materialInstanceName, instancePointsCreateSettings);
		MaterialInstanceAsset* instanceLinesAsset = m_AssetsMgr->GetAsset<MaterialInstanceAsset>(instanceAssetHandle);
		DefaultAssets::DEBUG_DRAW_POINTS_MATERIAL_INSTANCE = instanceLinesAsset->GetMaterialInstanceResourceHandle();
	}
}