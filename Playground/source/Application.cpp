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
		angle += 300.0f * delta.GetDeltaTime();
		transform.SetLocalEulerAngles(glm::vec3(glm::radians(0.0f), glm::radians(angle), 0.0f));
	}
private:
	float angle;

};

class DrawDebugLines : public Czuch::NativeBehaviour
{
public:

	Czuch::TransformComponent* transform;
	Czuch::TransformComponent* cameraTransform;
	DrawDebugLines()
	{
	}

	void OnUpdate(Czuch::TimeDelta delta) override
	{
		//transform->LookAt(cameraTransform->GetWorldPosition());
		//transform->Rotate(DEG2RAD * 90.0f, RIGHT);
	}

	void OnDebugDraw(Czuch::IDebugDrawBuilder* debugBuilder) override
	{
		//debugBuilder->DrawLine(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 3.0f, 0.0f), Color(1.0f, 0.0f, 0.0f,1.0f));
		//debugBuilder->DrawTriangle(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(3.0f, 3.0f, 0.0f), Color(1.0f, 0.0f, 0.0f, 0.5f));

		//debugBuilder->DrawQuad(glm::vec3(0.0f, 2.0f, 0.0f), glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)), 1.0f, Color(0.0f, 1.0f, 0.0f, 0.2f));
		//debugBuilder->DrawPoint(glm::vec3(0.0f, 2.0f, 0.0f), Color(1.0f, 1.0f, 1.0f, 1.0f),10.0f);
		//debugBuilder->DrawPoint(glm::vec3(0.0f, 3.0f, 0.0f), Color(1.0f, 1.0f, 1.0f, 1.0f),20.0f);

		//debugBuilder->DrawLinesSphere(glm::vec3(0.0f, 2.0f, 0.0f), 1.0f, Color(1.0f, 1.0f, 1.0f, 1.0f));

	}
};


int main()
{

	Czuch::EngineRoot* root = new Czuch::EngineRoot();
	root->Init("engineConfig.cfg", nullptr);

	auto& settings = Czuch::EngineRoot::GetEngineSettings();
	settings.debugSettings.SetDebugDrawOBBForMeshesEnabled(false);
	settings.debugSettings.SetDebugDrawNormalForMeshesEnabled(true);

	Czuch::Scene* scene = new Czuch::Scene("MainScene", root->GetRenderer().GetDevice());
	scene->SetAmbientColor(Vec4(0.01f, 0.01f, 0.01f, 1.0f));

	auto& sceneMgr = root->GetScenesManager();
	sceneMgr.AddScene(scene, true);

	auto& assetMgr = Czuch::AssetsManager::Get();

	//load sponza
	//auto sponzaHandle = assetMgr.LoadAsset<Czuch::ModelAsset>("Sponza\\sponza.gltf");
	//auto sponzaEntity = scene->AddModelToScene(sponzaHandle, "SponzaObject");

	auto texHandle = Czuch::AssetsManager::GetPtr()->Load2DTexture("Textures\\Orange\\texture_07.png");

	Czuch::MaterialInstanceCreateSettings instanceCreateSettings{};
	instanceCreateSettings.materialInstanceName = "DefaultAncientMaterial";
	instanceCreateSettings.desc.AddSampler("MainTexture", texHandle, false);
	Czuch::ColorUBO colorUBO;
	colorUBO.color = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	instanceCreateSettings.desc.AddBuffer("Color", Czuch::MaterialCustomBufferData((void*)&colorUBO, sizeof(Czuch::ColorUBO),Czuch::DescriptorBindingTagType::NONE));


	Czuch::MaterialObjectGPUData materialGPUData;
	materialGPUData.diffuseColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	materialGPUData.specularColor = Vec4(1.0f, 1.0f, 1.0f, 256.0f);
	Czuch::MaterialCustomBufferData materialData((void*)&materialGPUData, sizeof(Czuch::MaterialObjectGPUData), Czuch::DescriptorBindingTagType::MATERIALS_LIGHTING_DATA);

	instanceCreateSettings.desc.AddStorageBufferSingleData("MaterialsData", std::move(materialData));

	instanceCreateSettings.desc.materialAsset = Czuch::DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET;
	auto matInstanceHandle = Czuch::AssetsManager::GetPtr()->CreateMaterialInstance(instanceCreateSettings);


	Czuch::Entity planeEntity = scene->CreateEntity("PlaneObject");
	planeEntity.AddRenderable(Czuch::DefaultAssets::PLANE_ASSET, Czuch::DefaultAssets::PLANE_HANDLE, matInstanceHandle);
	planeEntity.Transform().SetLocalPosition(glm::vec3(0.0f, -2.0f, -5.0f));
	planeEntity.Transform().SetLocalScale(glm::vec3(10.0f, 1.0f, 10.0f));

	Czuch::Entity cubeEntity = scene->CreateEntity("CubeObject");
	cubeEntity.AddRenderable(Czuch::DefaultAssets::CUBE_ASSET, Czuch::DefaultAssets::CUBE_HANDLE, Czuch::DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET);
	cubeEntity.Transform().SetLocalPosition(glm::vec3(0.0f, 0.0f, 1.0f));
	auto& scripts = cubeEntity.AddComponent<Czuch::NativeBehaviourComponent>();
	RotateAround& rotateAround = scripts.AddNativeBehaviour<RotateAround>();
	rotateAround.SetEnabled(true);
	Czuch::NativeFree3DCameraController& camController = scripts.AddNativeBehaviour<Czuch::NativeFree3DCameraController>();

	Czuch::Entity cubeEntity2 = scene->CreateEntity("CubeObject2");
	cubeEntity2.AddRenderable(Czuch::DefaultAssets::CUBE_ASSET, Czuch::DefaultAssets::CUBE_HANDLE, Czuch::DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET);
	cubeEntity2.Transform().SetLocalPosition(glm::vec3(0.0f, 0.0f, -5.0f));

	Czuch::Entity planeEntity2 = scene->CreateEntity("PlaneObject2");
	planeEntity2.AddRenderable(Czuch::DefaultAssets::PLANE_ASSET, Czuch::DefaultAssets::PLANE_HANDLE, matInstanceHandle);
	planeEntity2.Transform().SetLocalPosition(glm::vec3(0.0f, 2.0f, 2.0f));
	planeEntity2.Transform().SetLocalScale(glm::vec3(5.0f, 1.0f,5.0f));
	planeEntity2.Transform().SetLocalEulerAngles(Vec3(-120, 0, 0));


	settings.debugSettings.SetDebugDrawSelectedEntityID(cubeEntity.GetID());



	//planeEntity.Transform().LookAt(glm::vec3(0.0f, 10.0f, 0.0f));
	//planeEntity.Transform().Rotate(DEG2RAD* 90.0f,RIGHT);

	//auto& debugLines = planeEntity.AddComponent<Czuch::NativeBehaviourComponent>().AddNativeBehaviour<DrawDebugLines>();
	//debugLines.transform = &planeEntity.Transform();

	//add simple entity with spot light
	Czuch::Entity spotLightEntity = scene->CreateEntity("SpotLightObject");
	spotLightEntity.Transform().SetLocalPosition(glm::vec3(0.0f, 1.0f, -8.0f));
	spotLightEntity.Transform().Rotate(DEG2RAD * 90.0f, RIGHT);

	auto& spot = spotLightEntity.AddComponent<Czuch::LightComponent>();
	spot.SetLightType(Czuch::LightType::Spot);
	spot.SetColor(Czuch::Colors::Red);
	spot.SetLightRange(20.0f);
	spot.SetInnerAngle(30.0f);
	spot.SetOuterAngle(35.0f);



	auto camComponent = scene->GetPrimaryCamera();
	auto camTransform = &camComponent->GetEntity().Transform();

	//debugLines.cameraTransform = camTransform;

	Czuch::Entity lightEntity = scene->CreateEntity("LightObject");
	lightEntity.Transform().SetLocalPosition(glm::vec3(0.0f, -1.0f, -4.0f));
	lightEntity.Transform().Rotate(0.0f);
	lightEntity.AddPointLight(Color(1.0f, 1.0f, 1.0f, 1.0f), 300.0f, 7.0f);
	//lightEntity.AddDirectionalLight(Czuch::Colors::White, 1.0f);

	/*Czuch::Entity additionalCameraEntity = scene->CreateEntity("AdditionalCameraObject");
	auto& cameraAdditional = additionalCameraEntity.AddComponent<Czuch::CameraComponent>();
	cameraAdditional.GetCamera().SetViewport(0.7f, 0.0f, 0.3f, 0.3f);*/

	//TODO : we need to refresh all rendering flags when new camera is added
	//root->GetRenderer().SetDebugRenderingFlag(Czuch::DebugRenderingFlag::MaterialIndexAsColor, true);


	root->Run();
	root->Shutdown();
	delete root;
	return 0;
}