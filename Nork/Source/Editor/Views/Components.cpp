#include "include/Components.h"
#include "../Views/include/Helpers.h"
#include "Platform/FileDialog.h"
#include "Editor/Editor.h"
#include "../Panels/include/ColliderEditorPanel.h"

namespace Nork::Editor {

	SceneNodeView::SceneNodeView(std::shared_ptr<SceneNode> node)
		:node(node)
	{
		//node->GetEntity().Id()
	}
	
	void SceneNodeView::Content()
	{
		using namespace Components;
		Content<Transform, Drawable, DirLight, PointLight, Components::Physics, Camera, Tag>();
	}
	template<class... T>
	void SceneNodeView::Content()
	{
		EditComponents<T...>();
		ListComponentsForAddition<T...>();
	}

	template<> void SceneNodeView::ShowComponent(Components::DirLight& dirLight, bool& changed)
	{
		//dirLight.RecalcVP();
		// dirLight.light->Update();
		auto dir = dirLight.light->direction;
		changed |= ImGui::Checkbox("Sun", &dirLight.sun);
		changed |= ImGui::SliderFloat3("Direction", &dirLight.light->direction.x, -1.01, 1.01);
		changed |= ImGui::SliderFloat("Out Of Proj Value", &dirLight.light->outOfProjValue, 0, 1);
		changed |= ImGui::ColorEdit3("Color", &(dirLight.light->color.r));
		changed |= ImGui::DragFloat3("Position", &dirLight.position.x);
		changed |= ImGui::DragFloat3("Rectangle", &dirLight.rectangle.x);
		
		if (!node->GetEntity().HasComponent<Components::DirShadowMap>())
		{
			if (ImGui::Button("Add shadow"))
			{
				node->GetEntity().AddComponent<Components::DirShadowMap>();
			}
		}
		else if (ImGui::TreeNode("Shadow"))
		{
			auto& shadowMap = node->GetEntity().GetComponent<Components::DirShadowMap>().map;
			ImGui::DragFloat("Bias", (float*)&(shadowMap.shadow->bias), 0.001f);
			ImGui::DragFloat("min Bias", (float*)&(shadowMap.shadow->biasMin), 0.001f);

			auto tex = shadowMap.fb->Depth();
			static auto imgSize = 100;
			ImGui::Image((ImTextureID)tex->GetHandle(), ImVec2(imgSize, imgSize), ImVec2(0, 1), ImVec2(1, 0),
				ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			ImGui::DragInt("Image Size", &imgSize, 1, 50, 500);
			ImGui::Text("Width: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->GetWidth()).c_str());
			ImGui::Text("Height: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->GetHeight()).c_str());
			ImGui::Text("Format: "); ImGui::SameLine(); ImGui::Text(Renderer::TextureFormatToString(tex->GetAttributes().format));
			static bool fix = false;
			static glm::ivec2 newSize;
			static Renderer::TextureFormat newFormat;
			if (ImGui::Button("Change Resolution##DirLi"))
			{
				ImGui::OpenPopup("chgrespshad##dir");
				newSize = { tex->GetWidth() , tex->GetHeight() };
				newFormat = tex->GetAttributes().format;
			}
			if (ImGui::BeginPopup("chgrespshad##dir"))
			{
				auto formatSelector = [&](Renderer::TextureFormat format)
				{
					ImGui::RadioButton(Renderer::TextureFormatToString(format), (int*)&newFormat, (int)format);
				};
				formatSelector(Renderer::TextureFormat::Depth16); ImGui::SameLine();
				formatSelector(Renderer::TextureFormat::Depth24);
				formatSelector(Renderer::TextureFormat::Depth32); ImGui::SameLine();
				formatSelector(Renderer::TextureFormat::Depth32F);
				ImGui::Checkbox("Fix to VP ratio", &fix);
				if (fix)
				{
					ImGui::InputInt("New Size (1 Dimension)", &newSize.x);
					if (ImGui::Button("OK"))
					{
						node->GetEntity().AddComponent<Components::DirShadowMap>().FixTextureRatio(dirLight, glm::pow(newSize.x, 2));
						ImGui::CloseCurrentPopup();
					}
				}
				else
				{
					ImGui::InputInt("New Width", &newSize.x);
					ImGui::InputInt("New Height", &newSize.y);
					if (ImGui::Button("OK"))
					{
						shadowMap.SetFramebuffer(newSize.x, newSize.y, newFormat);
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndPopup();
			}
			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Remove Shadow Map##Dir"))
			{
				node->GetEntity().RemoveComponent<Components::DirShadowMap>();
			}
			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}
	template<> void SceneNodeView::ShowComponent(Components::PointLight& pointLight, bool& changed)
	{
		changed |= ImGui::ColorEdit4("Color##PlIght", (float*)&(pointLight.light->color.r));

		int pow = pointLight.GetIntensity();
		if (changed |= ImGui::DragInt("Intensity", &(pow), 1, 0, 10000))
			pointLight.SetIntensity(pow);

		if (!node->GetEntity().HasComponent<Components::PointShadowMap>())
		{
			if (ImGui::Button("Add shadow"))
			{
				node->GetEntity().AddComponent<Components::PointShadowMap>();
			}
		}
		else if (ImGui::TreeNode("Shadow"))
		{
			auto& shadowMap = node->GetEntity().GetComponent<Components::PointShadowMap>().map;
			ImGui::SliderFloat("Bias", (float*)&(shadowMap.shadow->bias), 0, 1);
			ImGui::SliderFloat("min Bias", (float*)&(shadowMap.shadow->biasMin), 0, 1);
			ImGui::SliderFloat("Near", (float*)&(shadowMap.shadow->near), 0, 1);
			ImGui::SliderFloat("Far", (float*)&(shadowMap.shadow->far), 0, 1000, "%.1f", ImGuiSliderFlags_Logarithmic);

			auto tex = shadowMap.fb->Depth();
			ImGui::Text("Size: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->GetWidth()).c_str());
			ImGui::Text("Format: "); ImGui::SameLine(); ImGui::Text(Renderer::TextureFormatToString(tex->GetAttributes().format));
			static int newSize;
			static Renderer::TextureFormat newFormat;
			if (ImGui::Button("Change Resolution"))
			{
				ImGui::OpenPopup("chgrespshad");
				newSize = tex->GetWidth();
				newFormat = tex->GetAttributes().format;
			}
			if (ImGui::BeginPopup("chgrespshad"))
			{
				ImGui::InputInt("New Size", &newSize);
				auto formatSelector = [&](Renderer::TextureFormat format)
				{
					ImGui::RadioButton(Renderer::TextureFormatToString(format), (int*)&newFormat, (int)format);
				};
				formatSelector(Renderer::TextureFormat::Depth16); ImGui::SameLine();
				formatSelector(Renderer::TextureFormat::Depth24);
				formatSelector(Renderer::TextureFormat::Depth32); ImGui::SameLine();
				formatSelector(Renderer::TextureFormat::Depth32F);
				if (ImGui::Button("OK"))
				{
					shadowMap.SetFramebuffer(newSize, newFormat);
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			if (ImGui::Button("Remove Shadow"))
			{
				node->GetEntity().RemoveComponent<Components::PointShadowMap>();
			}
			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}
	template<> void SceneNodeView::ShowComponent(Components::Transform& tr, bool& changed)
	{
		ImGui::Text("Pos: %f;%f;%f", tr.Position().x, tr.Position().y, tr.Position().z);
		ImGui::Text("Scale: %f;%f;%f", tr.Scale().x, tr.Scale().y, tr.Scale().z);
		ImGui::Text("Quaternion: %f;%f;%f;%f", tr.Quaternion().w, tr.Quaternion().x, tr.Quaternion().y, tr.Quaternion().z);

		ImGui::Separator();
		changed |= ImGui::DragFloat3("Position", &tr.localPosition.x, 0.1f);
		changed |= ImGui::DragFloat3("Scale", &tr.localScale.x, 0.1f, 0.1f, 1000.0f, "%.3f");
		ImGui::Separator();

		glm::vec3 rotateAmount = { 0, 0, 0 };
		if (ImGui::DragFloat3("Rotate Locally", &rotateAmount.x, 0.01f) && (changed = true))
		{
			tr.Rotate(glm::vec3(1, 0, 0), rotateAmount.x);
			tr.Rotate(glm::vec3(0, 1, 0), rotateAmount.y);
			tr.Rotate(glm::vec3(0, 0, 1), rotateAmount.z);
		}
		glm::vec3 rotateAmount2 = { 0, 0, 0 };
		if (ImGui::DragFloat3("Rotate Globally", &rotateAmount.x, 0.01f) && (changed = true))
		{
			glm::quat rot(1, rotateAmount);
			tr.localQuaternion = glm::normalize(rot) * tr.localQuaternion;
		}
		ImGui::Separator();
		glm::vec3 rotationAxis = tr.RotationAxis();
		float angle = tr.RotationAngleDegrees();
		if (ImGui::DragFloat("Angle", &angle, 0.1f) && (changed = true))
		{
			tr.SetRotation(glm::normalize(rotationAxis), glm::radians(angle));
		}
		if (ImGui::DragFloat3("Set Rotation Axis", &rotationAxis.x, 0.01f) && (changed = true))
		{
			tr.SetRotation(glm::normalize(rotationAxis), glm::radians(angle));
		}
		if (ImGui::DragFloat4("Quaternion", &tr.localQuaternion.x, 0.1f) && (changed = true))
		{
			tr.localQuaternion = glm::normalize(tr.localQuaternion);
		}
		ImGui::Separator();
		if (ImGui::Button("Reset Rotation") && (changed = true))
		{
			tr.localQuaternion = glm::qua(1.0f, glm::vec3(0));
		}
	}
	template<> void SceneNodeView::ShowComponent(Components::Camera& cam, bool& changed)
	{
		changed |= ImGui::DragFloat3("near#camera", &cam.nearClip);
		changed |= ImGui::DragFloat3("far#camera", &cam.farClip);
		changed |= ImGui::DragFloat3("pitch", &cam.pitch);
		changed |= ImGui::DragFloat3("yaw", &cam.yaw);
		changed |= ImGui::DragFloat("FOV", &cam.FOV);
		changed |= ImGui::DragFloat("ratio", &cam.ratio);
		changed |= ImGui::DragFloat("moveSpeed", &cam.moveSpeed);
		changed |= ImGui::DragFloat3("position#camera", &cam.position.x);
	}
	template<> void SceneNodeView::ShowComponent(Components::Drawable& dr, bool& changed)
	{
		static int imgSize = 100;
		if (!dr.model->meshes.empty())
		{
			if (ImGui::TreeNode("Material Textures"))
			{
				ImGui::Unindent();
				static int meshIdx = 0;
				auto meshCount = dr.model->meshes.size();
				if (meshIdx >= meshCount)
					meshIdx = meshCount - 1;
				ImGui::SliderInt("Mesh of model", &meshIdx, 0, meshCount - 1);

				auto meshPath = GetEngine().resourceManager.PathFor(dr.model->meshes[meshIdx].mesh);
				auto matPath = GetEngine().resourceManager.PathFor(dr.model->meshes[meshIdx].material);
				ImGui::Text("Mesh ID:"); ImGui::SameLine();
				ImGui::Text(meshPath.has_value() ? (*meshPath).c_str() : "unkown");
				ImGui::Text("Material ID:"); ImGui::SameLine();
				ImGui::Text(matPath.has_value() ? (*matPath).c_str() : "unkown");

				ImGui::ColorEdit3("diffuse", &dr.model->meshes[meshIdx].material->diffuse.r, ImGuiColorEditFlags_DefaultOptions_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
				ImGui::SliderFloat("specular", &dr.model->meshes[meshIdx].material->specular, 0, 10);
				ImGui::SliderFloat("spec-exp", &dr.model->meshes[meshIdx].material->specularExponent, 0, 1024, "%.0f", ImGuiSliderFlags_Logarithmic);
				
				if (ImGui::Button("Save Material"))
				{
					GetEngine().resourceManager.SaveMaterial(dr.model->meshes[meshIdx].material);
				}
				if (ImGui::BeginTabBar("MaterialTexturesTab"))
				{
					auto displayTex = [&](Renderer::TextureMap type)
					{
						auto tex = dr.model->meshes[meshIdx].material.GetTextureMap(type);
						ImGui::Image((ImTextureID)tex->GetHandle(), ImVec2(imgSize, imgSize), ImVec2(0, 1), ImVec2(1, 0),
							ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
						ImGui::Text("Width: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->GetWidth()).c_str());
						ImGui::Text("Height: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->GetHeight()).c_str());
						ImGui::Text("Format: "); ImGui::SameLine(); ImGui::Text(Renderer::TextureFormatToString(tex->GetAttributes().format));

						ImGui::EndTabItem();
						if (ImGui::Button("Default"))
						{
							dr.model->meshes[meshIdx].material.SetDefaultTexture(type);
						}
						if (ImGui::Button("Load texture"))
						{
							std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::Image, L"Load Texture", L"Load");
							if (!p.empty())
							{
								auto newTex = GetEngine().resourceManager.GetTextureByPath(p);
								if (newTex != nullptr)
								{
									dr.model->meshes[meshIdx].material.SetTextureMap(newTex, type);
								}
							}
						}
						if (ImGui::Button("Import texture"))
						{
							std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::Image, L"Load Texture", L"Load");
							if (!p.empty())
							{
								GetEngine().resourceManager.ImportTexture(p);
								auto newTex = GetEngine().resourceManager.GetTexture(p);
								if (newTex != nullptr)
								{
									dr.model->meshes[meshIdx].material.SetTextureMap(newTex, type);
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

				auto meshSrc = GetEngine().resourceManager.IdFor(dr.model->meshes[meshIdx].mesh);
				ImGui::Text("Mesh: "); ImGui::SameLine();
				ImGui::Text(meshSrc.has_value() ? (*meshSrc).c_str() : "unkown");

				auto matSrc = GetEngine().resourceManager.IdFor(dr.model->meshes[meshIdx].material);
				ImGui::Text("Material: "); ImGui::SameLine();
				ImGui::Text(matSrc.has_value() ? (*matSrc).c_str() : "unkown");

				if (ImGui::Button("Load Material"))
				{
					std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::glTF, L"Load Material", L"Load");
					if (!p.empty())
					{
						dr.model->meshes[meshIdx].material = GetEngine().resourceManager.GetMaterialByPath(p);
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
						auto clone = GetEngine().resourceManager.CloneMaterial(dr.model->meshes[meshIdx].material, matNameBuf);
						if (clone != dr.model->meshes[meshIdx].material)
							dr.model->meshes[meshIdx].material = clone;
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
				mesh.material = GetEngine().resourceManager.GetMaterial("");
				mesh.mesh = GetEngine().resourceManager.GetMesh(p);
				dr.model->meshes.push_back(mesh);
			}
		}
		auto src = GetEngine().resourceManager.IdFor(dr.model);
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
				auto clone = GetEngine().resourceManager.CloneModel(dr.model, modelNamebuf);
				if (clone != nullptr)
					dr.model = clone;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (ImGui::Button("Save Model"))
		{
			GetEngine().resourceManager.SaveModel(dr.model);
		}
		if (ImGui::Button("Load Model"))
		{
			std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::Json, L"Load model", L"Load");
			if (!p.empty())
			{
				dr.model = GetEngine().resourceManager.GetModelByPath(p);
			}
		}
		if (ImGui::Button("Import Model"))
		{
			std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::_3D, L"Import model", L"Import");
			if (!p.empty())
			{
				dr.model = GetEngine().resourceManager.ImportModel(p);
			}
		}
		if (ImGui::Button("Export Model"))
		{
			GetEngine().resourceManager.ExportModel(dr.model);
		}
	}
	template<> void SceneNodeView::ShowComponent(Components::Physics& phx, bool& changed)
	{
		auto& kin = phx.handle.Get().kinem;
		changed |= ImGui::DragFloat("Mass (kg)", &kin.mass, 0.01f);
		changed |= ImGui::DragFloat("I", &kin.I, 0.01f);
		changed |= ImGui::DragFloat("Elasticity", &kin.elasticity, 0.01f, 0, 2);
		changed |= ImGui::DragFloat("Friction", &kin.friction, 0.01f, 0, 2);
		changed |= ImGui::Checkbox("Gravity", &kin.applyGravity);
		changed |= ImGui::Checkbox("Static", &kin.isStatic);
		changed |= ImGui::DragFloat3("Velocity", &kin.velocity.x, 0.001f);
		auto axis = glm::normalize(kin.w);
		auto angle = glm::length(kin.w);
		changed |= ImGui::DragFloat3("Angular Velocity Axis", &kin.w.x);
		// if (changed |= ImGui::DragFloat3("Angular Velocity Axis", &axis.x))
		// {
		// 	kin.w = axis * angle;
		// }
		// 
		// if (changed |= ImGui::DragFloat("Angular Velocity Speed", &angle, 0.001f))
		// {
		// 	kin.w = axis * angle;
		// }
	}
	template<> void SceneNodeView::ShowComponent(Components::Tag & tag, bool& changed)
	{
		changed |= Helpers::TextEditable(tag.tag);
	}

	template<class T>
	bool _EditComponent(const T& comp, Entity& ent,
		std::function<void(T& comp, bool& changed)> fun)
	{
		auto copy = comp;
		bool changed = false;
		fun(copy, changed);
		if (changed)
			ent.ReplaceComponent(copy);
		return changed;
	}

	template<class T>
	std::string componentName;

	template<class T>
	bool SceneNodeView::EditComponent()
	{
		auto& ent = node->GetEntity();
		if (ent.HasComponent<T>())
		{
			auto treeFlags = ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_ClipLabelForTrailingButton; 
			if constexpr (std::is_same<T, Components::Transform>::value)
			{
				treeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
			}
			bool opened = ImGui::TreeNodeEx((componentName<T> +"##" + std::to_string((uint64_t)ent.Id())).c_str(), treeFlags);
			
			auto w = ImGui::CalcItemWidth();
			auto align = ImGui::GetCurrentWindow()->Size.x - 20.f;
			ImGui::SameLine(align > w ? align : w);
			ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
			
			if (ImGui::Button(("X##Delete" + componentName<T>).c_str()))
			{	
				ent.RemoveComponent<T>();
				if (opened)
				{
					ImGui::TreePop();
					opened = false;
				}
			}
			ImGui::PopStyleColor();

			if (opened)
			{
				std::function<void(T& comp, bool& changed)> fun = [&](T& copy, bool& changed)
				{
					ShowComponent(copy, changed);
					if constexpr (std::is_same<T, Components::Physics>::value)
					{
						if (ImGui::Button(std::string("Edit Collider").append(std::to_string((size_t)ent.Id())).c_str()))
						{
							Editor::Get().AddPanel(std::make_shared<ColliderEditorPanel>(ent));
						}
						if (ImGui::Button(std::string("Convert").append(std::to_string((size_t)ent.Id())).c_str()))
						{
							auto& coll = ent.GetComponent<Components::Physics>().LocalCollider();
							auto poly = PolygonBuilder(coll).BuildMesh();
							std::vector<Renderer::Data::Vertex> vertices;
							for (auto& vert : poly.vertices)
							{
								vertices.push_back(Renderer::Data::Vertex{
									.position = vert,
									.normal = glm::vec3(0, 1, 0),
									.texCoords = glm::vec2(0),
									.tangent = glm::vec3(1, 0, 0),
									});
							}
							//GetEngine().resourceManager.drawState.vaoWrapper
							auto& mesh = ent.GetComponent<Components::Drawable>().model->meshes[0].mesh;
							// mesh->GetVaoWrapper().GetVertexWrapper().Erase(mesh->GetVertexPtr());
							// mesh->GetVaoWrapper().GetIndexWrapper().Erase(mesh->GetIndexPtr());
							// auto vertPtr = mesh->GetVaoWrapper().GetVertexWrapper().Add(vertices.data(), vertices.size());
							// auto idxPtr = mesh->GetVaoWrapper().GetIndexWrapper().Add((uint32_t*)poly.triangles.data(), poly.triangles.size() * 3);
							// mesh = std::make_shared<Renderer::Mesh>(mesh->GetVaoWrapper(), vertPtr, idxPtr, vertices.size(), poly.triangles.size() * 3);
							// vbo.Erase()
						}
					}
				};
				bool edited = _EditComponent(ent.GetComponent<T>(), ent, fun);
				ImGui::TreePop();
				return edited;
			}
		}
		return false;
	}

	template<class T, class... Rest>
	void SceneNodeView::_EditComponents()
	{
		EditComponent<T>();
		if constexpr (sizeof...(Rest) > 0)
		{
			_EditComponents<Rest...>();
		}
	}
	template<class... T>
	void SceneNodeView::EditComponents()
	{
		_EditComponents<T...>();
	}
	template<class T, class... Rest>
	void _ListComponentsForAddition(Entity& ent)
	{
		if (ImGui::Selectable(componentName<T>.c_str(), false, ent.HasComponent<T>() ? ImGuiSelectableFlags_Disabled : 0))
		{
			ent.AddComponent<T>();
			ImGui::CloseCurrentPopup();
		}
		if constexpr (sizeof...(Rest) > 0)
		{
			_ListComponentsForAddition<Rest...>(ent);
		}
	}

	template<class... T>
	void SceneNodeView::ListComponentsForAddition()
	{
		if (ImGui::Button("Add component"))
		{
			ImGui::OpenPopup("components");
		}
		if (ImGui::BeginPopup("components"))
		{
			_ListComponentsForAddition<T...>(node->GetEntity());
			ImGui::EndPopup();
		}
	}

	template<> std::string componentName<Components::DirLight> = "Directional Light";
	template<> std::string componentName<Components::PointLight> = "Point Light";
	template<> std::string componentName<Components::Transform> = "Transform";
	template<> std::string componentName<Components::Camera> = "Camera";
	template<> std::string componentName<Components::Drawable> = "Drawable";
	template<> std::string componentName<Components::Physics> = "Physics";
	template<> std::string componentName<Components::Tag> = "Tag";
}