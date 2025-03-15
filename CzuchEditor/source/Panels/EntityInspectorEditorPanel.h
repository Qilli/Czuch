#pragma once
#include "BaseEditorPanel.h"
#include "../EditorCommon.h"

namespace Czuch
{

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
		SelectMeshAssetHelper m_MeshAssetHelper;
		void DrawComponent(Entity entity) override;
		void OnSelectionChanged(Entity entity) override;
		void OnRemoveComponent(Entity entity) override;
		MeshInspectorDrawer() {
			id = 4; }
	};

	struct MeshRendererInspectorDrawer : public ComponentDrawer
	{
		SelectMaterialInstanceAssetHelper m_MaterialAssetHelper;
		SelectTextureAssetHelper m_TextureAssetHelper;
		void DrawComponent(Entity entity) override;
		void OnSelectionChanged(Entity entity) override;
		void OnRemoveComponent(Entity entity) override;
		MeshRendererInspectorDrawer() {
			id = 5;
		}
	private:
		void DrawMaterialInstance(MeshRendererComponent& meshRendererComponent);
	};

	struct LightInspectorDrawer : public ComponentDrawer
	{
		void DrawComponent(Entity entity) override;
		void OnSelectionChanged(Entity entity) override;
		void OnRemoveComponent(Entity entity) override;
		LightInspectorDrawer() {
			id = 6;
		}
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
		MeshRendererInspectorDrawer m_MeshRendererDrawer;
		LightInspectorDrawer m_LightDrawer;
	};
}


