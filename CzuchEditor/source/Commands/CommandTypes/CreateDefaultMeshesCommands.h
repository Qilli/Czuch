#pragma once
#include"../IEditorCommand.h"
#include"Subsystems/Scenes/Scene.h"
#include"Subsystems/Assets/BuildInAssets.h"

namespace Czuch
{
	class CreateNewEntityWithPlaneMeshCommand : public IEditorCommand
	{
	public:
		CreateNewEntityWithPlaneMeshCommand(Scene* scene, Entity parent) :m_Scene(scene), m_Parent(parent) {}
		virtual ~CreateNewEntityWithPlaneMeshCommand() = default;

		void Execute() override
		{
			m_Entity = m_Scene->CreateEntity("Plane Entity", m_Parent);
			m_Entity.AddRenderable(Czuch::DefaultAssets::PLANE_HANDLE, Czuch::DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE);
		}
		void Undo() override
		{
			m_Scene->DestroyEntity(m_Entity);
		}

		Entity GetCreatedEntity() { return m_Entity; }

		CzuchStr& ToString() override
		{
			if (m_Desc.empty())
			{
				m_Desc = "Cmd: Create Plane Entity";
				return m_Desc;
			}
			else
			{
				return m_Desc;
			}
		}
	private:
		Entity m_Entity;
		Entity m_Parent;
		Scene* m_Scene;
		CzuchStr m_Desc;
	};

	class CreateNewEntityWithCubeMeshCommand : public IEditorCommand
	{
	public:
		CreateNewEntityWithCubeMeshCommand(Scene* scene, Entity parent) :m_Scene(scene), m_Parent(parent) {}
		virtual ~CreateNewEntityWithCubeMeshCommand() = default;

		void Execute() override
		{
			m_Entity = m_Scene->CreateEntity("Cube Entity", m_Parent);
			m_Entity.AddRenderable(Czuch::DefaultAssets::CUBE_HANDLE, Czuch::DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE);
		}
		void Undo() override
		{
			m_Scene->DestroyEntity(m_Entity);
		}

		Entity GetCreatedEntity() { return m_Entity; }

		CzuchStr& ToString() override
		{
			if (m_Desc.empty())
			{
				m_Desc = "Cmd: Create Cube Entity";
				return m_Desc;
			}
			else
			{
				return m_Desc;
			}
		}
	private:
		Entity m_Entity;
		Entity m_Parent;
		Scene* m_Scene;
		CzuchStr m_Desc;
	};

}