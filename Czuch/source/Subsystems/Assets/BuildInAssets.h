#pragma once
#include"./Renderer/Graphics.h"

namespace Czuch
{
	class GraphicsDevice;
	class AssetsManager;

	static struct CZUCH_API DefaultAssets
	{
		static AssetHandle PLANE_ASSET;
		static MeshHandle PLANE_HANDLE;
		static AssetHandle PLANE_GRID_ASSET;
		static MeshHandle PLANE_GRID_HANDLE;
		static AssetHandle CUBE_ASSET;
		static MeshHandle CUBE_HANDLE;

		static MaterialHandle DEFAULT_SIMPLE_MATERIAL;
		static AssetHandle DEFAULT_SIMPLE_MATERIAL_ASSET;
		static AssetHandle DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET;
		static MaterialInstanceHandle DEFAULT_SIMPLE_MATERIAL_INSTANCE;

		static MaterialHandle DEFAULT_SIMPLE_TRANSPARENT_MATERIAL;
		static MaterialInstanceHandle DEFAULT_SIMPLE_TRANSPARENT_MATERIAL_INSTANCE;
		static AssetHandle DEFAULT_SIMPLE_TRANSPARENT_MATERIAL_INSTANCE_ASSET;
		static AssetHandle DEFAULT_SIMPLE_TRANSPARENT_MATERIAL_ASSET;

		//material for debug drawing
		static MaterialHandle DEBUG_DRAW_MATERIAL;
		static AssetHandle DEBUG_DRAW_MATERIAL_ASSET;
		static AssetHandle DEBUG_DRAW_MATERIAL_INSTANCE_ASSET;
		static MaterialInstanceHandle DEBUG_DRAW_MATERIAL_INSTANCE;

		//debug drawing for lights
		static AssetHandle DEBUG_DRAW_LIGHT_MATERIAL_INSTANCE_ASSET;
		static MaterialInstanceHandle DEBUG_DRAW_LIGHT_MATERIAL_INSTANCE;

		//debug drawing for lines
		static AssetHandle DEBUG_DRAW_LINES_MATERIAL_ASSET;
		static MaterialInstanceHandle DEBUG_DRAW_LINES_MATERIAL_INSTANCE;

		//debug drawing for triangles
		static AssetHandle DEBUG_DRAW_TRIANGLES_MATERIAL_ASSET;
		static MaterialInstanceHandle DEBUG_DRAW_TRIANGLES_MATERIAL_INSTANCE;

		//debug drawing for points
		static AssetHandle DEBUG_DRAW_POINTS_MATERIAL_ASSET;
		static MaterialInstanceHandle DEBUG_DRAW_POINTS_MATERIAL_INSTANCE;

		//material for depth prepass
		static MaterialHandle DEPTH_PREPASS_MATERIAL;
		static AssetHandle DEPTH_PREPASS_MATERIAL_ASSET;
		static AssetHandle DEPTH_PREPASS_MATERIAL_INSTANCE_ASSET;
		static MaterialInstanceHandle DEPTH_PREPASS_MATERIAL_INSTANCE;

		//material for final pass
		static MaterialHandle FINAL_PASS_MATERIAL;
		static AssetHandle FINAL_PASS_MATERIAL_ASSET;
		static AssetHandle FINAL_PASS_MATERIAL_INSTANCE_ASSET;
		static MaterialInstanceHandle FINAL_PASS_MATERIAL_INSTANCE;

		//material for depth linear prepass
		static MaterialHandle DEPTH_LINEAR_PREPASS_MATERIAL;
		static AssetHandle DEPTH_LINEAR_PREPASS_MATERIAL_ASSET;
		static AssetHandle DEPTH_LINEAR_PREPASS_MATERIAL_INSTANCE_ASSET;
		static MaterialInstanceHandle DEPTH_LINEAR_PREPASS_MATERIAL_INSTANCE;

		//material for debug draw material index
		static MaterialHandle DEBUG_DRAW_MATERIAL_INDEX_MATERIAL;
		static AssetHandle DEBUG_DRAW_MATERIAL_INDEX_MATERIAL_ASSET;
		static MaterialInstanceHandle DEBUG_DRAW_MATERIAL_INDEX_MATERIAL_INSTANCE;



		static TextureHandle WHITE_TEXTURE;
		static AssetHandle WHITE_TEXTURE_ASSET;

		static TextureHandle PINK_TEXTURE;
		static AssetHandle PINK_TEXTURE_ASSET;

		//editor assets
		static AssetHandle EDITOR_ICON_TRANSLATE;
		static AssetHandle EDITOR_ICON_ROTATE;
		static AssetHandle EDITOR_ICON_SCALE;
		static AssetHandle EDITOR_ICON_FILE;
		static AssetHandle EDITOR_ICON_FOLDER;
		static AssetHandle EDITOR_ICON_MODEL;
		static AssetHandle EDITOR_ICON_MATERIAL;
		static AssetHandle EDITOR_ICON_TEXTURE;
		static AssetHandle EDITOR_ICON_SHADER;
		static AssetHandle EDITOR_ICON_MATERIAL_INSTANCE;



		//shader assets
		static AssetHandle PHONG_VS_SHADER_ASSET;
		static AssetHandle PHONG_PS_SHADER_ASSET;
		static AssetHandle DEFAULT_SIMPLE_COLOR_PS_ASSET;
		static AssetHandle DEBUG_DRAW_VS_SHADER_ASSET;
		static AssetHandle DEBUG_DRAW_PS_SHADER_ASSET;
		static AssetHandle DEBUG_DRAW_VS_INSTANCED_LINES_ASSET;
		static AssetHandle DEBUG_DRAW_VS_INSTANCED_TRIANGLES_ASSET;
		static AssetHandle DEBUG_DRAW_VS_INSTANCED_POINTS_ASSET;
		static AssetHandle DEBUG_DRAW_PS_MATERIAL_INDEX_SHADER_ASSET;
	};

	class BuildInAssets
	{
	public:
		BuildInAssets(GraphicsDevice* device, AssetsManager* mgr,EngineMode mode);
		void BuildAndLoad();
	private:
		void CreateDefaultTextures();
		void FillLightingLayoutDesc(Czuch::DescriptorSetLayoutDesc& desc_LightBuffers);
		void CreateDefaultMaterials();
		void FillVertexStreamwithAttributes(Czuch::MaterialPassDesc& desc);
		void CreateDefaultModels();
		void LoadUIAssets();
	private:
		void CreateDepthPrePassMaterial();
		void CreateFinalPassMaterial();
		void CreateDepthLinearPrePassMaterial();
		void CreateDefaultSimpleTransparentMaterial();
		void CreateDebugDrawMaterials();
		void CreateDebugLinesMaterial();
		void CreateDebugTrianglesMaterial();
		void CreateDebugPointsMaterial();
		void CreateDebugDrawMaterialIndexMaterial();
	private:
		GraphicsDevice* m_Device;
		AssetsManager* m_AssetsMgr;
		EngineMode m_Mode;
		bool m_IsBuilt = false;
	};

}

