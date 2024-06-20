#include "czpch.h"
#include "ImGUIManager.h"
#include "./Renderer/Vulkan/VulkanDevice.h"
#include "imgui.h"
#include "./Platform/Windows/WinWindow.h"
#include"backends/imgui_impl_glfw.h"
#include"backends/imgui_impl_vulkan.h"
#include"Subsystems/Scenes/Scene.h"
#include"UIBaseElement.h"
#include"Editor/EngineEditorSubsystem.h"
#include"Subsystems/EventsManager.h"


namespace Czuch
{
	Scene* currentScene = nullptr;
	ImGUIManager::ImGUIManager(GraphicsDevice* device, Window* wnd): UIBaseManager(),m_Device(static_cast<VulkanDevice*>(device)),m_Window(static_cast<WinWindow*>(wnd))
	{
	
	}

	ImGUIManager::~ImGUIManager()
	{
		
	}

	void ImGUIManager::Init()
	{
		m_UIContext=m_Device->InitImGUI();
		LISTEN_TO_ALL_EVENTS(this)
	}

	void ImGUIManager::Update(TimeDelta timeDelta)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

;
		if (m_EditorModeEnabled)
		{
			EngineEditorSubsystem::GetPtr()->FillUI();
		}

		if (currentScene != nullptr)
		{
			for (auto element : currentScene->GetSceneUIElements())
			{
				element->UpdateUI(timeDelta);
			}
			currentScene = nullptr;
		}

		ImGui::Render();
	}

	void ImGUIManager::Shutdown()
	{
		m_Device->ShutdownImGUI();
	}

	void ImGUIManager::SetSceneForUI(Scene* scene)
	{
		currentScene = scene;
	}

	void ImGUIManager::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.SetHandled(io.WantCaptureMouse && e.GetCategory() == EventCategoryType::Mouse);
			e.SetHandled(io.WantCaptureKeyboard && e.GetCategory() == EventCategoryType::Keyboard);
		}
	}
}

