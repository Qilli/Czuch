#pragma once
#include "IEditorCommand.h"
#include"Core/Singleton.h"
#include<list>
namespace Czuch
{
	class EditorCommandsControl : public Singleton<EditorCommandsControl>
	{
	public:
		EditorCommandsControl(int maxCommands=10):m_MaxCommands(maxCommands) {}
		~EditorCommandsControl() { Clear(); }
		void ExecuteCommand(IEditorCommand* command);
		void Undo();
		void Redo();
		void Clear();
	private:
		std::list<IEditorCommand*> m_UndoCommands;
		std::list<IEditorCommand*> m_RedoCommands;
		int m_MaxCommands;
	};

}

