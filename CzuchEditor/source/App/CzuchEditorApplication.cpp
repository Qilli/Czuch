#include"czpch.h"
#include<Czuch.h>
#include <typeinfo> 
#include <iostream> 
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "../EditorControl.h"
#include "imgui.h"


int main()
{
	Czuch::EngineRoot* root = new Czuch::EngineRoot();
	Czuch::EditorControl editorControl;
	root->Init("engineConfig.cfg",&editorControl);

	Czuch::Scene* scene = new Czuch::Scene("MainScene", root->GetRenderer().GetDevice());
	scene->SetAmbientColor(Vec4(0.1f, 0.1f, 0.1f, 1.0f));
	auto& sceneMgr = root->GetScenesManager();
	sceneMgr.AddScene(scene, true);

	//Add plane mesh
	Czuch::Entity planeEntity = scene->CreateEntity("GroundPlane");
	auto& meshComp = planeEntity.AddComponent<Czuch::MeshComponent>();
	meshComp.SetMesh(Czuch::DefaultAssets::PLANE_ASSET, Czuch::DefaultAssets::PLANE_HANDLE);
	auto& meshRendererComp = planeEntity.AddComponent<Czuch::MeshRendererComponent>();
	meshRendererComp.SetOverrideMaterial(Czuch::DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET);
	//scale it in x an z plane by 10 times
	planeEntity.GetComponent<Czuch::TransformComponent>().SetLocalScale({ 10.0f,1.0f,10.0f });

	//Add cube mesh
	Czuch::Entity cubeEntity = scene->CreateEntity("Cube");
	auto& cubeMeshComp = cubeEntity.AddComponent<Czuch::MeshComponent>();
	cubeMeshComp.SetMesh(Czuch::DefaultAssets::CUBE_ASSET, Czuch::DefaultAssets::CUBE_HANDLE);
	auto& cubeMeshRendererComp = cubeEntity.AddComponent<Czuch::MeshRendererComponent>();
	cubeMeshRendererComp.SetOverrideMaterial(Czuch::DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET);
	cubeEntity.GetComponent<Czuch::TransformComponent>().SetLocalPosition({ 0.0f,1.0f,0.0f });

	//Add directional light
	Czuch::Entity lightEntity = scene->CreateEntity("DirectionalLight");
	auto& lightComp = lightEntity.AddComponent<Czuch::LightComponent>();
	lightComp.SetLightType(Czuch::LightType::Directional);
	//rotate it 45 degrees down
	lightEntity.GetComponent<Czuch::TransformComponent>().Rotate(18.0f * DEG2RAD, { 1.0f,0.0f,0.0f }, Czuch::TransformSpace::World);

	root->Run();
	root->Shutdown();
	delete root;
	return 0;
}