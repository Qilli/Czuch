#pragma once
#include"Core/EngineCore.h"

namespace Czuch {

	class IEditorCommand {
	public:
		virtual ~IEditorCommand() = default;
		virtual void Execute() = 0;
		virtual void Undo() = 0;
		virtual void OnRemoveFromUndoStack() {}
		virtual void OnRemoveFromRedoStack() {}
		virtual CzuchStr& ToString() = 0;
	};
}