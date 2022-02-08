#include "pch.h"
#include "Deferred.h"
#include "../Utils.h"
#include "../Config.h"
#include "Capabilities.h"

#include "../Resource/ResourceCreator.h"

namespace Nork::Renderer
{
	void DeferredData::SetLPassShader(Shader shader)
	{
		shaders.lPass = shader;

		shader.Use();
		shader.SetInt("gPos", 0);
		shader.SetInt("gDiff", 1);
		shader.SetInt("gNorm", 2);
		shader.SetInt("gSpec", 3);
		for (int i = 0; i < Config::LightData::dirShadowsLimit; i++)
			shader.SetInt("dirShadowMaps[" + std::to_string(i) + "]", i + Config::LightData::dirShadowBaseIndex);
		for (int i = 0; i < Config::LightData::pointShadowsLimit; i++)
			shader.SetInt("pointShadowMaps[" + std::to_string(i) + "]", i + Config::LightData::pointShadowBaseIndex);
	}

	void DeferredData::SetGPassShader(Shader shader)
	{
		shaders.gPass = shader;

		shader.Use();
		shader.SetInt("materialTex.diffuse", (int)TextureUse::Diffuse);
		shader.SetInt("materialTex.normals", (int)TextureUse::Normal);
		shader.SetInt("materialTex.roughness", (int)TextureUse::Roughness);
		shader.SetInt("materialTex.reflect", (int)TextureUse::Reflection);
	}

	Deferred::Deferred(DeferredData data)
		: data(data)
	{
		Logger::Info("Deferred pipeline initialized");
	}
	static std::chrono::high_resolution_clock clock;

	void Deferred::UseShadowMap(DirShadow shadow, DirShadowFramebuffer& fb)
	{
		GLuint depth = fb.Texture();
		Utils::Texture::Bind(depth, shadow.idx + Config::LightData::dirShadowBaseIndex);
	}
	void Deferred::UseShadowMap(PointShadow shadow, PointShadowFramebuffer& fb)
	{
		using namespace Utils::Texture;
		GLuint depth = fb.Texture();
		Bind<TextureType::Cube>(depth, shadow.idx + Config::LightData::pointShadowBaseIndex);
	}

	void Deferred::DrawScene(std::span<Model> models, LightPassFramebuffer& lightFb, GeometryFramebuffer& gFb)
	{
		gFb.ClearAndUse();
		DrawGBuffers(models);
		lightFb.ClearAndUse();
		DrawLightPass(gFb);
		//DrawSkybox();
	}
	void Deferred::DrawGBuffers(std::span<Model> models)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glDisable(GL_BLEND);
		
		data.shaders.gPass.Use();
		for (size_t i = 0; i < models.size(); i++)
		{
			auto& meshes = models[i].first;
			auto& mat = models[i].second;

			data.shaders.gPass.SetMat4("model", mat);
			data.shaders.gPass.SetInt("colliding", meshes[0].colliding ? 1 : 0);
			for (int j = 0; j < meshes.size(); j++)
			{
				meshes[j].BindTextures();
				meshes[j].Draw();
			};
		}
	}
	void Deferred::DrawLightPass(GeometryFramebuffer& gFb)
	{
		glDisable(GL_DEPTH_TEST);

		Utils::Texture::Bind(gFb.Position(), 0);
		Utils::Texture::Bind(gFb.Diffuse(), 1);
		Utils::Texture::Bind(gFb.Normal(), 2);
		Utils::Texture::Bind(gFb.Specular(), 3);

		data.shaders.lPass.Use();
		Utils::Draw::Quad();
		glEnable(GL_DEPTH_TEST);
	}
	void Deferred::DrawSkybox()
	{
		glDepthFunc(GL_LEQUAL);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, data.skyboxTex);
		data.shaders.skybox.Use();

		Utils::Draw::Cubemap();
		glDepthFunc(GL_LESS);
	}
}