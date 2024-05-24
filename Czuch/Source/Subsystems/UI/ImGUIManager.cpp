#include "czpch.h"
#include "ImGUIManager.h"
#include "./Renderer/Vulkan/VulkanDevice.h"
#include "imgui.h"
#include "./Platform/Windows/WinWindow.h"
#include"Platform/GLFW/ImGuiGLFWBackend.h"
#include"Platform/Vulkan/ImGuiVulkanBackend.h"


namespace Czuch
{
	ImGUIManager::ImGUIManager(GraphicsDevice* device, Window* wnd): UIBaseManager(),m_Device(static_cast<VulkanDevice*>(device))
	{
	
	}

	ImGUIManager::~ImGUIManager()
	{
	}

	void ImGUIManager::Init()
	{
		m_Device->InitImGUI();
	}

	void ImGUIManager::Update(TimeDelta timeDelta)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();
		ImGui::Render();
	}

	void ImGUIManager::Shutdown()
	{
		m_Device->ShutdownImGUI();
	}


}

