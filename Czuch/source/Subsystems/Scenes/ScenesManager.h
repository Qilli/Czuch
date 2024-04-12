#pragma once
#include "Scene.h"
#include"Subsystems/BaseSubsystem.h"

namespace Czuch
{
	class Renderer;
	class AssetsManager;

	class CZUCH_API ScenesManager : public BaseSubsystem<ScenesManager>
	{
	public:
		ScenesManager(Renderer* renderer, AssetsManager* assetsMgr);
		~ScenesManager();

		void Init() override;
		void Update() override;
		void Shutdown() override;

		void AddScene(Scene* scene, bool setAsActive = false);
		void RemoveScene(Scene* scene);
		void SetActiveScene(Scene* scene);
		Scene* GetActiveScene() const;
	private:
		void DeactivateActiveScene();
	private:
		std::vector<Scene*> m_Scenes;
		Scene* m_ActiveScene;
		Renderer* m_Renderer;
		AssetsManager* m_AssetsManager;
	};
}


