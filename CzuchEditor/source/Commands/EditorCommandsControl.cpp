#include "EditorCommandsControl.h"

namespace Czuch
{
	void EditorCommandsControl::ExecuteCommand(IEditorCommand* command)
	{
		command->Execute();
		m_UndoCommands.emplace_back(command);
		if (m_UndoCommands.size() > m_MaxCommands)
		{
			auto command = m_UndoCommands.front();
			m_UndoCommands.pop_front();
			command->OnRemoveFromUndoStack();
			delete command;
		}
	}
	void EditorCommandsControl::Undo()
	{
		if (m_UndoCommands.empty())
		{
			return;
		}
		auto command = m_UndoCommands.back();
		command->Undo();
		m_UndoCommands.pop_back();
		m_RedoCommands.emplace_back(command);

		if (m_RedoCommands.size() > m_MaxCommands)
		{
			auto command = m_RedoCommands.front();
			m_RedoCommands.pop_front();
			command->OnRemoveFromRedoStack();
			delete command;
		}
	}
	void EditorCommandsControl::Redo()
	{
		if (m_RedoCommands.empty())
		{
			return;
		}
		auto command = m_RedoCommands.back();
		command->Execute();
		m_RedoCommands.pop_back();
		m_UndoCommands.emplace_back(command);
	}
	void EditorCommandsControl::Clear()
	{
		for (auto command : m_UndoCommands)
		{
			delete command;
		}
		m_UndoCommands.clear();
		for (auto command : m_RedoCommands)
		{
			delete command;
		}
		m_RedoCommands.clear();
	}
}
