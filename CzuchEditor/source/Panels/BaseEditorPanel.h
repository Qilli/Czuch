#pragma once

namespace Czuch
{
	class BaseEditorPanel
	{
	public:
		BaseEditorPanel() = default;
		virtual ~BaseEditorPanel() = default;
		virtual void FillUI() = 0;
	};
}

