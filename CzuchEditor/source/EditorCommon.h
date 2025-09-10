#pragma once
#include"Subsystems/Assets/Asset/Asset.h"
#include"Subsystems/Scenes/Scene.h"
#include"imgui.h"

namespace Czuch
{
	struct MeshComponent;
	struct MeshRendererComponent;
	class MaterialInstanceAsset;


	ImTextureID GetAssetTexturePreview(Czuch::AssetsManager* mgr, ImTextureID defaultTexture, const std::filesystem::path& path);


	struct LockedVec3
	{
		LockedVec3() { isLocked = false; value = Vec3(0, 0, 0); }
		bool isLocked;
		Vec3 value;
	};


	struct ComponentModalData
	{
		bool show = false;
		CzuchStr title;
		CzuchStr text;
	};

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

	static const char* s_AssetTypesNames[] = {
	"Texture", "Material", "MaterialInstance", "Mesh", "Shader","All"
	};


	struct CustomDrawers
	{
	public:
		static bool DrawVector3(const CzuchStr& label, Vec3& vec, float colWidth, float resetValue, LockedVec3& locked);
		static bool DrawColorWithLabel(const CzuchStr& label, Vec4& color, float colWidth);
		static bool ButtonCenteredOnLine(const char* label, float alignment = 0.5f);
		static void LabelCenteredOnLine(const char* label, float alignment = 0.5f, float leftPadding = 0.0f, float rightPadding = 0.0f);
		static void ShowModalWindow(const char* title, const char* text, bool& isOpen);
		static void FormatAssetName(Czuch::ShortAssetInfo* const asset, char* tempName);
		static void* GetIconForType(AssetType type);
		static void DrawTextureParameter(const char* label, AssetHandle& asset, I32& resource, bool& showPopup, AssetType type);
		static bool DrawLinearColor(const CzuchStr& name, Vec4* color);
		static bool DrawLinearColor(const CzuchStr& name, Vec3* color);
	};

	struct ShowAssetSelectorPopupHelper
	{
		char searchBuffer[128] = "";
		char tempName[15];
		AssetType selectedType;
	    const Array<ShortAssetInfo*>* filteredAssets;
		const ShortAssetInfo* selected;
		bool filterOnEnter;
		void Reset();
		const ShortAssetInfo* ShowAssetSelectorPopup(const char* popupId,bool canChangeType);
	};


	struct SelectAssetHelper
	{
		ShowAssetSelectorPopupHelper m_AssetSelectorPopup;
		AssetType m_AssetType;
		CzuchStr m_AssetNameType;
		CzuchStr m_AssetPopupName;
		CzuchStr m_AssetSelectButtonID;
		bool ShowSelectAsset();
		virtual bool HasSelectedAsset()=0;
		virtual const CzuchStr* GetSelectedAssetName()=0;
		virtual void SetSelected(AssetHandle asset,I32 resource)=0;
	};

	struct SelectMeshAssetHelper : public SelectAssetHelper
	{
		SelectMeshAssetHelper() : m_MeshComponent(nullptr)
		{
			m_AssetType = AssetType::MESH;
			m_AssetNameType = " Mesh: ";
			m_AssetSelectButtonID = "Select##Mesh";
			m_AssetPopupName = "SelectMeshPopup";
			m_AssetSelectorPopup.filterOnEnter = true;
			m_AssetSelectorPopup.selectedType = AssetType::MESH;
		}

		MeshComponent* m_MeshComponent;
		bool HasSelectedAsset() override;
		const CzuchStr* GetSelectedAssetName() override;
		void SetSelected(AssetHandle asset, I32 resource) override;
	};

	struct SelectMaterialInstanceAssetHelper : public SelectAssetHelper
	{
		SelectMaterialInstanceAssetHelper() : m_MeshRendererComponent(nullptr)
		{
			m_AssetType = AssetType::MATERIAL_INSTANCE;
			m_AssetNameType = " Material: ";
			m_AssetSelectButtonID = "Select##Material";
			m_AssetPopupName = "SelectMaterialPopup";
			m_AssetSelectorPopup.filterOnEnter = true;
			m_AssetSelectorPopup.selectedType = AssetType::MATERIAL_INSTANCE;
		}

		MeshRendererComponent* m_MeshRendererComponent;
		bool HasSelectedAsset() override;
		const CzuchStr* GetSelectedAssetName() override;
		void SetSelected(AssetHandle asset, I32 resource) override;
	};

	struct SelectTextureAssetHelper : public SelectAssetHelper
	{
		SelectTextureAssetHelper() : m_MaterialInstance(nullptr)
		{
			m_AssetType = AssetType::TEXTURE;
			m_AssetNameType = " Texture: ";
			m_AssetSelectButtonID = "Select##Texture";
			m_AssetPopupName = "SelectTexturePopup";
			m_AssetSelectorPopup.filterOnEnter = true;
			m_AssetSelectorPopup.selectedType = AssetType::TEXTURE;
			m_ParamIndex = -1;
		}

		MaterialInstanceAsset* m_MaterialInstance;
		void SetMaterialInstance(MaterialInstanceAsset* mat, int index);
		void SetMaterialInstance(MaterialInstanceAsset* mat, int currentTexture, std::function<void(int)> onTextureSelected,CzuchStr& pramName);
		int m_ParamIndex;
		bool m_GlobalTexture;
		std::function<void(int)> m_OnTextureSelected;
		bool HasSelectedAsset() override;
		const CzuchStr* GetSelectedAssetName() override;
		void SetSelected(AssetHandle asset, I32 resource) override;
	};

}