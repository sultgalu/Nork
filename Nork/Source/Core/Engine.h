#pragma once
#include "Platform/Windows.h"
#include "Modules/Renderer/Pipeline/Deferred.h"
#include "Modules/Renderer/Pipeline/LightManager.h"
#include "Core/CameraController.h"
#include "Editor/Editor.h"
#include "Scene/Scene.h"

namespace Nork
{
	using namespace Renderer;
	using namespace Pipeline;

	struct Resources
	{
		std::unordered_map<std::string, std::vector<Renderer::Data::MeshResource>> models;
		std::unordered_map<std::string, Renderer::Data::ShaderResource> shaders;
		std::unordered_map<std::string, Renderer::Data::TextureResource> textures;
	};

	struct EngineConfig
	{
		EngineConfig() = default;

		uint32_t width = 1280, height = 720;
		inline EngineConfig& SetResolution(uint32_t w, uint32_t h) { width = w; height = h; return *this; }
	};

	class Engine
	{
	public:
		Engine(EngineConfig& config);
		~Engine();
		void Launch();
	private:
		void UpdateLights();
		void ViewProjectionUpdate();
		void FreeResources();
	public:
		Window window;
		Deferred pipeline;
		LightManager lightMan;
		CameraController camController;
		EventManager appEventMan;
		Resources resources;
		Scene::Scene scene;
	};

	extern Engine& GetEngine();

}