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

		void Init(RenderSettings* settings) override;
		void Update(TimeDelta timeDelta) override;
		void OnFinishFrame() override;
		void Shutdown() override;

		void AddScene(Scene* scene, bool setAsActive = false);
		void RemoveScene(Scene* scene);
		void SetActiveScene(Scene* scene);
		void CreateNewScene(const std::string& name,bool clearCurrent);
		Scene* GetActiveScene() const;
	public:
		bool SaveActiveScene(std::string& path);
		bool LoadScene(std::string& path);
	private:
		void DeactivateActiveScene();
	private:
		std::vector<Scene*> m_Scenes;
		Scene* m_ActiveScene;
		Renderer* m_Renderer;
		AssetsManager* m_AssetsManager;
	};
}


