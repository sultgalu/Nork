#pragma once

#include "Modules/Renderer/Objects/Shader/Shader.h"
#include "Modules/Renderer/Model/Model.h"
#include "Modules/Renderer/Pipeline/LightStateSynchronizer.h"
#include "Modules/Renderer/Pipeline/ShadowMapRenderer.h"
#include "Modules/Renderer/Pipeline/Deferred/DeferredPipeline.h"
#include "Components/All.h"

namespace Nork {
	
	class Shaders
	{
	private:
		static std::string GetFileContent(std::string path)
		{
			std::ifstream ifs(path);
			std::stringstream ss;
			ss << ifs.rdbuf();
			return ss.str();
		}
		static Renderer::ShaderType GetTypeByString(std::string_view str)
		{
			using enum Renderer::ShaderType;
			if (str._Equal("vertex")) [[likely]]
				return Vertex;
			else if (str._Equal("fragment"))
				return Fragment;
			else if (str._Equal("geometry"))
				return Geometry;
			else if (str._Equal("compute"))
				return Compute;

			Logger::Error("ERR:: Unkown shader type ", str);
			std::abort(); // :(
		}
		static std::unordered_map<Renderer::ShaderType, std::string> SplitShaders(std::string content)
		{
			const char* label = "#type";
			size_t labelLen = strlen(label);

			std::unordered_map<Renderer::ShaderType, std::string> shaderSrcs;

			size_t pos = content.find(label, 0);
			Renderer::ShaderType shadType;
			do
			{ // the file must start with #type
				size_t start = pos;
				size_t eol = content.find_first_of("\r\n", pos); // idx of the end of the "#type" line

				pos += labelLen + 1; // the first idx of "vertex"
				std::string type = content.substr(pos, eol - pos); // must be "#type vertex" with whitespaces exactly like that.
				shadType = GetTypeByString(type);

				//size_t nextLinePos = s.find_first_not_of("\r\n", eol); // filtering out spaces
				pos = content.find(label, eol); // next idx starting with #type
				if (pos == std::string::npos) // no more shaders
				{
					shaderSrcs[shadType] = content.substr(eol);
					break;
				}
				shaderSrcs[shadType] = content.substr(eol, pos - eol); // more shaders ahead

			} while (true);
			return shaderSrcs;
		}
		static void InitShaderFromSource(Renderer::Shader& shader, std::string path)
		{
			shader.Create().Compile(SplitShaders(GetFileContent(path)));
		}
	public:
		static void Init()
		{
			InitShaderFromSource(gPassShader, "Source/Shaders/gPass.shader");
			InitShaderFromSource(lPassShader, "Source/Shaders/lightPass.shader");
			InitShaderFromSource(dShadowShader, "Source/Shaders/dirShadMap.shader");
			InitShaderFromSource(pShadowShader, "Source/Shaders/pointShadMap.shader");
			InitShaderFromSource(skyboxShader, "Source/Shaders/skybox.shader");
			InitShaderFromSource(pointShader, "Source/Shaders/point.shader");
			InitShaderFromSource(lineShader, "Source/Shaders/line.shader");
			InitShaderFromSource(textureShader, "Source/Shaders/texture.shader");

			lPassShader.Use()
				.SetInt("gPos", 0)
				.SetInt("gDiff", 1)
				.SetInt("gNorm", 2)
				.SetInt("gSpec", 3);

			for (int i = 0; i < 5; i++)
				lPassShader.SetInt("dirShadowMaps[" + std::to_string(i) + "]", i + 10);
			for (int i = 0; i < 5; i++)
				lPassShader.SetInt("pointShadowMaps[" + std::to_string(i) + "]", i + 15);

			using enum Renderer::TextureMapType;
			gPassShader.Use()
				.SetInt("materialTex.diffuse", (int)Diffuse)
				.SetInt("materialTex.normals", (int)Normal)
				.SetInt("materialTex.roughness", (int)Roughness)
				.SetInt("materialTex.reflect", (int)Reflection);
		}
		inline static Renderer::Shader gPassShader, lPassShader,
			dShadowShader, pShadowShader,
			skyboxShader, textureShader,
			pointShader, lineShader;
	};

	class RenderingSystem
	{
	public:
		static std::vector<Renderer::Model> GetModels(entt::registry& reg)
		{
			std::vector<Renderer::Model> result;

			auto view = reg.view<Components::Model, Components::Transform>();
			result.reserve(view.size_hint());

			for (auto& id : view)
			{
				auto& model = view.get(id)._Myfirst._Val;
				auto& tr = view.get(id)._Get_rest()._Myfirst._Val;

				result.push_back(Renderer::Model{ .meshes = model.meshes, .modelMatrix = tr.GetModelMatrix() });
			}

			return result;
		}
		RenderingSystem& Init()
		{
			using namespace Renderer;
			Shaders::Init();

			lightStateSyncher.Initialize();
			for (auto& fb : pShadowFramebuffers)
			{
				auto depth = TextureCube().Create().Bind().SetParams().SetData(TextureAttributes{ .width = 1000, .height = 1000, .format = TextureFormat::Depth16 });
				fb.Create().Bind().SetAttachments(FramebufferAttachments().Depth(depth));
			}
			for (auto& fb : dShadowFramebuffers)
			{
				auto depth = Texture2D().Create().Bind().SetParams(TextureParams::FramebufferTex2DParams()).SetData(TextureAttributes{ .width = 4000, .height = 4000, .format = TextureFormat::Depth16 });
				fb.Create().Bind().SetAttachments(FramebufferAttachments().Depth(depth));
			}
			gFb.Create().Bind();
			using enum TextureFormat;
			gFb.CreateTextures(resolution.x, resolution.y, Depth16, RGB16F, RGB16F, RGB16F, RGBA16F);
			lFb.Create().Bind();
			lFb.CreateTextures(gFb, RGBA16F);
			return *this;
		}
		void UpdateLights(entt::registry& reg)
		{
			auto dLightsWS = reg.view<Components::DirLight, Components::DirShadow>();
			auto pLightsWS = reg.view<Components::PointLight, Components::PointShadow>();
			auto dLights = reg.view<Components::DirLight>(entt::exclude<Components::DirShadow>);
			auto pLights = reg.view<Components::PointLight>(entt::exclude<Components::PointShadow>);
			
			auto modelView = reg.view<Components::Model, Components::Transform>();
			std::vector<Renderer::Model> models;
			for (auto& id : modelView)
			{
				models.push_back(Renderer::Model {.meshes = modelView.get(id)._Myfirst._Val.meshes, .modelMatrix = modelView.get(id)._Get_rest()._Myfirst._Val.GetModelMatrix() });
			}
			// ---------------------------
			auto& lights = lightStateSyncher.GetLightState();
			lights.ClearAll();

			for (auto& id : dLightsWS)
			{
				const auto& light = dLightsWS.get(id)._Myfirst._Val.GetData();
				const auto& shadow = dLightsWS.get(id)._Get_rest()._Myfirst._Val.GetData();
				lights.dirLights.push_back(light);
				lights.dirShadows.push_back(shadow);

				Renderer::ShadowMapRenderer::RenderDirLightShadowMap(light, shadow, models, dShadowFramebuffers[shadow.idx], Shaders::dShadowShader);
				Renderer::ShadowMapRenderer::BindDirShadowMap(shadow, dShadowFramebuffers[shadow.idx]);
			}
			for (auto& id : pLightsWS)
			{
				auto& light = pLightsWS.get(id)._Myfirst._Val.GetData();
				auto& shadow = pLightsWS.get(id)._Get_rest()._Myfirst._Val.GetData();

				lights.pointLights.push_back(light);
				lights.pointShadows.push_back(shadow);

				Renderer::ShadowMapRenderer::RenderPointLightShadowMap(light, shadow, models, pShadowFramebuffers[shadow.idx], Shaders::pShadowShader);
				Renderer::ShadowMapRenderer::BindPointShadowMap(shadow, pShadowFramebuffers[shadow.idx]);
			}
			for (auto& id : dLights)
			{
				lights.dirLights.push_back(dLights.get(id)._Myfirst._Val.GetData());
			}
			for (auto& id : pLights)
			{
				lights.pointLights.push_back(pLights.get(id)._Myfirst._Val.GetData());
			}
			lightStateSyncher.Synchronize();
		}
		void ViewProjectionUpdate(Components::Camera& camera)
		{
			// shader.Use();
			// shader.SetMat4("VP", camera.viewProjection);
			// shader.SetVec4("colorDefault", triangleColor);
			// shader.SetVec4("colorSelected", glm::vec4(selectedColor, triAlpha));

			Shaders::pointShader.Use();
			Shaders::pointShader.SetMat4("VP", camera.viewProjection);
			Shaders::pointShader.SetFloat("aa", pointAA);
			Shaders::pointShader.SetFloat("size", pointInternalSize);
			Shaders::pointShader.SetVec4("colorDefault", pointColor);
			Shaders::pointShader.SetVec4("colorSelected", glm::vec4(selectedColor, pointAlpha));

			Shaders::lineShader.Use();
			Shaders::lineShader.SetMat4("VP", camera.viewProjection);
			Shaders::lineShader.SetFloat("width", lineWidth);
			Shaders::lineShader.SetVec4("colorDefault", lineColor);
			Shaders::lineShader.SetVec4("colorSelected", glm::vec4(selectedColor, lineAlpha));

			//lightMan.dShadowMapShader->SetMat4("VP", vp);
			Shaders::gPassShader.Use();
			Shaders::gPassShader.SetMat4("VP", camera.viewProjection);

			Shaders::lPassShader.Use();
			Shaders::lPassShader.SetVec3("viewPos", camera.position);

			Shaders::skyboxShader.Use();
			auto vp = camera.projection * glm::mat4(glm::mat3(camera.view));
			Shaders::skyboxShader.SetMat4("VP", vp);
		}
		void SyncComponents(entt::registry& reg)
		{
			auto pls = reg.view<Components::Transform, Components::PointLight>();

			for (auto& id : pls)
			{
				auto& tr = pls.get(id)._Myfirst._Val;
				auto& pl = pls.get(id)._Get_rest()._Myfirst._Val;

				pl.GetMutableData().position = tr.position;
			}
		}
		void RenderScene(std::span<Renderer::Model> models)
		{
			Renderer::DeferredPipeline::GeometryPass(gFb, Shaders::gPassShader, models);
			Renderer::DeferredPipeline::LightPass(gFb, lFb, Shaders::lPassShader);
		}
		void Update(Scene& scene)
		{
			auto models = GetModels(scene.registry);

			SyncComponents(scene.registry);
			auto& cam = scene.GetMainCamera();
			ViewProjectionUpdate(cam);
			UpdateLights(scene.registry);
			RenderScene(models);

			Renderer::Framebuffer::BindDefault();
		}
	public:
		Renderer::LightStateSynchronizer lightStateSyncher;
		Renderer::GeometryFramebuffer gFb;
		Renderer::LightFramebuffer lFb;
		std::array<Renderer::Framebuffer, 5> pShadowFramebuffers;
		std::array<Renderer::Framebuffer, 5> dShadowFramebuffers;
	public:
		int pointSize = 20;
		float pointInternalSize = 0.5f, pointAA = 0.3f, lineWidth = 0.005f;
		float pointAlpha = 1.0f, lineAlpha = 1.0f, triAlpha = 0.6f;
		glm::vec4 pointColor = { 1,0,0, 1.0f };
		glm::vec4 lineColor = { 0,0,1, 0.3f };
		glm::vec4 triangleColor = { 0,1,0, 0.4f };
		glm::vec3 selectedColor = { 1,0,1 };
		glm::uvec2 resolution = { 1920, 1080 };
	};
}