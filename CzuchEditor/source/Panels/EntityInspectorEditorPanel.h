#pragma once
#include "BaseEditorPanel.h"

namespace Czuch
{

	struct ComponentDrawer
	{
		bool isOpen = true;
		virtual void DrawComponent(Entity entity) {}
		virtual void OnSelectionChanged(Entity entity) {}
	};

	struct HeaderDrawer: public ComponentDrawer
	{
		void DrawComponent(Entity entity) override;
		void OnSelectionChanged(Entity entity) override;

		void UpdateTexts(Entity entity);

		char m_CurrentName[256];
		std::string m_HeaderText;
	};

	struct TransformDrawer : public ComponentDrawer
	{
		void DrawComponent(Entity entity) override;
		void OnSelectionChanged(Entity entity) override;
	};


	class EntityInspectorEditorPanel :
		public BaseEditorPanel
	{
	public:
		EntityInspectorEditorPanel() = default;
		~EntityInspectorEditorPanel() = default;
		void FillUI() override;
		void SelectedEntityChanged(Entity entity) override;
	private:
		HeaderDrawer m_HeaderDrawer;
		TransformDrawer m_TransformDrawer;
	};
}


