#include "BaseEditorPanel.h"
#include "EntityInspectorEditorPanel.h"
#include"imgui.h"
#include"imgui_internal.h"
#include"Subsystems/Scenes/Components/HeaderComponent.h"
#include"Subsystems/Scenes/Components/TransformComponent.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"
#include"Subsystems/Scenes/Components/MeshComponent.h"
#include"Subsystems/Scenes/Components/MeshRendererComponent.h"
#include"Subsystems/Scenes/Components/LightComponent.h"
#include"Subsystems/Assets/Asset/MaterialInstanceAsset.h"
#include"Subsystems/Assets/AssetsManager.h"
#include"Subsystems/Assets/Asset/ModelAsset.h"
#include"Subsystems/Assets/Asset/TextureAsset.h"
#include"../Commands/CommandTypes/ChangeTransformCommand.h"
#include"../Commands/EditorCommandsControl.h"
#include"../EditorCommon.h"

namespace Czuch
{

	void Czuch::EntityInspectorEditorPanel::FillUI()
	{
		if (IsEntityValid(m_SelectedEntity))
		{
			ImGui::Begin("Entity Inspector");

			// Backup the current style colors
			ImVec4 headerColor = ImGui::GetStyle().Colors[ImGuiCol_Header];
			ImVec4 headerHoveredColor = ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered];
			ImVec4 headerActiveColor = ImGui::GetStyle().Colors[ImGuiCol_HeaderActive];

			// Set new colors for the header
			ImGui::GetStyle().Colors[ImGuiCol_Header] = ImVec4(0, 0, 0, 0.2f); // Transparent background
			ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = ImVec4(0, 0, 0, 0.2f); // Transparent background
			ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = ImVec4(0, 0, 0, 0.2f); // Transparent background

			m_HeaderDrawer.DrawComponent(m_SelectedEntity);
			m_TransformDrawer.DrawComponent(m_SelectedEntity);
			m_CameraDrawer.DrawComponent(m_SelectedEntity);
			m_MeshDrawer.DrawComponent(m_SelectedEntity);
			m_MeshRendererDrawer.DrawComponent(m_SelectedEntity);
			m_LightDrawer.DrawComponent(m_SelectedEntity);

			ImGui::Separator();
			AddComponentMenu();

			// Restore the original style colors
			ImGui::GetStyle().Colors[ImGuiCol_Header] = headerColor;
			ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = headerHoveredColor;
			ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = headerActiveColor;

			ImGui::End();

			m_HeaderDrawer.DrawModalWindow();
			m_TransformDrawer.DrawModalWindow();
			m_CameraDrawer.DrawModalWindow();
			m_MeshDrawer.DrawModalWindow();
			m_MeshRendererDrawer.DrawModalWindow();
			m_LightDrawer.DrawModalWindow();

			if (m_CameraDrawer.removeComponent)
			{
				m_SelectedEntity.RemoveComponent<CameraComponent>();
				m_CameraDrawer.removeComponent = false;
			}

			if (m_MeshDrawer.removeComponent)
			{
				m_SelectedEntity.RemoveComponent<MeshComponent>();
				m_MeshDrawer.removeComponent = false;
			}

			if (m_MeshRendererDrawer.removeComponent)
			{
				m_SelectedEntity.RemoveComponent<MeshRendererComponent>();
				m_MeshRendererDrawer.removeComponent = false;
			}

			if (m_LightDrawer.removeComponent)
			{
				m_SelectedEntity.RemoveComponent<LightComponent>();
				m_LightDrawer.removeComponent = false;
			}
		}
		else
		{
			ImGui::Begin("Entity Inspector");
			ImGui::Text("No entity selected");
			ImGui::End();
		}
	}

	void EntityInspectorEditorPanel::SelectedEntityChanged(Entity entity)
	{
		m_SelectedEntity = entity;
		m_HeaderDrawer.OnSelectionChanged(entity);
	}

	void EntityInspectorEditorPanel::AddComponentMenu()
	{

		if (CustomDrawers::ButtonCenteredOnLine("Add Component", 0.5f))
		{
			ImGui::OpenPopup("AddComponentPopup");
		}

		if (ImGui::BeginPopup("AddComponentPopup"))
		{
			ImGui::SeparatorText("Add Component");
			if (ImGui::MenuItem(" Camera Component "))
			{
				m_SelectedEntity.AddComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem(" Mesh Component "))
			{
				m_SelectedEntity.AddComponent<MeshComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem(" Mesh Renderer Component "))
			{
				m_SelectedEntity.AddComponent<MeshRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem(" Light Component "))
			{
				m_SelectedEntity.AddComponent<LightComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::Dummy(ImVec2(0.0f, 3.0f));
			ImGui::EndPopup();
		}
	}


	void HeaderDrawer::DrawComponent(Entity entity)
	{
		if (entity.HasComponent<HeaderComponent>())
		{
			auto& header = entity.GetComponent<HeaderComponent>();

			bool open = DrawComponentHeader(m_HeaderText.c_str(), entity);

			if (open)
			{
				static char nameBuffer[256];
				std::string currentName = header.GetHeader();
				memset(nameBuffer, 0, sizeof(nameBuffer));
				strcpy_s(nameBuffer, sizeof(nameBuffer), currentName.c_str());
				nameBuffer[currentName.length()] = '\0';  // Ensure null-terminated string

				int flags = ImGuiInputTextFlags_EnterReturnsTrue;
				if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer), flags))
				{
					header.SetHeader(nameBuffer);
					UpdateTexts(entity);
				}
			}
		}
	}

	void HeaderDrawer::OnSelectionChanged(Entity entity)
	{
		if (entity.IsValid() && entity.HasComponent<HeaderComponent>())
		{
			UpdateTexts(entity);
		}
	}

	void HeaderDrawer::UpdateTexts(Entity entity)
	{
		auto& header = entity.GetComponent<HeaderComponent>();
		strncpy(m_CurrentName, header.GetHeader().c_str(), sizeof(m_CurrentName));
		m_CurrentName[sizeof(m_CurrentName) - 1] = '\0';  // Ensure null-terminated string
		m_HeaderText = "Header Component: " + header.GetHeader();
	}

	void TransformDrawer::DrawComponent(Entity entity)
	{
		if (entity.HasComponent<TransformComponent>())
		{
			auto& transform = entity.GetComponent<TransformComponent>();
			bool open = DrawComponentHeader("Transform Component", entity);
			if (open)
			{
				bool changed = false;

				if (CustomDrawers::DrawVector3("Position", transform.GetLocalPosition(), 80.0f, 0.0f, m_Position))
				{
					transform.ForceUpdateLocalTransform();
					changed = true;
				}
				if (CustomDrawers::DrawVector3("Rotation", transform.GetLocalEulerAngles(), 80.0f, 0.0f, m_Rotation))
				{
					transform.ForceUpdateLocalTransform();
					changed = true;
				}
				if (CustomDrawers::DrawVector3("Scale", transform.GetLocalScale(), 80.0f, 1.0f, m_Scale))
				{
					transform.ForceUpdateLocalTransform();
					changed = true;
				}

				if (changed)
				{
					auto command = NEW(ChangeTransformCommand((Scene*)entity.GetScene(), entity, m_Position.value, m_Rotation.value, m_Scale.value));
					m_Position.isLocked = false;
					m_Rotation.isLocked = false;
					m_Scale.isLocked = false;
					m_Position.value = transform.GetLocalPosition();
					m_Rotation.value = transform.GetLocalEulerAngles();
					m_Scale.value = transform.GetLocalScale();
					EditorCommandsControl::Get().ExecuteCommand(command);
				}
				transform.ForceUpdateLocalTransform();
			}
		}
	}

	void TransformDrawer::OnSelectionChanged(Entity entity)
	{
	}

#pragma region CameraDrawer

	void CameraDrawer::DrawComponent(Entity entity)
	{
		if (entity.HasComponent<CameraComponent>())
		{
			auto& camComponent = entity.GetComponent<CameraComponent>();
			bool open = DrawComponentHeader("Camera Component", entity);
			if (open)
			{
				auto& camera = camComponent.GetCamera();
				// Display and edit Near Plane
				float nearPlane = camera.GetNearPlane();
				if (ImGui::DragFloat("Near Plane", &nearPlane, 0.1f, 0.01f, 100.0f, "%.1f"))
				{
					camera.SetNearPlane(nearPlane);
				}

				// Display and edit Far Plane
				float farPlane = camera.GetFarPlane();
				if (ImGui::DragFloat("Far Plane", &farPlane, 0.1f, 1.0f, 10000.0f, "%.1f"))
				{
					camera.SetFarPlane(farPlane);
				}

				// Display and edit Field of View (FOV)
				float fov = camera.GetFov();
				if (ImGui::DragFloat("Field of View", &fov, 0.1f, 1.0f, 180.0f, "%.1f"))
				{
					camera.SetVerticalFov(fov);
				}

			}
		}
	}


	void CameraDrawer::OnSelectionChanged(Entity entity)
	{
	}

	void CameraDrawer::OnRemoveComponent(Entity entity)
	{
		removeComponent = true;
	}

#pragma endregion

	bool ComponentDrawer::DrawComponentHeader(const char* name, Entity entity)
	{
		int collapseFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap;
		bool open = ImGui::CollapsingHeader(name, collapseFlags);
		ImGui::SameLine();
		ImGui::PushID(id);
		if (CustomDrawers::ButtonCenteredOnLine("+", 1.0f))
		{
			LOG_BE_INFO("Opening popup");
			ImGui::OpenPopup(name);
		}

		if (ImGui::BeginPopup(name))
		{
			ImGui::SeparatorText("Actions");
			ImGui::Dummy(ImVec2(0.0f, 2.0f));
			if (ImGui::MenuItem(" Delete Component "))
			{
				OnRemoveComponent(entity);
			}
			ImGui::Dummy(ImVec2(0.0f, 2.0f));
			ImGui::Separator();
			ImGui::EndPopup();
		}
		ImGui::PopID();
		return open;
	}
	void ComponentDrawer::DrawModalWindow()
	{
		if (m_ModalData.show)
		{
			CustomDrawers::ShowModalWindow(m_ModalData.title.c_str(), m_ModalData.text.c_str(), m_ModalData.show);
		}
	}


#pragma region MeshComponent
	void MeshInspectorDrawer::DrawComponent(Entity entity)
	{
		if (entity.HasComponent<MeshComponent>())
		{
			auto& meshComponent = entity.GetComponent<MeshComponent>();
			m_MeshAssetHelper.m_MeshComponent = &meshComponent;
			bool open = DrawComponentHeader("Mesh Component", entity);
			if (open)
			{
				m_MeshAssetHelper.ShowSelectAsset();
			}
		}
	}
	void MeshInspectorDrawer::OnSelectionChanged(Entity entity)
	{

	}
	void MeshInspectorDrawer::OnRemoveComponent(Entity entity)
	{
		removeComponent = true;
	}
#pragma endregion

#pragma region MeshRendererComponent
	void MeshRendererInspectorDrawer::DrawComponent(Entity entity)
	{
		if (entity.HasComponent<MeshRendererComponent>())
		{
			auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();
			m_MaterialAssetHelper.m_MeshRendererComponent = &meshRendererComponent;
			bool open = DrawComponentHeader("Mesh Renderer Component", entity);
			if (open)
			{
				m_MaterialAssetHelper.ShowSelectAsset();
				DrawMaterialInstance(meshRendererComponent);
			}
		}
	}

	void MeshRendererInspectorDrawer::OnSelectionChanged(Entity entity)
	{
	}

	void MeshRendererInspectorDrawer::OnRemoveComponent(Entity entity)
	{
		removeComponent = true;
	}

	void MeshRendererInspectorDrawer::DrawMaterialInstance(MeshRendererComponent& meshRendererComponent)
	{
		auto materialAsset = AssetsManager::Get().GetAsset<MaterialInstanceAsset>(meshRendererComponent.GetMaterialAsset());
		if (materialAsset != nullptr)
		{
			ImGui::Text(" Parameters: ");
			ImGui::Separator();
			bool changed = false;

			for (int i = 0; i < materialAsset->GetParametersCount(); i++)
			{
				auto paramType = materialAsset->GetParameterAtIndexType(i);
				if (paramType == MaterialParamType::PARAM_BUFFER && materialAsset->IsParameterAtIndexInternal(i)==false)
				{
					auto param = materialAsset->GetUBOBufferAtIndex(i);
					auto buffer = std::get<0>(param);
					auto layout = std::get<1>(param);
					if (buffer != nullptr && layout != nullptr)
					{
						for (int j = 0; j < layout->elementsCount; j++)
						{
							auto element = layout->GetElement(j);
							if (element.elementType == UBOElementType::VectorType)
							{
								auto vec = buffer->desc.customData->GetVec4(element.offset);
								if (ImGui::DragFloat4(element.name.GetStrName().c_str(), &vec->x, 0.1f))
								{
									changed = true;
								}
							}
							else if(element.elementType == UBOElementType::ColorType)
							{
								auto vec = buffer->desc.customData->GetVec4(element.offset);
								if (CustomDrawers::DrawLinearColor(element.name.GetStrName().c_str(), vec))
								{
									changed = true;
								}
							}

							if (changed)
							{
								materialAsset->UpdateUBOBufferAtIndex(i);
							}
						}
					}
				}
				else if(paramType == MaterialParamType::PARAM_TEXTURE )
				{
					m_TextureAssetHelper.SetMaterialInstance(materialAsset, i);

					auto param = materialAsset->GetTextureAssetAtIndex(i);
					auto textureAsset = AssetsManager::Get().GetAsset<TextureAsset>(std::get<0>(param));
					if (textureAsset != nullptr)
					{
						m_TextureAssetHelper.ShowSelectAsset();
					}
				}
			}

			ImGui::Separator();
			if (ImGui::Button("Copy Material"))
			{
				auto newMaterial=materialAsset->CloneMaterialInstance();
				meshRendererComponent.SetOverrideMaterial(newMaterial->GetHandle());
			}
			
		}
	}
#pragma endregion

#pragma region Light Component
	void LightInspectorDrawer::DrawComponent(Entity entity)
	{
		if (entity.HasComponent<LightComponent>())
		{
			auto& lightComponent = entity.GetComponent<LightComponent>();
			bool open = DrawComponentHeader("Light Component", entity);
			if (open)
			{
				auto& light = lightComponent;
				// Display and edit Light Type
				LightType lightType = light.GetLightType();
				if (ImGui::BeginCombo("Light Type", GetLightTypeString(lightType)))
				{
					for (int i = 0; i < (int)LightType::Count; i++)
					{
						bool isSelected = lightType == (LightType)i;
						if (ImGui::Selectable(GetLightTypeString((LightType)i), isSelected))
						{
							light.SetLightType((LightType)i);
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				// Display and edit Light Color
				Color lightColor = light.GetColor();

				if (CustomDrawers::DrawLinearColor("Light Color", &lightColor) == true)
				{
					light.SetColor(lightColor);
				}

				if (lightType != LightType::Directional)
				{
					// Display and edit Light Range
					float lightRange = light.GetLightRange();
					if (ImGui::DragFloat("Light Range", &lightRange, 0.1f, 0.0f, 1000.0f, "%.1f"))
					{
						light.SetLightRange(lightRange);
					}
				}


				if (lightType == LightType::Spot)
				{
					// Display and edit Light Spot Inner Angle
					float lightSpotInnerAngle = light.GetInnerAngle()*RAD2DEG;
					if (ImGui::DragFloat("Light Spot Inner Angle", &lightSpotInnerAngle, 0.1f, 0.0f, 180.0f, "%.1f"))
					{
						light.SetInnerAngle(lightSpotInnerAngle*DEG2RAD);
					}

					float lightSpotOuterAngle = light.GetOuterAngle()*RAD2DEG;
					if (ImGui::DragFloat("Light Spot Outer Angle", &lightSpotOuterAngle, 0.1f, 0.0f, 180.0f, "%.1f"))
					{
						light.SetOuterAngle(lightSpotOuterAngle*DEG2RAD);
					}
				}
			}
		}
	}
	void LightInspectorDrawer::OnSelectionChanged(Entity entity)
	{
	}
	void LightInspectorDrawer::OnRemoveComponent(Entity entity)
	{
		removeComponent = true;
	}
#pragma endregion

}