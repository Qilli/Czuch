#include "czpch.h"
#include "EditorControl.h"
#include"imgui.h"
#include"imgui_internal.h"
#include"Platform/Windows/WinUtils.h"
#include"ImGuizmo.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"
#include"Commands/CommandTypes/ChangeTransformCommand.h"
#include"EditorWindows/RenderGraphEditorWindow.h"
#include"EditorWindows/AssetsEditorWindow.h"
#include"EditorWindows/AssetsInfoEditorWindow.h"
#include"Commands/CommandTypes/CreateDefaultMeshesCommands.h"
#include"glm/gtx/string_cast.hpp"

namespace Czuch
{

	void SetLightGreyStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		// Set default dark theme colors
		ImGui::StyleColorsDark();

		// Adjust specific colors to use dark grey instead of blue
		ImVec4 darkGrey = ImVec4(0.03f, 0.03f, 0.03f, 1.0f);
		ImVec4 darkGreyHover = ImVec4(0.07f, 0.07f, 0.07f, 0.3f);
		ImVec4 darkGreyActive = ImVec4(0.03f, 0.03f, 0.03f, 0.3f);

		colors[ImGuiCol_Header] = darkGrey;                               // Header background
		colors[ImGuiCol_HeaderHovered] = darkGreyHover;                   // Header background when hovered
		colors[ImGuiCol_HeaderActive] = darkGreyActive;                   // Header background when active
		colors[ImGuiCol_Button] = darkGrey;                               // Button background
		colors[ImGuiCol_ButtonHovered] = darkGreyHover;                   // Button background when hovered
		colors[ImGuiCol_ButtonActive] = darkGreyActive;                   // Button background when active
		colors[ImGuiCol_FrameBg] = darkGrey;                              // Frame background
		colors[ImGuiCol_FrameBgHovered] = darkGreyHover;                  // Frame background when hovered
		colors[ImGuiCol_FrameBgActive] = darkGreyActive;                  // Frame background when active
		colors[ImGuiCol_SliderGrab] = darkGrey;                           // Slider grab
		colors[ImGuiCol_SliderGrabActive] = darkGreyActive;               // Slider grab when active
		colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);      // Check mark color (white for visibility)
		colors[ImGuiCol_Separator] = darkGrey;                            // Separator
		colors[ImGuiCol_SeparatorHovered] = darkGreyHover;                // Separator when hovered
		colors[ImGuiCol_SeparatorActive] = darkGreyActive;                // Separator when active
		colors[ImGuiCol_ResizeGrip] = darkGrey;                           // Resize grip
		colors[ImGuiCol_ResizeGripHovered] = darkGreyHover;               // Resize grip when hovered
		colors[ImGuiCol_ResizeGripActive] = darkGreyActive;               // Resize grip when active
		colors[ImGuiCol_Tab] = darkGrey;                                  // Tab background
		colors[ImGuiCol_TabHovered] = darkGreyHover;                      // Tab background when hovered
		colors[ImGuiCol_TabActive] = darkGreyActive;                      // Tab background when active
		colors[ImGuiCol_TabUnfocused] = darkGrey;                         // Tab background when unfocused
		colors[ImGuiCol_TabUnfocusedActive] = darkGreyActive;             // Tab background when unfocused and active

		// Adjust window bar colors
		colors[ImGuiCol_TitleBg] = darkGrey;                              // Title bar background
		colors[ImGuiCol_TitleBgActive] = darkGreyActive;                  // Title bar background when active
		colors[ImGuiCol_TitleBgCollapsed] = darkGrey;                     // Title bar background when collapsed
		colors[ImGuiCol_MenuBarBg] = darkGrey;                            // Menubar background
		colors[ImGuiCol_ScrollbarBg] = darkGrey;                          // Scrollbar background
		colors[ImGuiCol_ScrollbarGrab] = darkGrey;                        // Scrollbar grab
		colors[ImGuiCol_ScrollbarGrabHovered] = darkGreyHover;            // Scrollbar grab when hovered
		colors[ImGuiCol_ScrollbarGrabActive] = darkGreyActive;            // Scrollbar grab when active

		// Adjust other elements as needed
		colors[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.01f, 0.01f, 1.0f);       // Window background
		colors[ImGuiCol_ChildBg] = ImVec4(0.01f, 0.01f, 0.01f, 1.0f);        // Child window background
		colors[ImGuiCol_PopupBg] = ImVec4(0.01f, 0.01f, 0.01f, 1.0f);        // Popup background
		colors[ImGuiCol_Border] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);         // Border color
		colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);           // Text color
		colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);   // Disabled text color

	}

	EditorControl::EditorControl() : m_Root(EngineRoot::GetPtr())
	{
		m_OffscreenPassInited = false;
		m_ShowConsoleLogPanel = true;
		m_SceneHierarchyPanel = nullptr;
		m_EntityInspectorPanel = nullptr;
		m_GizmoMode = GizmoMode::Translate;
		m_RenderGraphEditorWindow = new RenderGraphEditorWindow("Render Graph Preview");
		m_AssetsEditorWindow = new AssetsEditorWindow("Assets Editor");
		m_AssetsEditorWindow->SetWindowVisible(true);
		m_AssetsInfoEditorWindow = new AssetsInfoEditorWindow("Assets Info");
	}
	EditorControl::~EditorControl()
	{
		ImGuizmo::SetImGuiContext(nullptr);
		delete m_AssetsEditorWindow;
		delete m_RenderGraphEditorWindow;
		delete m_CommandsControl;
		delete m_SceneHierarchyPanel;
		delete m_EntityInspectorPanel;
	}
	void EditorControl::Init(void* context, RenderSettings* renderSettings)
	{
		ImGui::SetCurrentContext((ImGuiContext*)context);
		ImGuizmo::Enable(true);
		ImGuizmo::SetImGuiContext((ImGuiContext*)context);
		SetLightGreyStyle();
		m_CommandsControl = new EditorCommandsControl();
		UpdateOffscreenPass(renderSettings->targetWidth, renderSettings->targetHeight);

	}

	void EditorControl::AfterSystemInit()
	{
		auto mgr = AssetsManager::GetPtr();
		EditorAssets::s_EditorFileTexture = mgr->GetAsset<TextureAsset>(DefaultAssets::EDITOR_ICON_FILE)->GetUITextureIDPtr();
		EditorAssets::s_EditorFolderTexture = mgr->GetAsset<TextureAsset>(DefaultAssets::EDITOR_ICON_FOLDER)->GetUITextureIDPtr();
		EditorAssets::s_EditorModelTexture = mgr->GetAsset<TextureAsset>(DefaultAssets::EDITOR_ICON_MODEL)->GetUITextureIDPtr();
		EditorAssets::s_EditorMaterialTexture = mgr->GetAsset<TextureAsset>(DefaultAssets::EDITOR_ICON_MATERIAL)->GetUITextureIDPtr();
		EditorAssets::s_EditorTextureTexture = mgr->GetAsset<TextureAsset>(DefaultAssets::EDITOR_ICON_TEXTURE)->GetUITextureIDPtr();
		EditorAssets::s_EditorShaderTexture = mgr->GetAsset<TextureAsset>(DefaultAssets::EDITOR_ICON_SHADER)->GetUITextureIDPtr();
		EditorAssets::s_EditorMaterialInstanceTexture = mgr->GetAsset<TextureAsset>(DefaultAssets::EDITOR_ICON_MATERIAL_INSTANCE)->GetUITextureIDPtr();
	}

	void EditorControl::Shutdown()
	{
	}

	void EditorControl::Update(TimeDelta timeDelta)
	{
	}

	void EditorControl::FillUI(void* sceneViewportTexture)
	{
		ImGuizmo::BeginFrame();
		if (m_SceneHierarchyPanel == nullptr)
		{
			m_SceneHierarchyPanel = new SceneHierarchyEditorPanel(m_Root->GetScenesManager().GetActiveScene());
		}
		else
		{
			m_SceneHierarchyPanel->SetActiveScene(m_Root->GetScenesManager().GetActiveScene());
		}

		if (m_EntityInspectorPanel == nullptr)
		{
			m_EntityInspectorPanel = new EntityInspectorEditorPanel();
			m_SceneHierarchyPanel->AddOnSelectedEntityListener(m_EntityInspectorPanel);
		}

		FillMainMenubar();
		HandleTopBar();

		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0.0f });


		// Get the position & size of the ImGui window
		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 windowSize = ImGui::GetWindowSize();
		
		ImGui::Begin("Scene");
		bool isHovered = ImGui::IsWindowHovered();

		// Get the position & size of the ImGui window
		 windowPos = ImGui::GetWindowPos();
		 windowSize = ImGui::GetWindowSize();

		m_Root->GetUIBaseManager().SetBlockEvents(isHovered);

		auto targetViewportSize = ImGui::GetContentRegionAvail();

		if (targetViewportSize.x == 0)
		{
			targetViewportSize.x = 1;
		}

		if (targetViewportSize.y == 0)
		{
			targetViewportSize.y = 1;
		}


		if (UpdateOffscreenPass((U32)targetViewportSize.x, (U32)targetViewportSize.y))
		{
			ImGui::Image((ImTextureID)m_Root->GetRenderer().GetFrameGraphFinalResult(), ImGui::GetContentRegionAvail());
		}

		// Define the correct rendering rectangle
		ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
		Entity currentSelectedEntity = m_SceneHierarchyPanel->GetSelectedEntity();
		HandelGizmoTransforms(currentSelectedEntity);

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSETS_EDITOR_ASSET"))
			{
				auto path = (const wchar_t*)payload->Data;
				std::filesystem::path p(path);
				if (AssetsManager::GetPtr()->IsFormatAssetOfType(p.extension().string().c_str(), AssetType::MESH))
				{
					auto activeScene = m_Root->GetScenesManager().GetActiveScene();
					EditorCommandsControl::Get().ExecuteCommand(new CreateNewEntityHierarchyWithModelCommand(activeScene, activeScene->GetRootEntity(), p.string()));
				}
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::End();




		//scene hierarchy panel
		m_SceneHierarchyPanel->FillUI();
		//inspector panel
		m_EntityInspectorPanel->FillUI();

		//render graph editor
		m_RenderGraphEditorWindow->DrawWindow(m_Root);

		//assets editor window
		m_AssetsEditorWindow->DrawWindow(m_Root);

		//assets info editor window
		m_AssetsInfoEditorWindow->DrawWindow(m_Root);

		if (m_ShowConsoleLogPanel)
		{
			ShowConsoleLogPanel();
		}

		if (m_ShowCommandsStackPopup)
		{
			ShowCommandsStackPopup();
		}

		ImGui::PopStyleVar();
	}

	void EditorControl::HandleTopBar()
	{
		ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
		float height = 40.0f;

		if (ImGui::BeginViewportSideBar("##SecondaryMenuBar", viewport, ImGuiDir_Up, height, window_flags)) {
			ImVec2 sizeButton = { height * 0.5f,height * 0.5f };

			auto mgr = AssetsManager::GetPtr();
			auto translateIconId = mgr->GetAsset<TextureAsset>(DefaultAssets::EDITOR_ICON_TRANSLATE)->GetUITextureIDPtr();
			auto rotateIconId = mgr->GetAsset<TextureAsset>(DefaultAssets::EDITOR_ICON_ROTATE)->GetUITextureIDPtr();
			auto scaleIconId = mgr->GetAsset<TextureAsset>(DefaultAssets::EDITOR_ICON_SCALE)->GetUITextureIDPtr();

			ImVec4 buttonSelected = ImVec4(0.8f, 0.8f, 0.8f, 0.5f);
			ImVec4 buttonUnselected = ImVec4(0.05f, 0.05f, 0.05f, 0.5f);

			if (m_GizmoMode == GizmoMode::Translate)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, buttonSelected);
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, buttonUnselected);
			}

			if (ImGui::ImageButton("TranslateModeTopBar", (ImTextureID)translateIconId, sizeButton))
			{
				m_GizmoMode = GizmoMode::Translate;
			}
			ImGui::PopStyleColor();

			if (m_GizmoMode == GizmoMode::Rotate)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, buttonSelected);
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, buttonUnselected);
			}


			ImGui::SameLine();
			if (ImGui::ImageButton("RotateModeTopBar", (ImTextureID)rotateIconId, sizeButton))
			{
				m_GizmoMode = GizmoMode::Rotate;
			}
			ImGui::PopStyleColor();


			if (m_GizmoMode == GizmoMode::Scale)
			{
				ImGui::PushStyleColor(ImGuiCol_Button,buttonSelected);
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, buttonUnselected);
			}

			ImGui::SameLine();
			if (ImGui::ImageButton("ScaleModeTopBar", (ImTextureID)scaleIconId, sizeButton))
			{
				m_GizmoMode = GizmoMode::Scale;
			}

			ImGui::PopStyleColor();

			ImGui::End();
		}

		window_flags |= ImGuiWindowFlags_MenuBar;
		if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, ImGui::GetFrameHeight(), window_flags)) {
			if (ImGui::BeginMenuBar()) {
				ImGui::Text("Bottom status bar");
				ImGui::EndMenuBar();
			}
			ImGui::End();
		}
	}

	void EditorControl::HandelGizmoTransforms(Czuch::Entity& currentSelectedEntity)
	{
		if (currentSelectedEntity.IsValid())
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
			auto currentCamera = m_Root->GetScenesManager().GetActiveScene()->FindEditorCamera();

			if (currentCamera != nullptr)
			{
				auto entity = currentCamera->GetEntity();
				auto& cameraTransform = entity.GetComponent<TransformComponent>();
				auto cameraProjection = currentCamera->GetCamera().GetProjectionMatrix();
				auto &cam = currentCamera->GetCamera();
				cameraProjection = glm::perspective(glm::radians(cam.GetFov()), ImGui::GetWindowWidth() / (float)ImGui::GetWindowHeight(), cam.GetNearPlane(), cam.GetFarPlane());
				auto cameraView = currentCamera->GetCamera().GetInverseViewMatrix();

				auto& selectedTransform = currentSelectedEntity.GetComponent<TransformComponent>();
				auto selectedTransformMatrix = selectedTransform.GetLocalToWorld();

				auto operationSpace = ImGuizmo::MODE::LOCAL;

				auto diffMatrix = Mat4x4(1.0f);

				float matrixTranslation_[3], matrixRotation_[3], matrixScale_[3];
				ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(selectedTransformMatrix), matrixTranslation_, matrixRotation_, matrixScale_);
				bool manipulating=ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), GetCurrentGizmoMode(), operationSpace, glm::value_ptr(selectedTransformMatrix), glm::value_ptr(diffMatrix), nullptr);

				if (ImGuizmo::IsUsing())
				{
					m_IsGizmoActive = true;
					float matrixTranslation[3], matrixRotation[3], matrixScale[3];
					ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(selectedTransformMatrix), matrixTranslation, matrixRotation, matrixScale);
					float matrixTranslation_1[3], matrixRotation_1[3], matrixScale_1[3];
					ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(diffMatrix), matrixTranslation_1, matrixRotation_1, matrixScale_1);


					Vec3 currentWorldPos = selectedTransform.GetLocalPosition();
					//Vec3 diff = Vec3(matrixTranslation[0] - matrixTranslation_[0], matrixTranslation[1] - matrixTranslation_[1], matrixTranslation[2] - matrixTranslation_[2]);
					//Vec3 diffScale = Vec3(matrixScale_1[0] / matrixScale_[0], matrixScale_1[1] / matrixScale_[1], matrixScale_1[2] / matrixScale_[2]);
					Vec3 diffScale = Vec3(matrixScale_1[0], matrixScale_1[1], matrixScale_1[2]);
					Vec3 diff = Vec3(matrixTranslation_1[0], matrixTranslation_1[1], matrixTranslation_1[2]);


					if (GetCurrentGizmoMode() == ImGuizmo::OPERATION::ROTATE)
					{
						Mat4x4 localTransform = selectedTransform.TransformToLocalSpace(selectedTransformMatrix);
						Vec3 rot = glm::eulerAngles(glm::quat_cast(localTransform));

						selectedTransform.Rotate(rot.x, operationSpace == ImGuizmo::MODE::WORLD ? selectedTransform.GetRight() : Vec3(1, 0, 0), operationSpace == ImGuizmo::MODE::WORLD ? Czuch::TransformSpace::World : Czuch::TransformSpace::Local);
						selectedTransform.Rotate(rot.y, operationSpace == ImGuizmo::MODE::WORLD ? selectedTransform.GetUp() : Vec3(0, 1, 0), operationSpace == ImGuizmo::MODE::WORLD ? Czuch::TransformSpace::World : Czuch::TransformSpace::Local);
						selectedTransform.Rotate(rot.z, operationSpace == ImGuizmo::MODE::WORLD ? selectedTransform.GetForward() : Vec3(0, 0, 1), operationSpace == ImGuizmo::MODE::WORLD ? Czuch::TransformSpace::World : Czuch::TransformSpace::Local);
					}
					else if (GetCurrentGizmoMode() == ImGuizmo::OPERATION::SCALE)
					{
						selectedTransform.Scale(diffScale);
					}
					else if (GetCurrentGizmoMode() == ImGuizmo::OPERATION::TRANSLATE)
					{
						if (operationSpace == ImGuizmo::MODE::LOCAL)
						{
							diff = selectedTransform.TransformPointToLocalSpace(diff);
						}
						selectedTransform.Translate(diff, operationSpace == ImGuizmo::MODE::WORLD ? Czuch::TransformSpace::World : Czuch::TransformSpace::Local);
					}

					//selectedTransform.SetLocalEulerAngles(Vec3(matrixRotation[0], matrixRotation[1], matrixRotation[2]));
					//selectedTransform.SetLocalScale(Vec3(matrixScale[0], matrixScale[1], matrixScale[2]));

					selectedTransform.ForceUpdateLocalTransform();
				}
				else
				{
					if (m_IsGizmoActive)
					{
						m_IsGizmoActive = false;
						auto command = NEW(ChangeTransformCommand((Scene*)currentSelectedEntity.GetScene(), currentSelectedEntity, m_GizmoFrame.position, m_GizmoFrame.rotation, m_GizmoFrame.scale));
						EditorCommandsControl::Get().ExecuteCommand(command);
					}

					if (ImGuizmo::IsOver())
					{
						m_GizmoFrame.position = selectedTransform.GetLocalPosition();
						m_GizmoFrame.rotation = selectedTransform.GetLocalEulerAngles();
						m_GizmoFrame.scale = selectedTransform.GetLocalScale();
					}
				}
			}
		}
	}

	void EditorControl::FillMainMenubar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ShowMenuFile();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "CTRL+Z", false, EditorCommandsControl::Get().CanUndo()))
				{
					EditorCommandsControl::Get().Undo();
				}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, EditorCommandsControl::Get().CanRedo()))
				{
					EditorCommandsControl::Get().Redo();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "CTRL+X")) {}
				if (ImGui::MenuItem("Copy", "CTRL+C")) {}
				if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Debug"))
			{
				if (ImGui::MenuItem("Show Commands Stack", "CTRL+P")) {
					m_ShowCommandsStackPopup = true;
				}

				if (ImGui::MenuItem("Show Console Log", "CTRL+L")) {
					m_ShowConsoleLogPanel = true;
				}

				if (ImGui::MenuItem("Render Graph Editor", "CTRL+R")) {
					m_RenderGraphEditorWindow->SetWindowVisible(true);
				}

				if (ImGui::MenuItem("Assets Editor", "CTRL+A")) {
					m_AssetsEditorWindow->SetWindowVisible(true);
				}

				if (ImGui::MenuItem("Assets Info", "CTRL+I")) {
					m_AssetsInfoEditorWindow->SetWindowVisible(true);
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
	void EditorControl::ShowMenuFile()
	{
		if (ImGui::MenuItem("New"))
		{
			CheckCurrentSceneForSave();
			m_Root->GetScenesManager().CreateNewScene("NewScene", true);
			m_SceneHierarchyPanel->SetActiveScene(m_Root->GetScenesManager().GetActiveScene());
		}
		if (ImGui::MenuItem("Open", "Ctrl+O"))
		{
			//m_Root->GetScenesManager().LoadScene("Assets/Scenes/testScene.scene");
			std::string path = WinUtils::GetOpenFileNameDialog("Scene Files\0*.scene\0", "Open Scene");
			if (!path.empty())
			{
				CheckCurrentSceneForSave();
				m_Root->GetScenesManager().RemoveScene(m_Root->GetScenesManager().GetActiveScene());
				m_Root->GetScenesManager().LoadScene(path);
				m_CurrentScenePath = path;
				m_SceneHierarchyPanel->SetActiveScene(m_Root->GetScenesManager().GetActiveScene());
			}
		}

		if (ImGui::MenuItem("Save", "Ctrl+S"))
		{
			if (m_CurrentScenePath.empty())
			{
				ShowSaveMenu();
			}
			else
			{
				m_Root->GetScenesManager().SaveActiveScene(m_CurrentScenePath);
			}
		}
		if (ImGui::MenuItem("Save As.."))
		{
			ShowSaveMenu();
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("Options"))
		{
			ImGui::EndMenu();
		}

		ImGui::Separator();
		if (ImGui::MenuItem("Quit", "Alt+F4"))
		{
			WindowClosedEvent::CreateAndDispatch();
		}
	}

	bool EditorControl::ShowSaveMenu()
	{
		std::string path = WinUtils::GetSaveFileNameDialog("Scene Files\0*.scene\0", "Save Scene");
		if (!path.empty())
		{
			m_Root->GetScenesManager().SaveActiveScene(path);
			m_CurrentScenePath = path;
			return true;
		}
		return false;
	}

	void EditorControl::ShowCommandsStackPopup()
	{
		if (ImGui::Begin("Commands Stack", &m_ShowCommandsStackPopup, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar))
		{
			ImGui::Text("Undo Stack");
			ImGui::Separator();
			auto commandsStack = m_CommandsControl->GetUndoCommandsBeginIterator();
			for (auto it = commandsStack; it != m_CommandsControl->GetUndoCommandsEndIterator(); ++it)
			{
				ImGui::Text((*it)->ToString().c_str());
			}
			ImGui::Separator();
			ImGui::Text("Redo Stack");
			ImGui::Separator();
			commandsStack = m_CommandsControl->GetRedoCommandsBeginIterator();
			for (auto it = commandsStack; it != m_CommandsControl->GetRedoCommandsEndIterator(); ++it)
			{
				ImGui::Text((*it)->ToString().c_str());
			}
		}
		ImGui::End();
	}

	void EditorControl::ShowConsoleLogPanel()
	{
		if (m_ShowConsoleLogPanel)
		{
			if (ImGui::Begin("Console Log", &m_ShowConsoleLogPanel, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar))
			{
				auto logger = Czuch::Logging::GetPtr();
				auto& logs = logger->GetLogMessages();
				for (auto& log : logs)
				{
					ImGui::Text(log.c_str());
				}

			}
			ImGui::End();
		}
	}

	bool EditorControl::CheckCurrentSceneForSave()
	{
		auto currentScene = m_Root->GetScenesManager().GetActiveScene();

		if (currentScene->IsDirty())
		{
			if (WinUtils::ShowYesNoDialog("Save Scene", "You have some unsaved changes in current scene.\nDo you want to save it ? "))
			{
				return ShowSaveMenu();
			}
		}
		return false;
	}

	bool EditorControl::UpdateOffscreenPass(U32 width, U32 height)
	{
		if (m_OffscreenPassInited)
		{
			if (m_Width != width || m_Height != height)
			{
				if (width == 0 || height == 0)
				{
					return false;
				}
				m_Width = width;
				m_Height = height;
				m_UpdateOffscreenPass(width, height);
				return false;
			}
			return true;
		}
		m_OffscreenPassInited = true;
		m_Root->GetRenderer().RegisterRenderPassResizeEventResponse(width, height, true, &m_UpdateOffscreenPass);
		return false;
	}

	ImGuizmo::OPERATION EditorControl::GetCurrentGizmoMode()
	{
		if (m_GizmoMode == GizmoMode::Translate)
		{
			return ImGuizmo::OPERATION::TRANSLATE;
		}
		else if (m_GizmoMode == GizmoMode::Rotate)
		{
			return ImGuizmo::OPERATION::ROTATE;
		}
		else if (m_GizmoMode == GizmoMode::Scale)
		{
			return ImGuizmo::OPERATION::SCALE;
		}
		return ImGuizmo::OPERATION::TRANSLATE;
	}

}
