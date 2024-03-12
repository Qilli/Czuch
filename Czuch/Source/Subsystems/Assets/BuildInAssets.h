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

		static TextureHandle WHITE_TEXTURE;
		static AssetHandle WHITE_TEXTURE_ASSET;
	};

	class BuildInAssets
	{
	public:
		BuildInAssets(GraphicsDevice* device, AssetsManager* mgr);
		void BuildAndLoad();
	private:
		void CreateDefaultTextures();
		void CreateDefaultMaterials();
		void CreateDefaultModels();
	private:
		GraphicsDevice* m_Device;
		AssetsManager* m_AssetsMgr;
	};

}

