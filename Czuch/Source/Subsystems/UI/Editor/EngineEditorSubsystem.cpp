#include "czpch.h"
#include "EngineEditorSubsystem.h"
#include"EngineRoot.h"
#include "imgui.h"
#include "Subsystems/UI/ImGUIManager.h"
#include"EngineRoot.h"
#include "Renderer/Renderer.h"

namespace Czuch
{
	EngineEditorSubsystem::EngineEditorSubsystem(EngineEditorControl* editorControl) : m_Root(EngineRoot::GetPtr()), m_EditorControl(editorControl)
	{
	}

	void EngineEditorSubsystem::Init(RenderSettings* settings)
	{
		BaseSubsystem::Init(settings);
		if (m_EditorControl != nullptr)
		{
			m_EditorControl->Init(m_Root->GetUIBaseManager().GetUIContext(),settings);
		}
	}

	void EngineEditorSubsystem::AfterSystemInit()
	{
		if (m_EditorControl != nullptr)
		{
			m_EditorControl->AfterSystemInit();
		}

	}

	void EngineEditorSubsystem::Shutdown()
	{
		if (m_EditorControl != nullptr)
		{
			m_EditorControl->Shutdown();
		}
	}

	void EngineEditorSubsystem::Update(TimeDelta timeDelta)
	{
		if (m_EditorControl != nullptr)
		{
			m_EditorControl->Update(timeDelta);
		}
	}

	void EngineEditorSubsystem::FillUI()
	{
		if (m_EditorControl != nullptr)
		{
			m_EditorControl->FillUI(nullptr);
		}
	}

}
