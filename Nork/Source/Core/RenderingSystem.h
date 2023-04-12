#pragma once

#include "Scene/Scene.h"
#include "Core/ResourceManager.h"
#include "Modules/Renderer/Renderer.h"
#include "Modules/Renderer/Model/Mesh.h"
#include "Modules/Renderer/Model/Material.h"
#include "Modules/Renderer/Model/Lights.h"

namespace Nork {
	class RenderingSystem
	{
	public:
		static RenderingSystem& Instance();
		RenderingSystem(entt::registry& registry);
		~RenderingSystem();
	public:
		std::shared_ptr<Renderer::Mesh> NewMesh(uint32_t vertexCount, uint32_t indexCount)
		{
			auto vertices = Renderer::Resources::Instance().vertexBuffer->New(vertexCount);
			auto indices = Renderer::Resources::Instance().indexBuffer->New(indexCount);
			return std::make_shared<Renderer::Mesh>(vertices, indices);
		}
		std::shared_ptr<Renderer::Mesh> NewMesh(const std::vector<Renderer::Data::Vertex>& vertices, 
			const std::vector<uint32_t> indices)
		{
			auto mesh = NewMesh(vertices.size(), indices.size());
			mesh->vertices->Write(vertices.data(), vertices.size());
			mesh->indices->Write(indices.data(), indices.size());
			return mesh;
		}
		std::shared_ptr<Renderer::Material> NewMaterial()
		{
			return std::make_shared<Renderer::Material>(Renderer::Resources::Instance().materials->New());
		}
		std::shared_ptr<Renderer::BufferElement<glm::mat4>> NewModelMatrix()
		{
			return Renderer::Resources::Instance().modelMatrices->New();
		}
		std::shared_ptr<Renderer::DirLight> NewDirLight()
		{
			return std::make_shared<Renderer::DirLight>(Renderer::Resources::Instance().dirLights->New());
		}
		std::shared_ptr<Renderer::PointLight> NewPointLight()
		{
			return std::make_shared<Renderer::PointLight>(Renderer::Resources::Instance().pointLights->New());
		}
		std::shared_ptr<Renderer::Image> LoadImage(const std::string& path);
		void Update();
		void UpdateLights();
	public:
		glm::uvec2 resolution = { 1920, 1080 };
		entt::observer dirLightObserver;
		entt::observer pointLightObserver;
		entt::observer transformObserver;
	public:
		entt::registry& registry;
		std::shared_ptr<Components::Camera> camera;
	private:
		void OnDrawableAdded(entt::registry& reg, entt::entity id);
		void OnDrawableRemoved(entt::registry& reg, entt::entity id);
		void OnDrawableUpdated(entt::registry& reg, entt::entity id);

		bool shouldUpdateDirLightAndShadows = false;
		bool shouldUpdatePointLightAndShadows = false;
		void UpdateDirLightShadows(); // updates UBO idxs and shadowMapProvider
		void UpdatePointLightShadows();

		void OnDShadRemoved(entt::registry& reg, entt::entity id);
		void OnPShadRemoved(entt::registry& reg, entt::entity id);
		void OnDLightRemoved(entt::registry& reg, entt::entity id);
		void OnPLightRemoved(entt::registry& reg, entt::entity id);

		void OnDLightAdded(entt::registry& reg, entt::entity id);
		void OnPLightAdded(entt::registry& reg, entt::entity id);
		void OnDShadAdded(entt::registry& reg, entt::entity id);
		void OnPShadAdded(entt::registry& reg, entt::entity id);
	};
}