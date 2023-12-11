#pragma once

#include "Scene/Scene.h"
#include "Modules/Renderer/AssetLoader.h"
#include "Modules/Renderer/Renderer.h"
#include "Modules/Renderer/Model/Mesh.h"
#include "Modules/Renderer/Model/Material.h"
#include "Modules/Renderer/Model/Lights.h"
#include "Components/Camera.h"

namespace Nork {
	class RenderingSystem
	{
	public:
		static RenderingSystem& Instance();
		RenderingSystem();
		~RenderingSystem();
	public:
		std::shared_ptr<Renderer::DirLight> NewDirLight()
		{
			return std::make_shared<Renderer::DirLight>(Renderer::Resources::Instance().dirLights->New());
		}
		std::shared_ptr<Renderer::PointLight> NewPointLight()
		{
			return std::make_shared<Renderer::PointLight>(Renderer::Resources::Instance().pointLights->New());
		}
		void Update();
		void UpdateLights();
	public:
		glm::uvec2 resolution = { 1920, 1080 };
		entt::observer dirLightObserver;
		entt::observer pointLightObserver;
		entt::observer transformObserver;
	public:
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