#include "pch.h"
#include "Deferred.h"
#include "../Utils.h"
#include "../Config.h"

namespace Nork::Renderer::Pipeline
{
	/*void DeferredData::SetMainRes(int x, int y)
	{
		mainResX = x; mainResY = y;

		unsigned int texs[]{ gBuffer.pos, gBuffer.diff, gBuffer.norm, gBuffer.depth, lightPass.tex };
		unsigned int fbs[]{ gBuffer.fb, lightPass.fb };
		glDeleteTextures(sizeof(texs) / sizeof(unsigned int), texs);
		glDeleteFramebuffers(sizeof(fbs) / sizeof(unsigned int), fbs);

		using enum Utils::Texture::Format;

		gBuffer.fb = Utils::Framebuffer::Builder(x, y)
			.AddTexture(&gBuffer.pos, RGBA16F, GL_COLOR_ATTACHMENT0)
			.AddTexture(&gBuffer.diff, RGBA16F, GL_COLOR_ATTACHMENT1)
			.AddTexture(&gBuffer.norm, RGBA16F, GL_COLOR_ATTACHMENT2)
			.AddTexture(&gBuffer.depth, Depth16, GL_DEPTH_ATTACHMENT) // DEBUG
			//.AddRenderbuffer(&gBuffer.depth, TextureFormat::Depth32, GL_DEPTH_ATTACHMENT)
			.GetFramebuffer();

		GLenum bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(sizeof(bufs) / sizeof(GLenum), bufs);

		lightPass.fb = Utils::Framebuffer::Builder(x, y)
			.AddTexture(&lightPass.tex, RGBA16F, GL_COLOR_ATTACHMENT0)
			.AddTexture(gBuffer.depth, GL_DEPTH_ATTACHMENT)
			.GetFramebuffer();

		GLenum bufs2[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(sizeof(bufs2) / sizeof(GLenum), bufs2);
	}*/

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
		//this->data.SetMainRes(1080, 1920);
		Logger::Info("Deferred pipeline initialized");
	}
	static std::chrono::high_resolution_clock clock;

	void Deferred::UseShadowMap(DirShadow shadow, ShadowFramebuffer& fb)
	{
		GLuint depth = fb.GetDepthAttachment();
		Utils::Texture::Bind(depth, shadow.idx + Config::LightData::dirShadowBaseIndex);
	}
	void Deferred::UseShadowMap(PointShadow shadow, ShadowFramebuffer& fb)
	{
		GLuint depth = fb.GetDepthAttachment();
		Utils::Texture::Bind(depth, shadow.idx + Config::LightData::pointShadowBaseIndex);
	}

	void Deferred::DrawScene(std::span<Model> models, LightPassFramebuffer& lightFb, GeometryFramebuffer& gFb)
	{
		gFb.ClearAndUse();
		DrawGBuffers(models);
		lightFb.ClearAndUse();
		DrawLightPass(gFb);
		DrawSkybox();
		FramebufferBase::UseDefault();
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
			for (int i = 0; i < meshes.size(); i++)
			{
				meshes[i].BindTextures();
				meshes[i].Draw();
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