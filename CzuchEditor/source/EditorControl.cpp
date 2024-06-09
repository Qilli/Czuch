#include "czpch.h"
#include "EditorControl.h"
#include"imgui.h"

namespace Czuch
{
	EditorControl::EditorControl() : m_Root(EngineRoot::GetPtr())
	{
	}
	void EditorControl::Init(void* context)
	{
		ImGui::SetCurrentContext((ImGuiContext*)context);
	}

	void EditorControl::Shutdown()
	{
	}

	void EditorControl::Update(TimeDelta timeDelta)
	{
	}

	void EditorControl::FillUI(void* sceneViewportTexture)
	{
		FillMainMenubar();
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		ImGui::Begin("Scene");
		// Image((ImTextureID)VC.TC.DescriptorSet, ImGui::GetContentRegionAvail());
		//LOG_BE_INFO("Scene viewport texture: {0}", sceneViewportTexture);
		auto targetViewportSize = ImGui::GetContentRegionAvail();
		if (UpdateOffscreenPass((U32)targetViewportSize.x, (U32)targetViewportSize.y))
		{
			ImGui::Image(m_Root->GetRenderer().GetRenderPassResult(RenderPassType::OffscreenTexture), ImGui::GetContentRegionAvail());
		}

		ImGui::End();
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
				if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
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
		if (ImGui::MenuItem("New")) {}
		if (ImGui::MenuItem("Open", "Ctrl+O")) {}

		if (ImGui::BeginMenu("Open Recent"))
		{
			// ImGui::MenuItem("fish_hat.c");
		  // //  ImGui::MenuItem("fish_hat.inl");
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Save", "Ctrl+S")) {}
		if (ImGui::MenuItem("Save As..")) {}

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
