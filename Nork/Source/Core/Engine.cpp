#include "Engine.h"

#include "Modules/Renderer/Loaders/Loaders.h"
#include "Modules/Renderer/Resource/DefaultResources.h"

namespace Nork
{
	Data::Shader CreateShaderFromPath(std::string_view path)
	{
		std::ifstream stream(path.data());
		std::stringstream buf;
		buf << stream.rdbuf();

		auto data = Data::ShaderData{ .source = buf.str() };
		auto resource = Resource::CreateShader(data);
		stream.close();
		return Data::Shader(resource);
	}

	DeferredData CreatePipelineResources()
	{
		GLuint skyboxTex = Renderer::Utils::Texture::CreateCube("Resources/Textures/skybox", ".jpg");

		return DeferredData (DeferredData::Shaders
			{
				.gPass = CreateShaderFromPath("Source/Shaders/gPass.shader"),
				.lPass = CreateShaderFromPath("Source/Shaders/lightPass.shader"),
				.skybox = CreateShaderFromPath("Source/Shaders/skybox.shader"),
			}, skyboxTex);
	}

	Engine::Engine(EngineConfig& config)
		:window(config.width, config.height), pipeline(CreatePipelineResources()),
		camController(window.GetInput(), std::make_shared<Components::Camera>())
	{
		Renderer::Resource::DefaultResources::Init();
		GLuint skyboxTex = Renderer::Utils::Texture::CreateCube("Resources/Textures/skybox", ".jpg");
		std::string modelPath = "Resources/Models/lamp/untitled.obj";
		auto model = Renderer::Loaders::LoadModel(modelPath);
		std::vector<Renderer::Data::Mesh> meshes;
		for (size_t i = 0; i < model.size(); i++)
		{
			resources.models[modelPath].push_back(Renderer::Resource::CreateMesh(model[i]));
			meshes.push_back(Renderer::Data::Mesh(resources.models[modelPath][i]));
		}

		models.push_back(Renderer::Data::Model(meshes, glm::identity<glm::mat4>()));

		appEventMan.Subscribe<Events::OnUpdate>([&](const Event& e)
			{
				using namespace Input;
				camController.OnUpdate(1.0f);

				static Components::Transform tr;

				static constinit float  speed = 0.005;
				if (window.GetInput().IsKeyDown(Key::Up))
				{
					tr.position.y += speed;
				}
				if (window.GetInput().IsKeyDown(Key::Down))
				{
					tr.position.y -= speed;
				}
				if (window.GetInput().IsKeyDown(Key::Right))
				{
					tr.position.x += speed;
				}
				if (window.GetInput().IsKeyDown(Key::Left))
				{
					tr.position.x -= speed;
				}
				models[0].second = glm::translate(glm::identity<glm::mat4>(), tr.position);
				//lightMan.dShadowMapShader->SetMat4("VP", vp);
				pipeline.data.shaders.gPass.Use();
				pipeline.data.shaders.gPass.SetMat4("VP", camController.camera->viewProjection);

				pipeline.data.shaders.lPass.Use();
				pipeline.data.shaders.lPass.SetVec3("viewPos", camController.camera->position);

				pipeline.data.shaders.skybox.Use();
				auto vp = camController.camera->projection * glm::mat4(glm::mat3(camController.camera->view));
				pipeline.data.shaders.skybox.SetMat4("VP", vp);
			});

		window.GetInput().GetEventManager().Subscribe<Events::KeyDown>([&](const Event& ev)
			{
				if (ev.As<Events::KeyDown>().key == Input::Key::Esc)
				{
					for (auto& model : this->resources.models)
					{
						auto& meshes = model.second;
						for (size_t i = 0; i < meshes.size(); i++)
						{
							Renderer::Resource::DeleteMesh(meshes[i]);
						}
					}
				}
			});

		Logger::PushStream(std::cout);
	}
	
	void Engine::Launch()
	{
		while (window.IsRunning())
		{
			appEventMan.RaiseEvent(Events::OnUpdate());
			appEventMan.RaiseEvent(Events::OnRenderUpdate());
			pipeline.DrawScene(models);
			appEventMan.RaiseEvent(Events::RenderUpdated());
			window.Refresh();
			appEventMan.PollEvents();
			appEventMan.RaiseEvent(Events::Updated());
		}
	}
	Engine::~Engine()
	{
		FreeResources();
	}
	void Engine::FreeResources()
	{
		for (auto& model : resources.models)
		{
			auto& meshes = model.second;
			for (size_t i = 0; i < meshes.size(); i++)
			{
				Logger::Debug("Freeing MESH resource \"", model.first, "\" from Renderer");
				Renderer::Resource::DeleteMesh(meshes[i]);
			}
		}
		for (auto& shad : resources.shaders)
		{
			Logger::Debug("Freeing SHADER resource \"", shad.first, "\" from Renderer");
			Renderer::Resource::DeleteShader(shad.second);
		}
		for (auto& tex : resources.textures)
		{
			Logger::Debug("Freeing TEXTURE resource \"", tex.first, "\" from Renderer");
			Renderer::Resource::DeleteTexture(tex.second);
		}
	}
}