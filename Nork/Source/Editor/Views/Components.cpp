#include "include/Components.h"
#include "../Views/include/Helpers.h"
#include "Platform/FileDialog.h"
#include "Editor/Editor.h"
#include "Modules/Renderer/Data/Vertex.h"
#include "Core/PolygonBuilder.h"
#include "Editor/Utils/EditorImage.h"


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
		// dirLight.RecalcVP();
		// dirLight.light->Update();
		auto light = dirLight.Data();
		auto dir = light->direction;
		changed |= ImGui::Checkbox("Sun", &dirLight.sun);
		changed |= ImGui::SliderFloat3("Direction", &light->direction.x, -1, 1, "%.5f");
		changed |= ImGui::SliderFloat("Out Of Proj Value", &light->outOfProjValue, 0, 1);
		changed |= ImGui::ColorEdit3("Color (diffuse)", &(light->color2.r));
		changed |= ImGui::ColorEdit3("Color (ambient)", &(light->color.r));
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
			auto& shad = node->GetEntity().GetComponent<Components::DirShadowMap>();
			auto& shadowMap = shad.shadowMap; 
			auto shadow = shadowMap->shadow->Data();
			ImGui::Checkbox("Update##DirShadow", &shad.update);
			ImGui::DragFloat("Bias", (float*)&(shadow->bias), 0.001f);
			ImGui::DragFloat("min Bias", (float*)&(shadow->biasMin), 0.001f);

			static EditorImage img;
			auto& tex = shadowMap->image;
			img = tex;
			static auto imgSize = 100;
			ImGui::Image(img.descritptorSet, ImVec2(imgSize, imgSize));
			ImGui::DragInt("Image Size", &imgSize, 1, 50, 500);
			ImGui::Text("Width: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->img->Width()).c_str());
			ImGui::Text("Height: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->img->Height()).c_str());
			ImGui::Text("Format: "); ImGui::SameLine(); ImGui::Text(vk::to_string(tex->img->Format()).c_str());
			static bool fix = false;
			static glm::ivec2 newSize;
			if (ImGui::Button("Change Resolution##DirLi"))
			{
				ImGui::OpenPopup("chgrespshad##dir");
				newSize = { tex->img->Width() , tex->img->Height() };
			}
			if (ImGui::BeginPopup("chgrespshad##dir"))
			{
				ImGui::InputInt("New Width", &newSize.x);
				ImGui::InputInt("New Height", &newSize.y);
				if (ImGui::Button("OK"))
				{
					changed = true;
					shadowMap->CreateTexture(newSize.x, newSize.y);
					ImGui::CloseCurrentPopup();
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
		changed |= ImGui::ColorEdit4("Color##Plight", (float*)&(pointLight.Data()->color.r));

		int pow = pointLight.GetIntensity();
		if (ImGui::DragInt("Intensity", &(pow), 1, 0, 10000)) {
			changed = true;
			pointLight.SetIntensity(pow);
		}

		if (!node->GetEntity().HasComponent<Components::PointShadowMap>())
		{
			if (ImGui::Button("Add shadow"))
			{
				node->GetEntity().AddComponent<Components::PointShadowMap>();
			}
		}
		else if (ImGui::TreeNode("Shadow"))
		{
			auto& shad = node->GetEntity().GetComponent<Components::PointShadowMap>();
			auto& shadowMap = shad.shadowMap;
			auto& shadow = shadowMap->Shadow();
			ImGui::Checkbox("Update##PShadow", &shad.update);
			ImGui::DragFloat("Bias", (float*)&(shadow->bias), 0.0001f, 0, FLT_MAX, "%.4f");
			ImGui::DragFloat("min Bias", (float*)&(shadow->biasMin), 0.0001f, 0, FLT_MAX, "%.4f");
			ImGui::SliderFloat("Near", (float*)&(shadow->near), 0, 1);
			ImGui::SliderFloat("Far", (float*)&(shadow->far), 0, 1000, "%.1f", ImGuiSliderFlags_Logarithmic);

			auto& tex = shadowMap->image;
			ImGui::Text("Width: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->img->Width()).c_str());
			ImGui::Text("Height: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->img->Height()).c_str());
			ImGui::Text("Format: "); ImGui::SameLine(); ImGui::Text(vk::to_string(tex->img->Format()).c_str());
			static int newSize;
			if (ImGui::Button("Change Resolution"))
			{
				ImGui::OpenPopup("chgrespshad");
				newSize = tex->img->Width();
			}
			if (ImGui::BeginPopup("chgrespshad"))
			{
				ImGui::InputInt("New Size", &newSize);
				if (ImGui::Button("OK"))
				{
					changed = true;
					shadowMap->CreateTexture(newSize);
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
		const auto model = dr.GetModel();
		if (!model->nodes.empty())
		{
			if (ImGui::TreeNode("Meshes##1234")) {
				static int meshIdx = 0;
				auto meshCount = model->nodes.size();
				if (meshIdx >= meshCount)
					meshIdx = meshCount - 1;
				ImGui::SliderInt("Mesh of model", &meshIdx, 0, meshCount - 1);
				auto& node = model->nodes[meshIdx];

				ImGui::Unindent();
				if (node.localTransform.has_value())
				{
					glm::vec3 pos, scale, skew;
					glm::quat quat;
					glm::vec4 persp;
					glm::decompose(*node.localTransform, scale, quat, pos, skew, persp);
					// quat = glm::conjugate(quat);
					bool trChanged = false;

					trChanged |= ImGui::DragFloat3("Position", &pos.x, 0.001f);
					trChanged |= ImGui::DragFloat4("Rotation", &quat.x, 0.001f);
					trChanged |= ImGui::DragFloat3("Scale", &scale.x, 0.001f);
					changed |= trChanged;
					if (trChanged)
						node.localTransform = glm::scale(glm::translate(glm::identity<glm::mat4>(), pos) * glm::mat4_cast(glm::normalize(quat)), scale);
					if (ImGui::Button("Remove Local Transform")) {
						changed = true;
						node.localTransform.reset();
					}
				}
				else if (ImGui::Button("Add Local Transform")) {
					changed = true;
					node.localTransform.emplace(glm::identity<glm::mat4>());
				}

				if (ImGui::TreeNode("Materials##1234"))
				{
					ImGui::Unindent();
					static int subMeshIdx = 0;
					auto subMeshCount = node.mesh->subMeshes.size();
					if (subMeshIdx >= subMeshCount)
						subMeshIdx = subMeshCount - 1;
					ImGui::SliderInt("Mesh of model", &subMeshIdx, 0, subMeshCount - 1);
					auto& subMesh = node.mesh->subMeshes[subMeshIdx];

					auto material = subMesh.material->Data();
					ImGui::ColorEdit3("Base Color Factor", &material->baseColorFactor.r, ImGuiColorEditFlags_DefaultOptions_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
					ImGui::SliderFloat("Roughness Factor", &material->roughnessFactor, 0, 1);
					ImGui::SliderFloat("Metallic Factor", &material->metallicFactor, 0, 1);

					if (ImGui::BeginTabBar("MaterialTexturesTab"))
					{
						auto displayTex = [&](Renderer::TextureMap type)
						{
							static EditorImage img;

							auto tex = subMesh.material->GetTextureMap(type);
							img = tex->image;
							ImGui::Image(img.descritptorSet, ImVec2(imgSize, imgSize));
							ImGui::Text("Width: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->image->img->Width()).c_str());
							ImGui::Text("Height: "); ImGui::SameLine(); ImGui::Text(std::to_string(tex->image->img->Width()).c_str());
							ImGui::Text("Format: "); ImGui::SameLine(); ImGui::Text(vk::to_string(tex->image->img->Format()).c_str());

							ImGui::EndTabItem();
							if (ImGui::Button("Default"))
							{
								subMesh.material->SetDefaultTexture(type);
							}
							if (ImGui::Button("Load texture"))
							{
								std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::Image, L"Load Texture", L"Load");
								if (!p.empty())
								{
									auto uri = AssetLoader::Instance().AbsolutePathToUri(p);
									auto newTex = AssetLoader::Instance().LoadTexture(uri);
									if (newTex != nullptr)
									{
										subMesh.material->SetTextureMap(newTex, type);
									}
								}
							}
						};
						if (ImGui::BeginTabItem("Base Color"))
						{
							displayTex(Renderer::TextureMap::BaseColor);
						}
						if (ImGui::BeginTabItem("Normal"))
						{
							displayTex(Renderer::TextureMap::Normal);
						}
						if (ImGui::BeginTabItem("Metallic Roughness"))
						{
							displayTex(Renderer::TextureMap::MetallicRoughness);
						}
						ImGui::EndTabBar();
					}

					ImGui::DragInt("Image Size", &imgSize, 1, 50, 500);

					//auto meshUri = MeshResources::Instance().Uri(model->meshes[meshIdx].mesh).filename().string();
					//ImGui::Text("Mesh: "); ImGui::SameLine();
					//ImGui::Text(meshUri.c_str());

					// auto matSrc = MaterialResources::Instance().Id(dr.model->meshes[meshIdx].material);
					// ImGui::Text("Material: "); ImGui::SameLine();
					// ImGui::Text(matSrc.c_str());

					// if (ImGui::Button("Load Material"))
					// {
					// 	std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::glTF, L"Load Material", L"Load");
					// 	if (!p.empty())
					// 		dr.model->meshes[meshIdx].material = MaterialResources::Instance().GetByPath(p);
					// }
					ImGui::Indent();
					ImGui::TreePop();
				}

				ImGui::Indent();
				ImGui::TreePop();
			}
		}
		auto src = AssetLoader::Instance().Uri(model).filename().string();
		ImGui::Text(src.c_str());
		static char modelNamebuf[100] = { 0 };
		if (ImGui::Button("Save Model"))
		{
			AssetLoader::Instance().SaveModel(model);
		}
		if (ImGui::Button("Load Model"))
		{
			std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::glTF, L"Load model", L"Load");
			if (!p.empty()) {
				auto uri = AssetLoader::Instance().AbsolutePathToUri(p);
				dr.SetModel(AssetLoader::Instance().LoadModel(uri));
				changed = true;
			}
		}
		if (ImGui::Button("Reload Model")) {
			changed = true;
			AssetLoader::Instance().ReloadModel(dr.GetModel());
		}
		static fs::path selected;
		if (ImGui::Button("Switch"))
		{
			selected = "";
			ImGui::OpenPopup("SwitchPopup");
		}
		if (ImGui::BeginPopup("SwitchPopup"))
		{
			for (auto& uri : AssetLoader::Instance().ListLoadedModels())
			{
				if (ImGui::Selectable(uri.string().c_str(), selected == uri, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups))
					selected = uri;
			}
			if (ImGui::Button("Switch##Accept")) {
				changed = true;
				dr.SetModel(AssetLoader::Instance().LoadModel(selected));
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (ImGui::Button("Import Model"))
		{
			std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::_3D, L"Import model", L"Import");
			if (!p.empty())
			{
				changed = true;
				dr.SetModel(AssetLoader::Instance().ImportModel(p));
			}
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
		// if (ImGui::DragFloat3("Angular Velocity Axis", &axis.x))
		// {
		//	changed = true;
		// 	kin.w = axis * angle;
		// }
		// 
		// if (ImGui::DragFloat("Angular Velocity Speed", &angle, 0.001f))
		// {
		//	changed = true;
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