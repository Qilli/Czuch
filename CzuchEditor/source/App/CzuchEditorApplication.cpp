#include"czpch.h"
#include<Czuch.h>
#include <typeinfo> 
#include <iostream> 
#include <glm.hpp>
#include "../vendors/glm/gtc/matrix_transform.hpp"
#include "../EditorControl.h"
#include "imgui.h"


int main()
{
	Czuch::EngineRoot* root = new Czuch::EngineRoot();
	Czuch::EditorControl editorControl;
	root->Init("engineConfig.cfg",&editorControl);

	Czuch::Scene *scene= new Czuch::Scene("MainScene", &root->GetRenderSettings());

	scene->CreateEntity("Cube1");
	scene->CreateEntity("Cube2");
	auto e=scene->CreateEntity("Cube3");
	scene->CreateEntity("Cube4",e);

	auto& sceneMgr = root->GetScenesManager();
	sceneMgr.AddScene(scene, true);

	root->Run();
	root->Shutdown();
	delete root;
	return 0;
}