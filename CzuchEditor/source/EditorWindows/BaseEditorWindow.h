#pragma once

namespace Czuch
{
	class EngineRoot;
	class BaseEditorWindow
	{
	public:
		BaseEditorWindow();
		virtual ~BaseEditorWindow() = default;
		void SetWindowName(const char* name) { m_WindowName = name; }
		virtual void SetWindowVisible(bool visible) { m_IsVisible = visible; }
		virtual void DrawWindow(EngineRoot* root);
		virtual void FillUI() = 0;
		virtual bool IsVisible() const { return m_IsVisible; }
		virtual void Init(EngineRoot* root) = 0;
	private:
		const char* m_WindowName;
		bool m_IsVisible;
		bool m_IsInited;
	};

}