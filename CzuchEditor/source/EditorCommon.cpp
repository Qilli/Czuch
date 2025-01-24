#include "EditorCommon.h"

namespace Czuch
{

	void* EditorAssets::s_EditorFolderTexture;
	void* EditorAssets::s_EditorFileTexture;
	void* EditorAssets::s_EditorModelTexture;
	void* EditorAssets::s_EditorMaterialTexture;
	void* EditorAssets::s_EditorTextureTexture;
	void* EditorAssets::s_EditorShaderTexture;
	void* EditorAssets::s_EditorMaterialInstanceTexture;

	void* EditorAssets::GetIconForType(AssetType type)
	{
			switch (type)
			{
			case AssetType::TEXTURE:
				return s_EditorTextureTexture;
			case AssetType::MATERIAL:
				return s_EditorMaterialTexture;
			case AssetType::MATERIAL_INSTANCE:
				return s_EditorMaterialInstanceTexture;
			case AssetType::MESH:
				return s_EditorModelTexture;
			case AssetType::SHADER:
				return s_EditorShaderTexture;
			default:
				return s_EditorFileTexture;
			}
	}

}
