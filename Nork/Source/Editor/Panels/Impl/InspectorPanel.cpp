#include "pch.h"
#include "../InspectorPanel.h"
#include "Platform/FileDialog.h"

namespace Nork::Editor
{
	std::string GetName(const std::type_info& info)
	{
		std::string name = std::string(info.name());
		auto offs = name.find_last_of(':');
		return name.substr(offs + 1, name.size() - offs);
	}
	template<typename T>
	std::string GetCompName()
	{
		static std::string cached = GetName(typeid(T));
		return cached;
	}
	bool RenameButtonPopup(const char* button, std::string& str)
	{
		bool changed = false;
		static constexpr size_t bufSize = 100;
		static char buf[bufSize];
		if (ImGui::Button(button))
		{
			std::memcpy(buf, str.c_str(), str.size());
			std::memset(&buf[str.size()], 0, bufSize - str.size());
			ImGui::OpenPopup("rename");
		}
		if (ImGui::BeginPopup("rename"))
		{
			if (ImGui::InputText("", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				str = std::string(buf);
				changed = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		return changed;
	}

	void InspectorPanel::CameraComp(Camera* cam)
	{
		if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			/*if (ImGui::SliderFloat3("Position", &(tr->position.x), -20.0f, 20.0f))
			{
			}
			if (ImGui::SliderFloat3("Scale", &(tr->scale.x), 0.01f, 20.0f))
			{
			}
			if (ImGui::SliderFloat3("Rotation", &(tr->rotation.x), 0.01f, 20.0f))
			{
			}*/
			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			ImGui::DragFloat("FOV", &cam->FOV);
			ImGui::DragFloat("moveSpeed", &cam->moveSpeed);
			ImGui::DragFloat("ratio", &cam->ratio);
			ImGui::DragFloat3("position#camera", &cam->position.x);
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<Camera>();
			}
			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}
	void InspectorPanel::TransformComp(Transform* tr)
	{
		if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto pos = tr->GetPosition();
			auto scale = tr->GetScale();
			auto quat = tr->GetRotation();

			if (ImGui::DragFloat3("Position", &(pos.x), 0.1f))
			{
				tr->SetPosition(pos);
			}
			if (ImGui::DragFloat3("Scale", &(scale.x), 0.001f, 0.001f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic))
			{
				tr->SetScale(scale);
			}
			glm::vec3 rotationAxis = glm::axis(quat);
			float angle = glm::angle(quat);
			float rotateAmount = 0;
			if (ImGui::DragFloat("Rotate", &rotateAmount, 0.01f))
			{
				tr->Rotate(rotationAxis, rotateAmount);
			}
			if (ImGui::DragFloat("Set Rotation", &angle, 0.01f))
			{
				tr->SetRotation(glm::angleAxis(angle, glm::normalize(rotationAxis)));
			}
			if (ImGui::DragFloat3("Set Rotation Axis", &rotationAxis.x, 0.01f))
			{
				tr->SetRotation(glm::angleAxis(angle, glm::normalize(rotationAxis)));
			}
			if (ImGui::DragFloat4("Quaternion", &quat.w, 0.1f))
			{
				tr->SetRotation(glm::normalize(quat));
			}
			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<Transform>();
			}
			ImGui::PopStyleColor();
			ImGui::TreePop();
		}
	}
	void InspectorPanel::ModelComp(Drawable* dr)
	{
		if (ImGui::TreeNodeEx("Model", ImGuiTreeNodeFlags_DefaultOpen))
		{
			static int imgSize = 100;
			if (ImGui::TreeNode("Material Textures"))
			{
				ImGui::Unindent();
				static int meshIdx = 0;
				if (meshIdx >= dr->model.Meshes().size())
					meshIdx = dr->model.Meshes().size() - 1;
				ImGui::SliderInt("Mesh of model", &meshIdx, 0, dr->model.Meshes().size() - 1);
				if (ImGui::BeginTabBar("MaterialTexturesTab"))
				{
					if (ImGui::BeginTabItem("Diffuse"))
					{
						ImGui::Image((ImTextureID)dr->model.Meshes()[meshIdx].DiffuseMap()->GetHandle(), ImVec2(imgSize, imgSize), ImVec2(0, 1), ImVec2(1, 0),
							ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Normal"))
					{
						ImGui::Image((ImTextureID)dr->model.Meshes()[meshIdx].NormalMap()->GetHandle(), ImVec2(imgSize, imgSize), ImVec2(0, 1), ImVec2(1, 0),
							ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Roughness"))
					{
						ImGui::Image((ImTextureID)dr->model.Meshes()[meshIdx].RoughnessMap()->GetHandle(), ImVec2(imgSize, imgSize), ImVec2(0, 1), ImVec2(1, 0),
							ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Metalness"))
					{
						ImGui::Image((ImTextureID)dr->model.Meshes()[meshIdx].ReflectionMap()->GetHandle(), ImVec2(imgSize, imgSize), ImVec2(0, 1), ImVec2(1, 0),
							ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}

				ImGui::DragInt("Image Size", &imgSize, 1, 50, 500);
				ImGui::Indent();
				ImGui::TreePop();
			}

			ImGui::Text("Path: ");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0, 0.8f, 0.4f, 1.0f), "Showing source is not implemented yet");
			if (ImGui::Button("LoadFromWin"))
			{
				std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::_3D, L"Load model", L"Load");
				if (!p.empty())
				{
					MetaLogger().Error("implement this");
					//scene.RemoveComponent<Model>(data.selectedEnt);
					//scene.AddModel(data.selectedEnt, p);
				}
			}
			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<Drawable>();
			}
			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}
	void InspectorPanel::PointLighComp(PointLight* pL, PointShadow* shad)
	{
		if (ImGui::TreeNodeEx("PLight", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::ColorEdit4("Color##PlIght", (float*)&(pL->color.r))) {}
			int pow = pL->GetIntensity();
			if (ImGui::DragInt("Intensity", &(pow), 1, 0, 10000))
				pL->SetIntensity(pow);

			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<PointLight>();
			}
			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}
	void InspectorPanel::PointShadowComp(PointShadow* comp, PointLight* light)
	{
		if (ImGui::TreeNodeEx("Point shadow", ImGuiTreeNodeFlags_DefaultOpen))
		{
			static bool on = false;
			if (ImGui::Checkbox("WildCard", &on))
			{
				if (on)
					data.engine.renderingSystem.skybox = data.engine.renderingSystem.pointShadowMaps[comp->idx]->Get();
				else
					data.engine.renderingSystem.skybox = nullptr;
			}
			ImGui::DragFloat2("Far, Near", &comp->far, 0.001f, 0, 0, "%.3f");
			float bias = comp->bias;
			float biasMin = comp->biasMin;
			int blur = (int)comp->blur;
			float rad = comp->radius;
			ImGui::SliderFloat("Bias", &comp->bias, 0, 1, "%.5f", ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("Minimum bias", &comp->biasMin, 0, 1, "%.5f", ImGuiSliderFlags_Logarithmic);
			ImGui::SliderInt("Blur", &comp->blur, 0, 9);
			(ImGui::SliderFloat("Radius", &comp->radius, 0.0f, 1.0f));
			if (ImGui::SliderInt("IDX", &comp->idx, 0, 4));
			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<DirShadow>();
			}
			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}
	void InspectorPanel::DirLightComp(DirLight* dL, DirShadow* ds)
	{
		if (ImGui::TreeNodeEx("Directional light", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::SliderFloat3("Direction", (float*)&(dL->direction.x), -1, 1))
			{
				if (ds != nullptr)
					ds->RecalcVP(dL->GetView());
				// dL->SetDirection(std::forward<glm::vec3&>(glm::normalize(dL->GetData().direction)));
			}
			if (ImGui::ColorEdit4("Color", &(dL->color.r))) {}
			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<DirLight>();
			}
			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}
	void InspectorPanel::DirShadowComp(DirShadow* comp, DirLight* dl)
	{
		if (ImGui::TreeNodeEx("Directional shadow", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::DragFloat2("Left, Right", &comp->left))
			{
				comp->RecalcVP(dl->GetView());
			}
			if (ImGui::DragFloat2("Bottom, Top", &comp->bottom))
			{
				comp->RecalcVP(dl->GetView());
			}
			if (ImGui::DragFloat2("Near, Far", &comp->near))
			{
				comp->RecalcVP(dl->GetView());
			}
			float bias = comp->bias;
			float biasMin = comp->biasMin;
			int pcfSize = (int)comp->pcfSize;
			if (ImGui::SliderFloat("Bias", &bias, 0, 1, "%.5f", ImGuiSliderFlags_Logarithmic))
			{
				comp->SetBias(bias);
			}
			if (ImGui::SliderFloat("Minimum bias", &biasMin, 0, 1, "%.5f", ImGuiSliderFlags_Logarithmic))
			{
				comp->SetBiasMin(biasMin);
			}
			if (ImGui::SliderInt("PCF quality", &pcfSize, 0, 9))
			{
				comp->SetPcfSize(pcfSize);
			}
			if (ImGui::SliderInt("IDX", &comp->idx, 0, 4));
			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<DirShadow>();
			}
			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}
	void InspectorPanel::KinematicComp(Kinematic* comp)
	{
		if (ImGui::TreeNodeEx("Kinematic", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::DragFloat("Mass (kg)", &comp->mass, 0.01f);
			ImGui::DragFloat3("Velocity", &comp->velocity.x, 0.001f);
			auto axis = glm::normalize(comp->w);
			auto angle = glm::length(comp->w);
			if(ImGui::DragFloat3("Angular Velocity Axis", &axis.x))
			{
				comp->w = axis * angle;
			}
			
			if (ImGui::DragFloat("Angular Velocity Speed", &angle, 0.001f))
			{
				comp->w = axis * angle;
			}

			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<Kinematic>();
			}
			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}
	void InspectorPanel::PolyComp(Polygon* poly)
	{
		if (ImGui::TreeNodeEx("Poly", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if(ImGui::Button("Select"))
			{
				data.selectedPoly = poly;
			}

			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<Polygon>();
			}
			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}
	void InspectorPanel::NameComp(Tag* name)
	{
		if (ImGui::TreeNodeEx("Name", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::LabelText("name", name->tag.c_str());
			if (RenameButtonPopup("Rename", name->tag))
			{
				ImGui::OpenPopup("NameChangedPopup");
				if (ImGui::BeginPopup("NameChangedPopup"))
				{
					ImGui::Text("Tag Name Successfully changed", 1.0f);
					if (ImGui::Button("Close#SuccPopUp"))
					{
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
			}
			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<Tag>();
			}
			ImGui::PopStyleColor();

			ImGui::TreePop();
		}

	}
	template<typename T>
	void InspectorPanel::CompSelector()
	{
		if (ImGui::Selectable(GetCompName<T>().c_str(), false, data.selectedNode->GetEntity().HasAnyComponentsOf<T>() ? ImGuiSelectableFlags_Disabled : 0))
		{
			data.selectedNode->GetEntity().AddComponent<T>();
			ImGui::CloseCurrentPopup();
		}
	}
	void InspectorPanel::CompAdder()
	{
		if (ImGui::Button("Add component"))
		{
			ImGui::OpenPopup("components");
		}
		if (ImGui::BeginPopup("components"))
		{
			this->CompSelector<Tag>();
			this->CompSelector<Transform>();
			this->CompSelector<PointLight>();
			this->CompSelector<PointShadow>();
			this->CompSelector<DirLight>();
			this->CompSelector<DirShadow>();
			this->CompSelector<Camera>();
			this->CompSelector<Drawable>();
			this->CompSelector<Kinematic>();
			this->CompSelector<Polygon>();

			ImGui::EndPopup();
		}
	}
	void InspectorPanel::DrawContent()
	{
		entt::registry& reg = scene.registry;

		auto selected = data.selectedNode;

		using namespace Components;

		if (selected != nullptr)
		{
			auto& ent = selected->GetEntity();
			auto* tr = ent.TryGetComponent<Transform>();
			auto* pL = ent.TryGetComponent<PointLight>();
			auto* pS = ent.TryGetComponent<PointShadow>();
			auto* dL = ent.TryGetComponent<DirLight>();
			auto* dS = ent.TryGetComponent<DirShadow>();
			auto* name = ent.TryGetComponent<Tag>();
			auto* dr = ent.TryGetComponent<Drawable>();
			auto* cam = ent.TryGetComponent<Camera>();
			auto* kin = ent.TryGetComponent<Kinematic>();
			auto* poly = ent.TryGetComponent<Polygon>();

			if (tr != nullptr)
			{
				TransformComp(tr);
				ImGui::Separator();
			}
			if (kin != nullptr)
			{
				KinematicComp(kin);
				ImGui::Separator();
			}
			if (poly != nullptr)
			{
				PolyComp(poly);
				ImGui::Separator();
			}
			if (pL != nullptr)
			{
				PointLighComp(pL, pS);
				ImGui::Separator();
			}
			if (pS != nullptr)
			{
				PointShadowComp(pS, pL);
				ImGui::Separator();
			}
			if (dr != nullptr)
			{
				ModelComp(dr);
				ImGui::Separator();
			}
			if (dL != nullptr)
			{
				DirLightComp(dL, dS);
				ImGui::Separator();
			}
			if (dS != nullptr && dL != nullptr)
			{
				DirShadowComp(dS, dL);
				ImGui::Separator();
			}
			if (name != nullptr)
			{
				NameComp(name);
				ImGui::Separator();
			}
			if (cam != nullptr)
			{
				CameraComp(cam);
				ImGui::Separator();
			}

			CompAdder();
		}
		else
		{
			// Logger::Warning("The current value of selected entity is not pointing to a valid entity");
		}
	}
}