#pragma once

namespace Czuch {

	class IEditorCommand {
	public:
		virtual void Execute() = 0;
		virtual void Undo() = 0;
		virtual void OnRemoveFromUndoStack() {}
		virtual void OnRemoveFromRedoStack() {}
	};
}