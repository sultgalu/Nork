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
			if (ImGui::DragFloat3("Scale", &(scale.x), 0.1f, 0.1f, 1000.0f, "%.3f"))
			{
				tr->SetScale(scale);
			}
			glm::vec3 rotationAxis = glm::axis(quat);
			float angle = glm::angle(quat);
			glm::vec3 rotateAmount = {0, 0, 0};
			if (ImGui::DragFloat3("Rotate", &rotateAmount.x, 0.01f))
			{
				tr->Rotate(glm::vec3(1, 0, 0), rotateAmount.x);
				tr->Rotate(glm::vec3(0, 1, 0), rotateAmount.y);
				tr->Rotate(glm::vec3(0, 0, 1), rotateAmount.z);
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
			if (!dr->model->meshes.empty())
			{
				if (ImGui::TreeNode("Material Textures"))
				{
					ImGui::Unindent();
					static int meshIdx = 0;
					auto meshCount = dr->model->meshes.size();
					if (meshIdx >= meshCount)
						meshIdx = meshCount - 1;
					ImGui::SliderInt("Mesh of model", &meshIdx, 0, meshCount - 1);

					auto meshPath = data.engine.resourceManager.PathFor(dr->model->meshes[meshIdx].mesh);
					auto matPath = data.engine.resourceManager.PathFor(dr->model->meshes[meshIdx].material);
					ImGui::Text("Mesh ID:"); ImGui::SameLine();
					ImGui::Text(meshPath.has_value() ? (*meshPath).c_str() : "unkown");
					ImGui::Text("Material ID:"); ImGui::SameLine();
					ImGui::Text(matPath.has_value() ? (*matPath).c_str() : "unkown");

					if (ImGui::SliderFloat3("diffuse", &dr->model->meshes[meshIdx].material->diffuse.r, 0, 1))
					{
						dr->model->meshes[meshIdx].material->Update();
					}
					if (ImGui::SliderFloat("specular", &dr->model->meshes[meshIdx].material->specular, 0, 10))
					{
						dr->model->meshes[meshIdx].material->Update();
					}
					if (ImGui::SliderFloat("spec-exp", &dr->model->meshes[meshIdx].material->specularExponent, 0, 1024, "%.0f", ImGuiSliderFlags_Logarithmic))
					{
						dr->model->meshes[meshIdx].material->Update();
					}
					if (ImGui::Button("Save Material"))
					{
						data.engine.resourceManager.SaveMaterial(dr->model->meshes[meshIdx].material);
					}
					if (ImGui::BeginTabBar("MaterialTexturesTab"))
					{
						auto displayTex = [&](Renderer::TextureMap type)
						{
							auto tex = dr->model->meshes[meshIdx].material->GetTextureMap(type);
							ImGui::Image((ImTextureID)tex->GetHandle(), ImVec2(imgSize, imgSize), ImVec2(0, 1), ImVec2(1, 0),
								ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
							ImGui::Text("Width: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->GetWidth()).c_str());
							ImGui::Text("Height: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->GetHeight()).c_str());
							ImGui::Text("Format: "); ImGui::SameLine(); ImGui::Text(Renderer::TextureFormatToString(tex->GetAttributes().format));

							ImGui::EndTabItem();
							if (ImGui::Button("Load texture"))
							{
								std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::Image, L"Load Texture", L"Load");
								if (!p.empty())
								{
									auto newTex = data.engine.resourceManager.GetTextureByPath(p);
									if (newTex != nullptr)
									{
										dr->model->meshes[meshIdx].material->SetTextureMap(newTex, type);
										dr->model->meshes[meshIdx].material->Update();
									}
								}
							}
							if (ImGui::Button("Import texture"))
							{
								std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::Image, L"Load Texture", L"Load");
								if (!p.empty())
								{
									data.engine.resourceManager.ImportTexture(p);
									auto newTex = data.engine.resourceManager.GetTexture(p);
									if (newTex != nullptr)
									{
										dr->model->meshes[meshIdx].material->SetTextureMap(newTex, type);
										dr->model->meshes[meshIdx].material->Update();
									}
								}
							}
						};
						if (ImGui::BeginTabItem("Diffuse"))
						{
							displayTex(Renderer::TextureMap::Diffuse);
						}
						if (ImGui::BeginTabItem("Normal"))
						{
							displayTex(Renderer::TextureMap::Normal);
						}
						if (ImGui::BeginTabItem("Roughness"))
						{
							displayTex(Renderer::TextureMap::Roughness);
						}
						if (ImGui::BeginTabItem("Metalness"))
						{
							displayTex(Renderer::TextureMap::Reflection);
						}
						ImGui::EndTabBar();
					}

					ImGui::DragInt("Image Size", &imgSize, 1, 50, 500);

					auto meshSrc = data.engine.resourceManager.IdFor(dr->model->meshes[meshIdx].mesh);
					ImGui::Text("Mesh: "); ImGui::SameLine();
					ImGui::Text(meshSrc.has_value() ? (*meshSrc).c_str() : "unkown");

					auto matSrc = data.engine.resourceManager.IdFor(dr->model->meshes[meshIdx].material);
					ImGui::Text("Material: "); ImGui::SameLine();
					ImGui::Text(matSrc.has_value() ? (*matSrc).c_str() : "unkown");

					if (ImGui::Button("Load Material"))
					{
						std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::glTF, L"Load Material", L"Load");
						if (!p.empty())
						{
							dr->model->meshes[meshIdx].material = data.engine.resourceManager.GetMaterialByPath(p);
						}
					}
					static char matNameBuf[100] = { 0 };
					if (ImGui::Button("Clone Material"))
					{
						auto cloneId = (*matSrc + "_");
						std::memset(matNameBuf, 0, sizeof(matNameBuf));
						std::memcpy(matNameBuf, cloneId.c_str(), cloneId.size());
						ImGui::OpenPopup("newNameForMat");
					}
					if (ImGui::BeginPopup("newNameForMat"))
					{
						if (ImGui::InputText("", matNameBuf, sizeof(matNameBuf), ImGuiInputTextFlags_EnterReturnsTrue))
						{
							auto clone = data.engine.resourceManager.CloneMaterial(dr->model->meshes[meshIdx].material, matNameBuf);
							if (clone != nullptr)
								dr->model->meshes[meshIdx].material = clone;
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
					ImGui::Indent();
					ImGui::TreePop();
				}
			}
			if (ImGui::Button("Add Mesh"))
			{
				std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::glTF, L"Add Mesh", L"Add");
				if (!p.empty())
				{
					Components::Mesh mesh;
					mesh.material = data.engine.resourceManager.GetMaterial("");
					mesh.mesh = data.engine.resourceManager.GetMesh(p);
					dr->model->meshes.push_back(mesh);
				}
			}
			auto src = data.engine.resourceManager.IdFor(dr->model);
			ImGui::Text(src.has_value() ? (*src).c_str() : "unkown");
			static char modelNamebuf[100] = { 0 };
			if (ImGui::Button("Clone Model"))
			{
				auto cloneId = (*src + "_");
				std::memset(modelNamebuf, 0, sizeof(modelNamebuf));
				std::memcpy(modelNamebuf, cloneId.c_str(), cloneId.size());
				ImGui::OpenPopup("newNameForModel");
			}
			if (ImGui::BeginPopup("newNameForModel"))
			{
				if (ImGui::InputText("", modelNamebuf, sizeof(modelNamebuf), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					auto clone = data.engine.resourceManager.CloneModel(dr->model, modelNamebuf);
					if (clone != nullptr)
						dr->model = clone;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			if (ImGui::Button("Save Model"))
			{
				data.engine.resourceManager.SaveModel(dr->model);
			}
			if (ImGui::Button("Load Model"))
			{
				std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::Json, L"Load model", L"Load");
				if (!p.empty())
				{
					dr->model = data.engine.resourceManager.GetModelByPath(p);
				}
			}
			if (ImGui::Button("Import Model"))
			{
				std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::_3D, L"Import model", L"Import");
				if (!p.empty())
				{
					dr->model = data.engine.resourceManager.ImportModel(p);
				}
			}
			if (ImGui::Button("Export Model"))
			{
				data.engine.resourceManager.ExportModel(dr->model);
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
	void InspectorPanel::PointLighComp(PointLight* pL)
	{
		if (ImGui::TreeNodeEx("PLight", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::ColorEdit4("Color##PlIght", (float*)&(pL->light->color.r)))
				pL->light->Update();
			int pow = pL->GetIntensity();
			if (ImGui::DragInt("Intensity", &(pow), 1, 0, 10000))
				pL->SetIntensity(pow);

			if (pL->shadow == nullptr)
			{
				if (ImGui::Button("Add shadow"))
				{
					data.selectedNode->GetEntity().AddComponent<PointShadowRequest>();
				}
			}
			else if (ImGui::TreeNode("Shadow"))
			{
				if (ImGui::SliderFloat("Bias", (float*)&(pL->shadow->bias), 0, 1))
					pL->shadow->Update();
				if (ImGui::SliderFloat("min Bias", (float*)&(pL->shadow->biasMin), 0, 1))
					pL->shadow->Update();
				if (ImGui::SliderFloat("Near", (float*)&(pL->shadow->near), 0, 1))
					pL->shadow->Update();
				if (ImGui::SliderFloat("Far", (float*)&(pL->shadow->far), 0, 1000, "%.1f", ImGuiSliderFlags_Logarithmic))
					pL->shadow->Update();

				auto tex = pL->shadow->shadowMap.Get();
				ImGui::Text("Width: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->GetWidth()).c_str());
				ImGui::Text("Height: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->GetHeight()).c_str());
				ImGui::Text("Format: "); ImGui::SameLine(); ImGui::Text(Renderer::TextureFormatToString(tex->GetAttributes().format));

				ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
				if (ImGui::Button("Delete"))
				{
					pL->shadow = nullptr;
				}
				ImGui::PopStyleColor();

				ImGui::TreePop();
			}

			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<PointLight>();
			}
			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}
	/*void InspectorPanel::PointShadowComp(PointShadow* comp, PointLight* light)
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
			if (ImGui::SliderInt("IDX", &comp->idx, 0, Renderer::Config::LightData::pointShadowsLimit - 1));
			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<DirShadow>();
			}
			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}*/
	void InspectorPanel::DirLightComp(DirLight* dL)
	{
		if (ImGui::TreeNodeEx("Directional light", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto dir = dL->light->direction;
			if (ImGui::SliderFloat3("Direction", &dir.x, -1.01, 1.01))
			{
				dL->light->direction = dir;
				//if (dir * dir != glm::zero<glm::vec3>()) // this logic should go into light
					//dL->light->direction = glm::normalize(dir);
				dL->RecalcVP(dL->GetView());
			}
			if (ImGui::SliderFloat("outOfProjValue", &dL->light->outOfProjValue, 0, 1))
				dL->light->Update();
			if (ImGui::ColorEdit4("Color", &(dL->light->color.r)))
				dL->light->Update();
			if (ImGui::DragFloat2("Left, Right", &dL->left))
				dL->RecalcVP(dL->GetView());
			if (ImGui::DragFloat2("Bottom, Top", &dL->bottom))
				dL->RecalcVP(dL->GetView());
			if (ImGui::DragFloat2("Near, Far", &dL->near))
				dL->RecalcVP(dL->GetView());

			if (dL->shadow == nullptr)
			{
				if (ImGui::Button("Add shadow"))
				{
					data.selectedNode->GetEntity().AddComponent<DirShadowRequest>();
				}
			}
			else if (ImGui::TreeNode("Shadow"))
			{
				if (ImGui::SliderFloat("Bias", (float*)&(dL->shadow->bias), 0, 1))
					dL->shadow->Update();
				if (ImGui::SliderFloat("min Bias", (float*)&(dL->shadow->biasMin), 0, 1))
					dL->shadow->Update();

				static auto imgSize = 100;
				auto tex = dL->shadow->shadowMap.Get();
				ImGui::Image((ImTextureID)tex->GetHandle(), ImVec2(imgSize, imgSize), ImVec2(0, 1), ImVec2(1, 0),
					ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
				ImGui::DragInt("Image Size", &imgSize, 1, 50, 500);
				ImGui::Text("Width: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->GetWidth()).c_str());
				ImGui::Text("Height: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->GetHeight()).c_str());
				ImGui::Text("Format: "); ImGui::SameLine(); ImGui::Text(Renderer::TextureFormatToString(tex->GetAttributes().format));
				
				ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
				if (ImGui::Button("Delete"))
				{
					data.selectedNode->GetEntity().RemoveComponent<DirShadowRequest>();
				}
				ImGui::PopStyleColor();
				
				ImGui::TreePop();
			}
			

			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<DirLight>();
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
	void InspectorPanel::ColliderComp(Collider* coll)
	{
		if (ImGui::TreeNodeEx("Collider", ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool changed = false;
			if (ImGui::Button("Set to cube"))
			{
				*coll = Collider::Cube();
			}
			if (ImGui::TreeNode("Points##OfCollider"))
			{
				auto flags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_ContextMenuInBody
					| ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH
					; // | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
				if (ImGui::BeginTable("Points##Table", 2, flags))
				{
					ImGui::TableSetupColumn("id");
					ImGui::TableSetupColumn("pos");
					ImGui::TableHeadersRow();

					auto& points = coll->PointsMutable();
					for (size_t i = 0; i < points.size(); i++)
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						std::string idx = std::to_string(i);
						ImGui::Text(idx.c_str());
						ImGui::TableSetColumnIndex(1);
						//ImGui::Text(idx.c_str());
						if (ImGui::DragFloat3(("##ASD" + idx).c_str(), &points[i].x, 0.001f))
						{
							changed = true;
						}
					}
					ImGui::EndTable();
					if (ImGui::Button("Add##ColliderPoint"))
					{
						coll->AddPoint(glm::vec3(0));
					}
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Faces##OfCollider"))
			{
				auto faces = coll->Faces();
				for (size_t i = 0; i < faces.size(); i++)
				{
					if (ImGui::TreeNode((std::to_string(i)).c_str()))
					{
						auto& face = faces[i];
						std::string normStr = "normal: ";
						for (size_t j = 0; j < 3; j++)
						{
							normStr += std::to_string(face.normal[j]).substr(0, 5) + ";";
						}
						ImGui::Text(normStr.c_str());
						auto& points = coll->PointsMutable();
						for (auto idx : face.points)
						{
							if (ImGui::DragFloat3(("##ASD2" + std::to_string(idx)).c_str(), &points[idx].x, 0.001f))
							{
								changed = true;
							}
						}
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
			if (changed)
			{
				coll->BuildTriangleFaces();
				coll->CombineFaces();
			}

			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Delete"))
			{
				data.selectedNode->GetEntity().RemoveComponent<Collider>();
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
			}
			if (ImGui::BeginPopup("NameChangedPopup"))
			{
				ImGui::Text("Tag Name Successfully changed", 1.0f);
				if (ImGui::Button("Close##SuccPopUp"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
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
			this->CompSelector<DirLight>();
			this->CompSelector<Camera>();
			this->CompSelector<Drawable>();
			this->CompSelector<Kinematic>();
			this->CompSelector<Collider>();

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
			auto* dL = ent.TryGetComponent<DirLight>();
			auto* name = ent.TryGetComponent<Tag>();
			auto* dr = ent.TryGetComponent<Drawable>();
			auto* cam = ent.TryGetComponent<Camera>();
			auto* kin = ent.TryGetComponent<Kinematic>();
			auto* poly = ent.TryGetComponent<Polygon>();
			auto* coll = ent.TryGetComponent<Collider>();

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
			if (coll != nullptr)
			{
				ColliderComp(coll);
				ImGui::Separator();
			}
			if (pL != nullptr)
			{
				PointLighComp(pL);
				ImGui::Separator();
			}
			if (dr != nullptr)
			{
				ModelComp(dr);
				ImGui::Separator();
			}
			if (dL != nullptr)
			{
				DirLightComp(dL);
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