#include"czpch.h"
#include<Czuch.h>
#include <typeinfo> 
#include <iostream> 
#include <glm.hpp>
#include "../vendors/glm/gtc/matrix_transform.hpp"

class EventHandler : public Czuch::IEventsListener
{
	// Odziedziczono za poœrednictwem elementu IEventsListener
	virtual void OnEvent(const Czuch::Event& e) override
	{
		LOG_BE_ERROR(e.ToString());
		LOG_BE_INFO("Test log only gfdgdfgdfg");
	}
};



int main()
{

	Czuch::EngineRoot* root = new Czuch::EngineRoot();
	root->Init("engineConfig.cfg");

	//Czuch::Scene scene("MainScene");

	Czuch::Renderer& renderer = root->GetRenderer();
	Czuch::RenderContextCreateInfo renderContextCreateInfo{};
	renderContextCreateInfo.autoClearAfterRender = false;
	renderContextCreateInfo.renderLayer = Czuch::RenderLayer::LAYER_0;
	renderContextCreateInfo.renderType = Czuch::RenderType::General;

	Czuch::RenderContext renderContext(renderContextCreateInfo);
	renderer.RegisterRenderContext(&renderContext);

	Czuch::RenderObjectInstance cubeInstance{};
	cubeInstance.mesh = Czuch::DefaultAssets::CUBE_HANDLE;

	auto materialAsset = Czuch::AssetsManager::GetPtr()->GetAsset<Czuch::MaterialAsset>(Czuch::DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET);
	auto texAsset = Czuch::AssetsManager::GetPtr()->LoadAsset<Czuch::TextureAsset, Czuch::TextureLoadSettings>("/Textures/texture.jpg", Czuch::TextureLoadSettings{});
	auto texResource = Czuch::AssetsManager::GetPtr()->GetAsset<Czuch::TextureAsset>(texAsset);

	//ancientMaterial.paramsOverride.push_back(Czuch::MaterialParameterOverride{"MainTexture",Czuch::DescriptorType::SAMPLER,texResource->GetTextureAssetHandle().handle});

	Czuch::MaterialInstanceCreateSettings instanceCreateSettings{};
	instanceCreateSettings.materialInstanceName = "DefaultAncientMaterial";
	instanceCreateSettings.desc.AddSampler("MainTexture", texResource->GetTextureResourceHandle());
	instanceCreateSettings.desc.materialAsset = Czuch::DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET;
	instanceCreateSettings.desc.isTransparent = false;

	auto matInstanceHandle=Czuch::AssetsManager::GetPtr()->CreateAsset<Czuch::MaterialInstanceAsset>(instanceCreateSettings.materialInstanceName, instanceCreateSettings);
	auto instanceResource = Czuch::AssetsManager::GetPtr()->GetAsset<Czuch::MaterialInstanceAsset>(matInstanceHandle);
	cubeInstance.overrideMaterial = instanceResource->GetMaterialInstanceResourceHandle();

	Mat4x4 local = Mat4x4(1.0f);
	Mat4x4 world = glm::rotate(local, (float)glm::radians(-45.0f), glm::vec3{1,0,0 });
	Mat4x4 view = glm::translate(Mat4x4(1.0f),glm::vec3{ 0, 0, -2 });
	// camera projection
	Mat4x4 projection = glm::perspective(glm::radians(70.f), (float)1600 / (float)900, 1000.f, 0.1f);

	cubeInstance.localToClipSpaceTransformation = projection * view * world;

	//renderContext.AddToRenderList(cubeInstance);

	root->Run();
	root->Shutdown();
	delete root;
	return 0;
}