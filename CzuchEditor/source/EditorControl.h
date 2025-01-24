#pragma once
#include "Czuch.h"
#include"Panels/SceneHierarchyEditorPanel.h"
#include"Panels/EntityInspectorEditorPanel.h"
#include"Commands/EditorCommandsControl.h"
#include"EditorCommon.h"

namespace ImGuizmo
{
	enum OPERATION;
}

namespace Czuch
{
	class EngineRoot;

	enum class GizmoMode
	{
		Translate,
		Rotate,
		Scale
	};

	struct GizmoFrame
	{
		Vec3 position;
		Vec3 rotation;
		Vec3 scale;
	};

	class RenderGraphEditorWindow;
	class AssetsEditorWindow;
	class AssetsInfoEditorWindow;


	class EditorControl : public EngineEditorControl
	{
	public:
		EditorControl();
		virtual ~EditorControl();
		virtual void Init(void* context, RenderSettings* renderSettings) override;
		virtual void AfterSystemInit() override;
		virtual void Shutdown() override;
		virtual void Update(TimeDelta timeDelta) override;
		virtual void FillUI(void* sceneViewportTexture);
		void HandleTopBar();
		void HandelGizmoTransforms(Czuch::Entity& currentSelectedEntity);
		float GetSnapValue();

	protected:
		virtual void FillMainMenubar();
		void ShowMenuFile();
		bool ShowSaveMenu();
		void ShowCommandsStackPopup();
		void ShowConsoleLogPanel();
		bool CheckCurrentSceneForSave();
		bool UpdateOffscreenPass(U32 width, U32 height);
		ImGuizmo::OPERATION GetCurrentGizmoMode();
	private:
		EngineRoot* m_Root;
		std::string m_CurrentScenePath;
		SceneHierarchyEditorPanel* m_SceneHierarchyPanel;
		EntityInspectorEditorPanel* m_EntityInspectorPanel;
		bool m_OffscreenPassInited;
		U32 m_Width, m_Height;
		std::function<void(U32, U32)> m_UpdateOffscreenPass;
		EditorCommandsControl* m_CommandsControl;

		bool m_ShowCommandsStackPopup;
		bool m_ShowConsoleLogPanel;
	private: //windows
		RenderGraphEditorWindow* m_RenderGraphEditorWindow;
		AssetsEditorWindow* m_AssetsEditorWindow;
		AssetsInfoEditorWindow* m_AssetsInfoEditorWindow;


	private: //gizmo
		GizmoMode m_GizmoMode;
		bool m_IsGizmoActive;
		GizmoFrame m_GizmoFrame;
		
	};
}

