#include "include/Components.h"
#include "../Views/include/Helpers.h"
#include "Core/PolygonBuilder.h"
#include "Editor/Utils/EditorImage.h"
#include "Modules/Renderer/Data/Vertex.h"
#include "Platform/FileDialog.h"
#include "Scripting/ScriptSystem.h"

namespace Nork::Editor {

struct ChangeObserver {
    bool changed = false;
    bool operator()(bool val)
    {
        changed |= val;
        return val;
    }
};

SceneNodeView::SceneNodeView(std::shared_ptr<SceneNode> node)
    : node(node)
{
    // node->GetEntity().Id()
}

void SceneNodeView::Content()
{
    using namespace Components;
    Content<Transform, Drawable, DirLight, PointLight, Components::Physics, Camera, Tag, ScriptComponent>();
}
template <class... T>
void SceneNodeView::Content()
{
    EditComponents<T...>();
    ListComponentsForAddition<T...>();
}

template <>
bool SceneNodeView::ShowComponent(Components::DirLight& dirLight)
{
    ChangeObserver observer;
    // dirLight.RecalcVP();
    // dirLight.light->Update();
    auto light = dirLight.Data();
    auto dir = light->direction;
    observer(ImGui::Checkbox("Sun", &dirLight.sun));
    observer(ImGui::SliderFloat3("Direction", &light->direction.x, -1, 1, "%.5f"));
    observer(ImGui::SliderFloat("Out Of Proj Value", &light->outOfProjValue, 0, 1));
    observer(ImGui::ColorEdit3("Color (diffuse)", &(light->color2.r)));
    observer(ImGui::ColorEdit3("Color (ambient)", &(light->color.r)));
    observer(ImGui::DragFloat3("Position", &dirLight.position.x));
    observer(ImGui::DragFloat3("Rectangle", &dirLight.rectangle.x));

    if (!node->GetEntity().HasComponent<Components::DirShadowMap>()) {
        if (ImGui::Button("Add shadow")) {
            node->GetEntity().AddComponent<Components::DirShadowMap>();
        }
    } else if (ImGui::TreeNode("Shadow")) {
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
        ImGui::Text("Width: ");
        ImGui::SameLine();
        ImGui::Text(std::to_string(tex->img->Width()).c_str());
        ImGui::Text("Height: ");
        ImGui::SameLine();
        ImGui::Text(std::to_string(tex->img->Height()).c_str());
        ImGui::Text("Format: ");
        ImGui::SameLine();
        ImGui::Text(vk::to_string(tex->img->Format()).c_str());
        static bool fix = false;
        static glm::ivec2 newSize;
        if (ImGui::Button("Change Resolution##DirLi")) {
            ImGui::OpenPopup("chgrespshad##dir");
            newSize = { tex->img->Width(), tex->img->Height() };
        }
        if (ImGui::BeginPopup("chgrespshad##dir")) {
            ImGui::InputInt("New Width", &newSize.x);
            ImGui::InputInt("New Height", &newSize.y);
            if (observer(ImGui::Button("OK"))) {
                shadowMap->CreateTexture(newSize.x, newSize.y);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
        if (ImGui::Button("Remove Shadow Map##Dir")) {
            node->GetEntity().RemoveComponent<Components::DirShadowMap>();
        }
        ImGui::PopStyleColor();

        ImGui::TreePop();
    }
    return observer.changed;
}
template <>
bool SceneNodeView::ShowComponent(Components::PointLight& pointLight)
{
    ChangeObserver observer;
    observer(ImGui::ColorEdit4("Color##Plight", (float*)&(pointLight.Data()->color.r)));

    int pow = pointLight.GetIntensity();
    if (observer(ImGui::DragInt("Intensity", &(pow), 1, 0, 10000))) {
        pointLight.SetIntensity(pow);
    }

    if (!node->GetEntity().HasComponent<Components::PointShadowMap>()) {
        if (ImGui::Button("Add shadow")) {
            node->GetEntity().AddComponent<Components::PointShadowMap>();
        }
    } else if (ImGui::TreeNode("Shadow")) {
        auto& shad = node->GetEntity().GetComponent<Components::PointShadowMap>();
        auto& shadowMap = shad.shadowMap;
        auto& shadow = shadowMap->Shadow();
        ImGui::Checkbox("Update##PShadow", &shad.update);
        ImGui::DragFloat("Bias", (float*)&(shadow->bias), 0.0001f, 0, FLT_MAX, "%.4f");
        ImGui::DragFloat("min Bias", (float*)&(shadow->biasMin), 0.0001f, 0, FLT_MAX, "%.4f");
        ImGui::SliderFloat("Near", (float*)&(shadow->near), 0, 1);
        ImGui::SliderFloat("Far", (float*)&(shadow->far), 0, 1000, "%.1f", ImGuiSliderFlags_Logarithmic);

        auto& tex = shadowMap->image;
        ImGui::Text("Width: ");
        ImGui::SameLine();
        ImGui::Text(std::to_string(tex->img->Width()).c_str());
        ImGui::Text("Height: ");
        ImGui::SameLine();
        ImGui::Text(std::to_string(tex->img->Height()).c_str());
        ImGui::Text("Format: ");
        ImGui::SameLine();
        ImGui::Text(vk::to_string(tex->img->Format()).c_str());
        static int newSize;
        if (ImGui::Button("Change Resolution")) {
            ImGui::OpenPopup("chgrespshad");
            newSize = tex->img->Width();
        }
        if (ImGui::BeginPopup("chgrespshad")) {
            ImGui::InputInt("New Size", &newSize);
            if (observer(ImGui::Button("OK"))) {
                shadowMap->CreateTexture(newSize);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
        if (ImGui::Button("Remove Shadow")) {
            node->GetEntity().RemoveComponent<Components::PointShadowMap>();
        }
        ImGui::PopStyleColor();
        ImGui::TreePop();
    }
    return observer.changed;
}
template <>
bool SceneNodeView::ShowComponent(Components::Transform& tr)
{
    ChangeObserver observer;
    ImGui::Text("Pos: %f;%f;%f", tr.Position().x, tr.Position().y, tr.Position().z);
    ImGui::Text("Scale: %f;%f;%f", tr.Scale().x, tr.Scale().y, tr.Scale().z);
    ImGui::Text("Quaternion: %f;%f;%f;%f", tr.Quaternion().w, tr.Quaternion().x, tr.Quaternion().y, tr.Quaternion().z);

    ImGui::Separator();
    observer(ImGui::DragFloat3("Position", &tr.localPosition.x, 0.1f));
    observer(ImGui::DragFloat3("Scale", &tr.localScale.x, 0.1f, 0.1f, 1000.0f, "%.3f"));
    ImGui::Separator();

    glm::vec3 rotateAmount = { 0, 0, 0 };
    if (observer(ImGui::DragFloat3("Rotate Locally", &rotateAmount.x, 0.01f))) {
        tr.Rotate(glm::vec3(1, 0, 0), rotateAmount.x);
        tr.Rotate(glm::vec3(0, 1, 0), rotateAmount.y);
        tr.Rotate(glm::vec3(0, 0, 1), rotateAmount.z);
    }
    glm::vec3 rotateAmount2 = { 0, 0, 0 };
    if (observer(ImGui::DragFloat3("Rotate Globally", &rotateAmount.x, 0.01f))) {
        glm::quat rot(1, rotateAmount);
        tr.localQuaternion = glm::normalize(rot) * tr.localQuaternion;
    }
    ImGui::Separator();
    glm::vec3 rotationAxis = tr.RotationAxis();
    float angle = tr.RotationAngleDegrees();
    if (observer(ImGui::DragFloat("Angle", &angle, 0.1f))) {
        tr.SetRotation(glm::normalize(rotationAxis), glm::radians(angle));
    }
    if (observer(ImGui::DragFloat3("Set Rotation Axis", &rotationAxis.x, 0.01f))) {
        tr.SetRotation(glm::normalize(rotationAxis), glm::radians(angle));
    }
    if (observer(ImGui::DragFloat4("Quaternion", &tr.localQuaternion.x, 0.1f))) {
        tr.localQuaternion = glm::normalize(tr.localQuaternion);
    }
    ImGui::Separator();
    if (observer(ImGui::Button("Reset Rotation"))) {
        tr.localQuaternion = glm::qua(1.0f, glm::vec3(0));
    }
    return observer.changed;
}
template <>
bool SceneNodeView::ShowComponent(Components::Camera& cam)
{
    ChangeObserver observer;
    observer(ImGui::DragFloat3("near#camera", &cam.nearClip));
    observer(ImGui::DragFloat3("far#camera", &cam.farClip));
    observer(ImGui::DragFloat3("pitch", &cam.pitch));
    observer(ImGui::DragFloat3("yaw", &cam.yaw));
    observer(ImGui::DragFloat("FOV", &cam.FOV));
    observer(ImGui::DragFloat("ratio", &cam.ratio));
    observer(ImGui::DragFloat3("position#camera", &cam.position.x));
    return observer.changed;
}
template <>
bool SceneNodeView::ShowComponent(Components::Drawable& dr)
{
    ChangeObserver observer;
    static int imgSize = 100;
    const auto model = dr.GetModel();
    if (!model->nodes.empty()) {
        if (ImGui::TreeNode("Meshes##1234")) {
            static int meshIdx = 0;
            auto meshCount = model->nodes.size();
            if (meshIdx >= meshCount)
                meshIdx = meshCount - 1;
            ImGui::SliderInt("Mesh of model", &meshIdx, 0, meshCount - 1);
            auto& node = model->nodes[meshIdx];

            ImGui::Unindent();
            if (node.localTransform.has_value()) {
                glm::vec3 pos, scale, skew;
                glm::quat quat;
                glm::vec4 persp;
                glm::decompose(*node.localTransform, scale, quat, pos, skew, persp);
                // quat = glm::conjugate(quat);
                bool trChanged = false;

                trChanged |= ImGui::DragFloat3("Position", &pos.x, 0.001f);
                trChanged |= ImGui::DragFloat4("Rotation", &quat.x, 0.001f);
                trChanged |= ImGui::DragFloat3("Scale", &scale.x, 0.001f);
                observer(trChanged);
                if (trChanged)
                    node.localTransform = glm::scale(glm::translate(glm::identity<glm::mat4>(), pos) * glm::mat4_cast(glm::normalize(quat)), scale);
                if (observer(ImGui::Button("Remove Local Transform"))) {
                    node.localTransform.reset();
                }
            } else if (observer(ImGui::Button("Add Local Transform"))) {
                node.localTransform.emplace(glm::identity<glm::mat4>());
            }

            if (ImGui::TreeNode("Materials##1234")) {
                ImGui::Unindent();
                static int subMeshIdx = 0;
                auto subMeshCount = node.mesh->primitives.size();
                if (subMeshIdx >= subMeshCount)
                    subMeshIdx = subMeshCount - 1;
                ImGui::SliderInt("Mesh of model", &subMeshIdx, 0, subMeshCount - 1);
                auto& primitive = node.mesh->primitives[subMeshIdx];

                auto material = primitive.material->Data();
                bool blend = primitive.shadingMode == Renderer::ShadingMode::Blend;
                if (ImGui::Checkbox("Blend", &blend)) {
                    primitive.shadingMode = blend ? Renderer::ShadingMode::Blend : Renderer::ShadingMode::Default;
                }
                ImGui::SameLine();
                bool unlit = primitive.shadingMode == Renderer::ShadingMode::Unlit;
                if (ImGui::Checkbox("Unlit", &unlit)) {
                    primitive.shadingMode = unlit ? Renderer::ShadingMode::Unlit : Renderer::ShadingMode::Default;
                }
                ImGui::ColorEdit4("Base Color Factor", &material->baseColorFactor.r, ImGuiColorEditFlags_DefaultOptions_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
                ImGui::SliderFloat("Roughness Factor", &material->roughnessFactor, 0, 1);
                ImGui::SliderFloat("Metallic Factor", &material->metallicFactor, 0, 1);
                ImGui::ColorEdit3("Emissive Factor", &material->emissiveFactor.r, ImGuiColorEditFlags_DefaultOptions_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
                float emissiveMultiplier = 1.0;
                if (ImGui::DragFloat("Emissive Multiplier", &emissiveMultiplier, 0.01f, 0.1f, 1.1f)) {
                    material->emissiveFactor *= emissiveMultiplier;
                }

                if (ImGui::BeginTabBar("MaterialTexturesTab")) {
                    auto displayTex = [&](Renderer::TextureMap type) {
                        static EditorImage img;

                        auto tex = primitive.material->GetTextureMap(type);
                        img = tex->image;
                        ImGui::Image(img.descritptorSet, ImVec2(imgSize, imgSize));
                        ImGui::Text("Width: ");
                        ImGui::SameLine();
                        ImGui::Text(std::to_string(tex->image->img->Width()).c_str());
                        ImGui::Text("Height: ");
                        ImGui::SameLine();
                        ImGui::Text(std::to_string(tex->image->img->Width()).c_str());
                        ImGui::Text("Format: ");
                        ImGui::SameLine();
                        ImGui::Text(vk::to_string(tex->image->img->Format()).c_str());

                        ImGui::EndTabItem();
                        if (ImGui::Button("Default")) {
                            primitive.material->SetDefaultTexture(type);
                        }
                        if (ImGui::Button("Load texture")) {
                            std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::Image, L"Load Texture", L"Load");
                            if (!p.empty()) {
                                auto uri = AssetLoader::Instance().AbsolutePathToUri(p);
                                auto newTex = AssetLoader::Instance().LoadTexture(uri);
                                if (newTex != nullptr) {
                                    primitive.material->SetTextureMap(newTex, type);
                                }
                            }
                        }
                    };
                    if (ImGui::BeginTabItem("Base Color")) {
                        displayTex(Renderer::TextureMap::BaseColor);
                    }
                    if (ImGui::BeginTabItem("Normal")) {
                        displayTex(Renderer::TextureMap::Normal);
                    }
                    if (ImGui::BeginTabItem("Metallic Roughness")) {
                        displayTex(Renderer::TextureMap::MetallicRoughness);
                    }
                    if (ImGui::BeginTabItem("Occusion")) {
                        displayTex(Renderer::TextureMap::Occlusion);
                    }
                    if (ImGui::BeginTabItem("Emissive")) {
                        displayTex(Renderer::TextureMap::Emissive);
                    }
                    ImGui::EndTabBar();
                }

                ImGui::DragInt("Image Size", &imgSize, 1, 50, 500);

                // auto meshUri = MeshResources::Instance().Uri(model->meshes[meshIdx].mesh).filename().string();
                // ImGui::Text("Mesh: "); ImGui::SameLine();
                // ImGui::Text(meshUri.c_str());

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
    if (ImGui::Button("Save##Model")) {
        AssetLoader::Instance().SaveModel(model);
    }
    if (ImGui::Button("Load##Model")) {
        std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::glTF, L"Load model", L"Load");
        if (observer(!p.empty())) {
            auto uri = AssetLoader::Instance().AbsolutePathToUri(p);
            dr.SetModel(AssetLoader::Instance().LoadModel(uri));
        }
    }
    if (ImGui::Button("Delete From Cache##Model")) {
        AssetLoader::Instance().DeleteFromCache(dr.GetModel());
    }
    if (observer(ImGui::Button("Reload##Model"))) {
        AssetLoader::Instance().ReloadModel(dr.GetModel());
    }
    static fs::path selected;
    if (ImGui::Button("Switch")) {
        selected = "";
        ImGui::OpenPopup("SwitchPopup");
    }
    if (ImGui::BeginPopup("SwitchPopup")) {
        for (auto& uri : AssetLoader::Instance().ListLoadedModels()) {
            if (ImGui::Selectable(uri.string().c_str(), selected == uri, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups))
                selected = uri;
        }
        if (observer(ImGui::Button("Switch##Accept"))) {
            dr.SetModel(AssetLoader::Instance().LoadModel(selected));
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (ImGui::Button("Import##Model")) {
        std::string p = FileDialog::OpenFile(FileDialog::EngineFileTypes::_3D, L"Import model", L"Import");
        if (observer(!p.empty())) {
            dr.SetModel(AssetLoader::Instance().ImportModel(p));
        }
    }
    return observer.changed;
}
template <>
bool SceneNodeView::ShowComponent(Components::Physics& phx)
{
    ChangeObserver observer;
    ImGui::Text(std::to_string(phx.Object().volume).append(" | Volume").c_str());
    {
        std::stringstream centerSs;
        auto& center = phx.Object().centerOfMass;
        centerSs << center.x << ";" << center.y << ";" << center.z << " | Center Of Mass";
        ImGui::Text(centerSs.str().c_str());
    }
    auto& kin = phx.handle.Get().kinem;
    observer(ImGui::DragFloat("Mass (kg)", &kin.mass, 0.01f));
    observer(ImGui::Checkbox("Auto Calculate Mass", &phx.Object().autoMass));
    if (phx.Object().autoMass)
        observer(ImGui::DragFloat("Mass Density", &phx.Object().massDensity, 0.01f));
    observer(ImGui::DragFloat("I", &kin.I, 0.01f));
    observer(ImGui::DragFloat("Elasticity", &kin.elasticity, 0.01f, 0, 2));
    observer(ImGui::DragFloat("Friction", &kin.friction, 0.01f, 0, 2));
    observer(ImGui::Checkbox("Gravity", &kin.applyGravity));
    observer(ImGui::Checkbox("Static", &kin.isStatic));
    observer(ImGui::DragFloat3("Velocity", &kin.velocity.x, 0.001f));
    auto axis = glm::normalize(kin.w);
    auto angle = glm::length(kin.w);
    observer(ImGui::DragFloat3("Angular Velocity Axis", &kin.w.x, 0.001f));
    if (ImGui::TreeNodeEx("Colliders", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)) {
        bool collChanged = false;
        int idx = 0;
        int idxToRemove = -1;
        for (auto& collNode : phx.Colliders()) {
            ImGui::Text(std::to_string(idx).c_str());
            ImGui::Text(std::to_string(collNode.local.volume).append(" | Volume").c_str());
            {
                std::stringstream centerSs;
                auto& center = collNode.local.center;
                centerSs << center.x << ";" << center.y << ";" << center.z << " | center";
                ImGui::Text(centerSs.str().c_str());
            }
            glm::vec3 scale = glm::vec3(1);
            if (ImGui::DragFloat3(("scale##collider" + std::to_string(idx)).c_str(), &scale.x, 0.01f, 0.01f)) {
                collChanged = true;
                for (auto& vert : collNode.local.verts) {
                    vert *= scale;
                }
            }
            collChanged |= ImGui::DragFloat3(("offset##collider" + std::to_string(idx)).c_str(), &collNode.offset.x, 0.01f);
            ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));
            if (ImGui::Button("Remove")) {
                collChanged = true;
                idxToRemove = idx;
            }
            ImGui::PopStyleColor();
            ImGui::Separator();
            idx++;
        }
        if (ImGui::Button("Add Collider")) {
            collChanged = true;
            phx.Colliders().push_back(Physics::ColliderNode { .local = Physics::Collider::Cube() });
        }
        if (idxToRemove != -1) {
            phx.Colliders().erase(phx.Colliders().begin() + idxToRemove);
        }
        if (collChanged) {
            phx.Object().OnLocalColliderChanged();
        }
        observer(collChanged);
        ImGui::TreePop();
    }
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
    return observer.changed;
}
template <>
bool SceneNodeView::ShowComponent(Components::Tag& tag)
{
    return Helpers::TextEditable(tag.tag);
}
template <>
bool SceneNodeView::ShowComponent(ScriptComponent& comp)
{
    ChangeObserver observer;
    static std::string selected;
    if (ImGui::Button("Select")) {
        if (ScriptSystem::Instance().scriptFactory) {
            selected = comp.script ? comp.script->Id() : "";
            ImGui::OpenPopup("SelectScriptPopup");
        }
    }
    if (ImGui::BeginPopup("SelectScriptPopup")) {
        for (auto& id : ScriptSystem::Instance().scriptFactory->Ids()) {
            if (observer(ImGui::Selectable(id.c_str(), selected == id))) {
                comp.script = ScriptSystem::Instance().scriptFactory->Create(node, id);
            }
        }
        ImGui::EndPopup();
    }
    return observer.changed;
}

template <class T>
bool _EditComponent(const T& comp, Entity& ent,
    std::function<bool(T& comp)> fun)
{
    auto copy = comp;
    bool changed = fun(copy);
    if (changed)
        ent.ReplaceComponent(copy);
    return changed;
}

template <class T>
std::string componentName;

template <class T>
bool SceneNodeView::EditComponent()
{
    auto& ent = node->GetEntity();
    if (ent.HasComponent<T>()) {
        auto treeFlags = ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_ClipLabelForTrailingButton;
        if constexpr (std::is_same<T, Components::Transform>::value) {
            treeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
        }
        bool opened = ImGui::TreeNodeEx((componentName<T> + "##" + std::to_string((uint64_t)ent.Id())).c_str(), treeFlags);

        auto w = ImGui::CalcItemWidth();
        auto align = ImGui::GetCurrentWindow()->Size.x - 20.f;
        ImGui::SameLine(align > w ? align : w);
        ImGui::PushStyleColor(0, ImVec4(0.5f, 0, 0, 1));

        if (ImGui::Button(("X##Delete" + componentName<T>).c_str())) {
            ent.RemoveComponent<T>();
            if (opened) {
                ImGui::TreePop();
                opened = false;
            }
        }
        ImGui::PopStyleColor();

        if (opened) {
            std::function<bool(T & comp)> fun = [&](T& copy) {
                return ShowComponent(copy);
            };
            bool edited = _EditComponent(ent.GetComponent<T>(), ent, fun);
            ImGui::TreePop();
            return edited;
        }
    }
    return false;
}

template <class T, class... Rest>
void SceneNodeView::_EditComponents()
{
    EditComponent<T>();
    if constexpr (sizeof...(Rest) > 0) {
        _EditComponents<Rest...>();
    }
}
template <class... T>
void SceneNodeView::EditComponents()
{
    _EditComponents<T...>();
}
template <class T, class... Rest>
void _ListComponentsForAddition(Entity& ent)
{
    if (ImGui::Selectable(componentName<T>.c_str(), false, ent.HasComponent<T>() ? ImGuiSelectableFlags_Disabled : 0)) {
        ent.AddComponent<T>();
        ImGui::CloseCurrentPopup();
    }
    if constexpr (sizeof...(Rest) > 0) {
        _ListComponentsForAddition<Rest...>(ent);
    }
}

template <class... T>
void SceneNodeView::ListComponentsForAddition()
{
    if (ImGui::Button("Add component")) {
        ImGui::OpenPopup("components");
    }
    if (ImGui::BeginPopup("components")) {
        _ListComponentsForAddition<T...>(node->GetEntity());
        ImGui::EndPopup();
    }
}

template <>
std::string componentName<ScriptComponent> = "Script";
template <>
std::string componentName<Components::DirLight> = "Directional Light";
template <>
std::string componentName<Components::PointLight> = "Point Light";
template <>
std::string componentName<Components::Transform> = "Transform";
template <>
std::string componentName<Components::Camera> = "Camera";
template <>
std::string componentName<Components::Drawable> = "Drawable";
template <>
std::string componentName<Components::Physics> = "Physics";
template <>
std::string componentName<Components::Tag> = "Tag";
}