#include "czpch.h"
#include "ScenesManager.h"
#include "Renderer/Renderer.h"
#include "Subsystems/Assets/AssetsManager.h"
#include"Core/Time.h"
#include"Serialization/SceneSerializer.h"

namespace Czuch
{
    ScenesManager::ScenesManager(Renderer* renderer, AssetsManager* assetsMgr)
    {
        m_Renderer = renderer;
		m_AssetsManager = assetsMgr;
    }

    ScenesManager::~ScenesManager()
	{
		for (auto scene : m_Scenes)
		{
			delete scene;
		}
	}

	bool ScenesManager::SaveActiveScene(std::string& path)
	{
		if (m_ActiveScene == nullptr)
		{
			return false;
		}
		SceneSerializer serializer(m_ActiveScene);
		m_ActiveScene->SetDirty(false);
		return serializer.Serialize(path, false);
	}

	bool ScenesManager::LoadScene(std::string& path)
	{
		Scene* scene = new Scene("NewScene",m_RenderSettings);
		SceneSerializer serializer(scene);
		if (serializer.Deserialize(path, false))
		{
			AddScene(scene, true);
			LOG_BE_INFO("Successfuly loaded scene from file: {0}", path);
			return true;
		}
		return false;
	}

	void ScenesManager::Init(RenderSettings* settings)
	{
		BaseSubsystem::Init(settings);
	}

	void ScenesManager::Update(TimeDelta timeDelta)
	{
		if (m_ActiveScene != nullptr)
		{
			m_ActiveScene->OnUpdate(timeDelta);
		}
	}

	void ScenesManager::OnFinishFrame()
	{
		if (m_ActiveScene != nullptr)
		{
			m_ActiveScene->OnFinishFrame();
		}
	}


	void ScenesManager::Shutdown()
	{
		for (auto scene : m_Scenes)
		{
			delete scene;
		}
		m_Scenes.clear();
		m_ActiveScene = nullptr;
	}

	void ScenesManager::AddScene(Scene* scene, bool setAsActive)
	{
		m_Scenes.push_back(scene);
		if (setAsActive)
		{
			SetActiveScene(scene);
		}
	}

	void ScenesManager::RemoveScene(Scene* scene)
	{
		if (m_ActiveScene == scene)
		{
			SetActiveScene(nullptr);
		}

		auto it = std::find(m_Scenes.begin(), m_Scenes.end(), scene);
		if (it != m_Scenes.end())
		{
			m_Scenes.erase(it);
			delete scene;
		}
	}

	void ScenesManager::DeactivateActiveScene()
	{
		if (m_ActiveScene != nullptr)
		{
			m_ActiveScene->OnDettached();
			m_Renderer->SetActiveScene(nullptr);
		}
	}

	void ScenesManager::SetActiveScene(Scene* scene)
	{
		if (scene == nullptr)
		{
			DeactivateActiveScene();
			m_ActiveScene = nullptr;
			return;
		}

		if (m_ActiveScene != scene)
		{
			DeactivateActiveScene();
			m_ActiveScene = scene;

			m_ActiveScene->OnAttached();
			m_Renderer -> SetActiveScene(m_ActiveScene);
		}
	}

	void ScenesManager::CreateNewScene(const std::string& name, bool clearCurrent)
	{
		if (clearCurrent)
		{
			RemoveScene(m_ActiveScene);
		}
		Scene* scene = new Scene(name, m_RenderSettings);
		AddScene(scene, true);
	}

	Scene* ScenesManager::GetActiveScene() const
	{
		return m_ActiveScene;
	}

}
