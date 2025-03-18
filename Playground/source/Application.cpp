#include"czpch.h"
#include<Czuch.h>
#include <typeinfo> 
#include <iostream> 
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class EventHandler : public Czuch::IEventsListener
{
	// Odziedziczono za poœrednictwem elementu IEventsListener
	virtual void OnEvent(Czuch::Event& e) override
	{
		LOG_BE_ERROR(e.ToString());
		LOG_BE_INFO("Test log only gfdgdfgdfg");
	}
};

class DemoImGUI : public Czuch::UIBaseElement
{
public:
	void UpdateUI(Czuch::TimeDelta timeDelta) override
	{
		//ImGui::ShowDemoWindow();
	}
};

class RotateAround : public Czuch::NativeBehaviour
{
public:
	RotateAround()
	{
		angle = 10.0f;
	}

	void OnUpdate(Czuch::TimeDelta delta) override
	{
		auto& transform = GetComponent<Czuch::TransformComponent>();
		angle+=30.0f * delta.GetDeltaTime();
		transform.SetLocalEulerAngles(glm::vec3(glm::radians(0.0f), glm::radians(angle), 0.0f));
	}
private:
	float angle;

};


int main()
{

	Czuch::EngineRoot* root = new Czuch::EngineRoot();
	root->Init("engineConfig.cfg",nullptr);

	Czuch::Scene *scene= new Czuch::Scene("MainScene",&root->GetRenderSettings());

	auto& sceneMgr = root->GetScenesManager();
	sceneMgr.AddScene(scene, true);

	auto& assetMgr = Czuch::AssetsManager::Get();

	//load sponza
	auto sponzaHandle = assetMgr.LoadAsset<Czuch::ModelAsset>("Sponza\\sponza.gltf");
    auto sponzaEntity = scene->AddModelToScene(sponzaHandle, "SponzaObject");

	auto texHandle = Czuch::AssetsManager::GetPtr()->Load2DTexture("Textures\\texture.jpg");

	Czuch::MaterialInstanceCreateSettings instanceCreateSettings{};
	instanceCreateSettings.materialInstanceName = "DefaultAncientMaterial";
	instanceCreateSettings.desc.AddSampler("MainTexture", texHandle,false);
	Czuch::ColorUBO colorUBO;
	colorUBO.color = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	instanceCreateSettings.desc.AddBuffer("Color", Czuch::UBO((void*)&colorUBO, sizeof(Czuch::ColorUBO)));
	instanceCreateSettings.desc.materialAsset = Czuch::DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET;
	auto matInstanceHandle = Czuch::AssetsManager::GetPtr()->CreateMaterialInstance(instanceCreateSettings);

	Czuch::Entity cubeEntity = scene->CreateEntity("CubeObject");
	cubeEntity.AddRenderable(Czuch::DefaultAssets::CUBE_ASSET, Czuch::DefaultAssets::CUBE_HANDLE,matInstanceHandle);
	cubeEntity.Transform().SetLocalPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	auto& scripts=cubeEntity.AddComponent<Czuch::NativeBehaviourComponent>();
	RotateAround& rotateAround = scripts.AddNativeBehaviour<RotateAround>();
	rotateAround.SetEnabled(true);
	Czuch::NativeFree3DCameraController& camController = scripts.AddNativeBehaviour<Czuch::NativeFree3DCameraController>();

	Czuch::Entity planeEntity = scene->CreateEntity("PlaneObject");
	planeEntity.AddRenderable(Czuch::DefaultAssets::PLANE_ASSET,Czuch::DefaultAssets::PLANE_HANDLE, matInstanceHandle);
	planeEntity.Transform().SetLocalPosition(glm::vec3(0.0f, -1.0f, 0.0f));
	planeEntity.Transform().SetLocalScale(glm::vec3(10.0f, 1.0f, 10.0f));

	root->Run();
	root->Shutdown();
	delete root;
	return 0;
}