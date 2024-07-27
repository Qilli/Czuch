#include "czpch.h"
#include "EditorControl.h"
#include"imgui.h"
#include"Platform/Windows/WinUtils.h"

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
		m_OffscreenPassAdded = false;
		m_SceneHierarchyPanel = nullptr;
		m_EntityInspectorPanel = nullptr;
	}
	EditorControl::~EditorControl()
	{
		delete m_CommandsControl;
		delete m_SceneHierarchyPanel;
		delete m_EntityInspectorPanel;
	}
	void EditorControl::Init(void* context)
	{
		ImGui::SetCurrentContext((ImGuiContext*)context);
		SetLightGreyStyle();
		m_CommandsControl = new EditorCommandsControl();
	}

	void EditorControl::Shutdown()
	{
	}

	void EditorControl::Update(TimeDelta timeDelta)
	{
	}

	void EditorControl::FillUI(void* sceneViewportTexture)
	{
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
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		ImGui::Begin("Scene");
		bool isFocused=ImGui::IsWindowFocused();
		bool isHovered = ImGui::IsWindowHovered();

		m_Root->GetUIBaseManager().SetBlockEvents(!isFocused || !isHovered);

		auto targetViewportSize = ImGui::GetContentRegionAvail();
		if (UpdateOffscreenPass((U32)targetViewportSize.x, (U32)targetViewportSize.y))
		{
			ImGui::Image(m_Root->GetRenderer().GetRenderPassResult(RenderPassType::OffscreenTexture), ImGui::GetContentRegionAvail());
		}

		ImGui::End();

		//scene hierarchy panel
		m_SceneHierarchyPanel->FillUI();
		//inspector panel
		m_EntityInspectorPanel->FillUI();


		ImGui::PopStyleVar();
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
				if (ImGui::MenuItem("Undo", "CTRL+Z",false, EditorCommandsControl::Get().CanUndo()))
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
		if (m_OffscreenPassAdded)
		{
			if (m_Width != width || m_Height != height)
			{
				m_Width = width;
				m_Height = height;
				m_UpdateOffscreenPass(width, height);
				return false;
			}
			return true;
		}
		m_OffscreenPassAdded = true;
		m_Root->GetRenderer().AddOffscreenRenderPass(nullptr, width, height, false,&m_UpdateOffscreenPass);
		return false;
	}

}
