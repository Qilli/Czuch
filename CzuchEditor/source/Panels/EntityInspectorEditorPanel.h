#pragma once
#include "BaseEditorPanel.h"

namespace Czuch
{

	struct CustomDrawers
	{
	public:
		static bool DrawVector3(const CzuchStr& label, Vec3& vec,float colWidth,float resetValue,LockedVec3& locked);
		static bool ButtonCenteredOnLine(const char* label,float alignment=0.5f);
		static void LabelCenteredOnLine(const char* label, float alignment = 0.5f, float leftPadding = 0.0f, float rightPadding = 0.0f);
		static void ShowModalWindow(const char* title,const char* text, bool& isOpen);
		static const ShortAssetInfo* ShowAssetSelectorPopup(const char* popupId, AssetType filterSearchType,bool canChangeType);
		static void FormatAssetName(Czuch::ShortAssetInfo* const asset);
		static void* GetIconForType(AssetType type);
	public:
		static bool m_ResetShowAssetModal;
		static char tempName[15];
	};

	struct ComponentModalData
	{
		bool show = false;
		std::string title;
		std::string text;
	};

	struct ComponentDrawer
	{
		bool isOpen = true;
		bool removeComponent = false;
		int id = 0;
		ComponentModalData m_ModalData;
		ComponentDrawer() { id = 0; }
		virtual void DrawComponent(Entity entity) {}
		virtual void OnSelectionChanged(Entity entity) {}
		virtual void OnRemoveComponent(Entity entity) { m_ModalData.show = true; m_ModalData.title = "Info"; m_ModalData.text = "You cannot remove this component"; }
		bool DrawComponentHeader(const char* name,Entity entity);
		void DrawModalWindow();

	};

	struct HeaderDrawer: public ComponentDrawer
	{
		void DrawComponent(Entity entity) override;
		void OnSelectionChanged(Entity entity) override;
		void UpdateTexts(Entity entity);

		char m_CurrentName[256];
		std::string m_HeaderText;

		HeaderDrawer() { id = 1; }
	};

	struct TransformDrawer : public ComponentDrawer
	{
		LockedVec3 m_Position;
		LockedVec3 m_Rotation;
		LockedVec3 m_Scale;
		void DrawComponent(Entity entity) override;
		void OnSelectionChanged(Entity entity) override;
		TransformDrawer() { id = 2; }
	};

	struct CameraDrawer : public ComponentDrawer
	{
		void DrawComponent(Entity entity) override;
		void OnSelectionChanged(Entity entity) override;
		void OnRemoveComponent(Entity entity) override;
		CameraDrawer() { id = 3; }
	};

	struct MeshInspectorDrawer : public ComponentDrawer
	{
		void DrawComponent(Entity entity) override;
		void OnSelectionChanged(Entity entity) override;
		void OnRemoveComponent(Entity entity) override;
		MeshInspectorDrawer() { id = 4; }
	};

	class EntityInspectorEditorPanel :
		public BaseEditorPanel
	{
	public:
		EntityInspectorEditorPanel() = default;
		~EntityInspectorEditorPanel() = default;
		void FillUI() override;
		void SelectedEntityChanged(Entity entity) override;
	private:
		void AddComponentMenu();
	private:
		HeaderDrawer m_HeaderDrawer;
		TransformDrawer m_TransformDrawer;
		CameraDrawer m_CameraDrawer;
		MeshInspectorDrawer m_MeshDrawer;
	};
}


