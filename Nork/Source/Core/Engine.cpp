#include "Engine.h"

#include "Modules/Renderer/Loaders/Loaders.h"
#include "Modules/Renderer/Resource/DefaultResources.h"

namespace Nork
{
	static Data::Shader CreateShaderFromPath(std::string_view path)
	{
		std::ifstream stream(path.data());
		std::stringstream buf;
		buf << stream.rdbuf();

		auto data = Data::ShaderData{ .source = buf.str() };
		auto resource = Resource::CreateShader(data);
		stream.close();
		return Data::Shader(resource);
	}

	static DeferredData CreatePipelineResources()
	{
		GLuint skyboxTex = Renderer::Utils::Texture::CreateCube("Resources/Textures/skybox", ".jpg");

		return DeferredData (DeferredData::Shaders
			{
				.gPass = CreateShaderFromPath("Source/Shaders/gPass.shader"),
				.lPass = CreateShaderFromPath("Source/Shaders/lightPass.shader"),
				.skybox = CreateShaderFromPath("Source/Shaders/skybox.shader"),
			}, skyboxTex);
	}

	static std::vector<Data::Model> GetModels(ECS::Registry& r)
	{
		std::vector<std::pair<std::vector<Data::Mesh>, glm::mat4>> result;
		auto& reg = r.GetUnderlying();

		auto view = reg.view<Components::Model, Components::Transform>();
		result.reserve(view.size_hint());

		for (auto& id : view)
		{
			auto& model = view.get(id)._Myfirst._Val;
			auto& tr = view.get(id)._Get_rest()._Myfirst._Val;

			glm::mat4 rot = glm::rotate(glm::identity<glm::mat4>(), tr.rotation.z, glm::vec3(0, 0, 1));
			rot *= glm::rotate(glm::identity<glm::mat4>(), tr.rotation.x, glm::vec3(1, 0, 0));
			rot *= glm::rotate(glm::identity<glm::mat4>(), tr.rotation.y, glm::vec3(0, 1, 0)); // rotation around y-axis stays local (most used rotation usually)

			result.push_back(std::pair(model.meshes,
				glm::scale(glm::translate(glm::identity<glm::mat4>(), tr.position) * rot, tr.scale)));
		}

		return result;
	}

	Engine::Engine(EngineConfig& config)
		:window(config.width, config.height), pipeline(CreatePipelineResources()),
		camController(window.GetInput(), std::make_shared<Components::Camera>())
	{
		Logger::PushStream(std::cout);
		Renderer::Resource::DefaultResources::Init();
	}

	void Engine::Launch()
	{
		while (window.IsRunning())
		{
			appEventMan.RaiseEvent(Events::OnUpdate());
			appEventMan.RaiseEvent(Events::OnRenderUpdate());

			auto models = GetModels(this->scene.registry);
			ViewProjectionUpdate();
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
	void Engine::ViewProjectionUpdate()
	{
		camController.OnUpdate(1.0f);
		//lightMan.dShadowMapShader->SetMat4("VP", vp);
		pipeline.data.shaders.gPass.Use();
		pipeline.data.shaders.gPass.SetMat4("VP", camController.camera->viewProjection);

		pipeline.data.shaders.lPass.Use();
		pipeline.data.shaders.lPass.SetVec3("viewPos", camController.camera->position);

		pipeline.data.shaders.skybox.Use();
		auto vp = camController.camera->projection * glm::mat4(glm::mat3(camController.camera->view));
		pipeline.data.shaders.skybox.SetMat4("VP", vp);
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