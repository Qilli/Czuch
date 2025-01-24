#pragma once
#include"Subsystems/Assets/Asset/Asset.h"

namespace Czuch
{
	static struct EditorAssets
	{
		static void* s_EditorFolderTexture;
		static void* s_EditorFileTexture;
		static void* s_EditorModelTexture;
		static void* s_EditorMaterialTexture;
		static void* s_EditorTextureTexture;
		static void* s_EditorShaderTexture;
		static void* s_EditorMaterialInstanceTexture;
		static void* GetIconForType(AssetType type);
	};

}