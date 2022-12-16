#pragma once

#include "Scene/Scene.h"
#include "Core/ResourceManager.h"

namespace Nork {
	class RenderingSystem
	{
	public:
		static RenderingSystem& Instance();
		RenderingSystem(entt::registry& registry);
	private:
		// void ViewProjectionUpdate(Components::Camera& camera);

	private:
		glm::uvec2 resolution = { 1920, 1080 };
		entt::observer dirLightObserver;
		entt::observer pointLightObserver;
		entt::observer transformObserver;
	public:
		entt::registry& registry;
	private:
	};
}