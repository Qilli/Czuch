#include "czpch.h"
#include "TextureAssetManager.h"
#include "../BuildInAssets.h"
#include "Renderer/GraphicsDevice.h"
#include"imgui.h"
namespace Czuch
{

	void TextureAssetManager::Init()
	{
		m_DefaultAsset = GetAsset(DefaultAssets::PINK_TEXTURE_ASSET);
	}

	TextureHandle TextureAssetManager::GetGlobalTextureHandleWithIndex(I32 globalIndex)
	{
		TextureHandle result;
		ExecuteOnAllAssets([&result,globalIndex](Asset* asset){
			TextureAsset* tex = dynamic_cast<TextureAsset*>(asset);
			if (tex != nullptr)
			{
				auto handle=	tex->GetTextureResourceHandle();
				if (handle.globalIndex == globalIndex)
				{
					result = handle;
				}
			}
			});
		return result;
	}

	Asset* TextureAssetManager::CreateAsset(const CzuchStr& path, BaseCreateSettings& settings)
	{
		TextureAsset* texRes = new TextureAsset(path, (TextureCreateSettings&)settings, m_Device, AssetsManager::GetPtr());
		StringID strId = StringID::MakeStringID(path);
		RegisterAsset(strId, texRes);
		return texRes;
	}

	void TextureAssetManager::DynamicGlobalTexturesDebugWindow::DrawDebugWindow()
	{
		if (ImGui::Begin(windowName.c_str(), &windowOpen))
		{
			ImGui::Text("Dynamic Global Textures");
			ImGui::Separator();
			ImGui::Text("Total Textures: %zu", mgr->GetAllAssetsCount());
			ImGui::Separator();
			

			if (ImGui::CollapsingHeader("Asset List"))
			{
				if (ImGui::BeginChild("ScrollableList", ImVec2(0, 200), true, ImGuiWindowFlags_AlwaysVerticalScrollbar))
				{
					mgr->ExecuteOnAllAssets([](Asset* asset)
						{
							TextureAsset* tex = dynamic_cast<TextureAsset*>(asset);
							ImGui::Text(tex->GetAssetLoadInfo().c_str());
						});
					ImGui::EndChild();
				}
			}
		
		}
		ImGui::End();
	}

	TextureAssetManager::TextureAssetManager(GraphicsDevice* device) :m_Device(device)
	{
		m_DebugWindow.mgr = this;
		m_DebugWindow.windowName = "Dynamic Global Textures";
		device->AddDebugWindow(&m_DebugWindow);
	}

}
