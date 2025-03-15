#pragma once
#include "IEditorCommand.h"
#include"Core/Singleton.h"
#include<list>
namespace Czuch
{
	class EditorCommandsControl : public Singleton<EditorCommandsControl>
	{
	public:
		EditorCommandsControl(int maxCommands=30):m_MaxCommands(maxCommands) {}
		~EditorCommandsControl() { Clear(); }
		void ExecuteCommand(IEditorCommand* command);
		void Undo();
		void Redo();
		void Clear();
		bool CanUndo() { return !m_UndoCommands.empty(); }
		bool CanRedo() { return !m_RedoCommands.empty(); }
		std::list<IEditorCommand*>::iterator GetUndoCommandsBeginIterator() { return m_UndoCommands.begin(); }
		std::list<IEditorCommand*>::iterator GetRedoCommandsBeginIterator() { return m_RedoCommands.begin(); }
		std::list<IEditorCommand*>::iterator GetUndoCommandsEndIterator() { return m_UndoCommands.end(); }
		std::list<IEditorCommand*>::iterator GetRedoCommandsEndIterator() { return m_RedoCommands.end(); }
	private:
		std::list<IEditorCommand*> m_UndoCommands;
		std::list<IEditorCommand*> m_RedoCommands;
		int m_MaxCommands;
	};

}

