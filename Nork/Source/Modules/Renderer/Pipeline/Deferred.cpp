#include "pch.h"
#include "Deferred.h"
#include "../Utils.h"

namespace Nork::Renderer::Pipeline
{
	void DeferredData::SetMainRes(int x, int y)
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
	}

	void DeferredData::SetLPassShader(Shader shader)
	{
		shaders.lPass = shader;

		shader.SetInt("gPos", 0);
		shader.SetInt("gDiff", 1);
		shader.SetInt("gNorm", 2);
		for (int i = 0; i < 5; i++)
			shader.SetInt("shadowMaps[" + std::to_string(i) + "]", i + 10);
		for (int i = 0; i < 5; i++)
			shader.SetInt("shadowMapsCube[" + std::to_string(i) + "]", i + 15);
	}

	void DeferredData::SetGPassShader(Shader shader)
	{
		shaders.gPass = shader;

		shader.SetInt("materialTex.diffuse", (int)TextureType::Diffuse);
		shader.SetInt("materialTex.normals", (int)TextureType::Normal);
		shader.SetInt("materialTex.roughness", (int)TextureType::Roughness);
		shader.SetInt("materialTex.reflect", (int)TextureType::Reflection);
	}

	Deferred::Deferred(DeferredData data)
		: data(data)
	{
		this->data.SetMainRes(1080, 1920);
		Logger::Info("Deferred pipeline initialized");
	}
	static std::chrono::high_resolution_clock clock;

	void Deferred::DrawScene(std::span<Model> models)
	{
		DrawGBuffers(models);
		DrawLightPass();
		//DrawSkybox();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void Deferred::DrawGBuffers(std::span<Model> models)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glDisable(GL_BLEND);

		glViewport(0, 0, data.mainResX, data.mainResY);
		glBindFramebuffer(GL_FRAMEBUFFER, data.gBuffer.fb);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	void Deferred::DrawLightPass()
	{
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, data.mainResX, data.mainResY);
		glBindFramebuffer(GL_FRAMEBUFFER, data.lightPass.fb); // gets cleared in BeginFrame
		glClear(GL_COLOR_BUFFER_BIT);

		Utils::Texture::Bind(data.gBuffer.pos, 0);
		Utils::Texture::Bind(data.gBuffer.diff, 1);
		Utils::Texture::Bind(data.gBuffer.norm, 2);

		data.shaders.lPass.Use();
		Utils::Draw::Quad();
		glEnable(GL_DEPTH_TEST);
	}
	void Deferred::DrawSkybox()
	{
		//glBindFramebuffer(GL_FRAMEBUFFER, data.lightPass.fb); generates performance error with IntelHD

		glDepthFunc(GL_LEQUAL);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, data.skyboxTex);
		data.shaders.skybox.Use();

		Utils::Draw::Cubemap();
		glDepthFunc(GL_LESS);
	}
}