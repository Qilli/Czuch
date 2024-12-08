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
		static MaterialInstanceHandle DEFAULT_SIMPLE_MATERIAL_INSTANCE;

		//material for depth prepass
		static MaterialHandle DEPTH_PREPASS_MATERIAL;
		static AssetHandle DEPTH_PREPASS_MATERIAL_ASSET;
		static MaterialInstanceHandle DEPTH_PREPASS_MATERIAL_INSTANCE;

		//material for final pass
		static MaterialHandle FINAL_PASS_MATERIAL;
		static AssetHandle FINAL_PASS_MATERIAL_ASSET;
		static MaterialInstanceHandle FINAL_PASS_MATERIAL_INSTANCE;

		//material for depth linear prepass
		static MaterialHandle DEPTH_LINEAR_PREPASS_MATERIAL;
		static AssetHandle DEPTH_LINEAR_PREPASS_MATERIAL_ASSET;
		static MaterialInstanceHandle DEPTH_LINEAR_PREPASS_MATERIAL_INSTANCE;

		static TextureHandle WHITE_TEXTURE;
		static AssetHandle WHITE_TEXTURE_ASSET;

		//editor assets
		static AssetHandle EDITOR_ICON_TRANSLATE;
		static AssetHandle EDITOR_ICON_ROTATE;
		static AssetHandle EDITOR_ICON_SCALE;
	};

	class BuildInAssets
	{
	public:
		BuildInAssets(GraphicsDevice* device, AssetsManager* mgr,EngineMode mode);
		void BuildAndLoad();
	private:
		void CreateDefaultTextures();
		void CreateDefaultMaterials();
		void CreateDefaultModels();
		void LoadUIAssets();
	private:
		void CreateDepthPrePassMaterial();
		void CreateFinalPassMaterial();
		void CreateDepthLinearPrePassMaterial();
	private:
		GraphicsDevice* m_Device;
		AssetsManager* m_AssetsMgr;
		EngineMode m_Mode;
	};

}

