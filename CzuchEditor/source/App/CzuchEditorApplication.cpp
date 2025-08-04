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
	auto& sceneMgr = root->GetScenesManager();
	sceneMgr.AddScene(scene, true);

	root->Run();
	root->Shutdown();
	delete root;
	return 0;
}