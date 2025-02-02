#pragma once
#include"../IEditorCommand.h"
#include"Subsystems/Scenes/Scene.h"
#include"Subsystems/Assets/BuildInAssets.h"
#include"Subsystems/Assets/AssetsManager.h"
#include"Subsystems/Assets/AssetManagersTypes/ModelAssetManager.h"

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
			m_Entity.AddRenderable(DefaultAssets::PLANE_ASSET,DefaultAssets::PLANE_HANDLE, Czuch::DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET);
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
			m_Entity.AddRenderable(Czuch::DefaultAssets::CUBE_ASSET,Czuch::DefaultAssets::CUBE_HANDLE, DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET);
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

	class CreateNewEntityHierarchyWithModelCommand : public IEditorCommand
	{
	public:
		CreateNewEntityHierarchyWithModelCommand(Scene* scene, Entity parent, const CzuchStr& path) :m_Scene(scene), m_Parent(parent), m_Path(path) {}
		virtual ~CreateNewEntityHierarchyWithModelCommand() = default;

		void Execute() override
		{
			StringID strId = StringID::MakeStringID(m_Path);
			ModelAsset* asset = AssetsManager::GetPtr()->LoadAndGetResource<ModelAsset,ModelLoadSettings>(strId,ModelLoadSettings());
			if(asset==nullptr)
			{
				LOG_BE_ERROR("Failed to create instance of model asset with path: {0}", m_Path);
				return;
			}
			m_Entity = asset->AddModelToScene(m_Scene, m_Parent);
		}
		void Undo() override
		{
			m_Scene->DestroyEntity(m_Entity);
		}

		Entity GetCreatedRootEntity() { return m_Entity; }

		CzuchStr& ToString() override
		{
			if (m_Desc.empty())
			{
				m_Desc = "Cmd: Create Model Entity";
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
		CzuchStr m_Path;
		CzuchStr m_Desc;
	};

}