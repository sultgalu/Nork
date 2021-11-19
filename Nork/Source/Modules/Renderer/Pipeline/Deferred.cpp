#include "pch.h"
#include "Deferred.h"
#include "../Utils.h"
#include "../Config.h"

#include "../Resource/ResourceCreator.h"

namespace Nork::Renderer::Pipeline
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

	template<class T>
	struct SSBO
	{
		SSBO(uint32_t idx, size_t size = 0, void* data = nullptr)
		{
			glGenBuffers(1, &id);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
			glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, idx, id);
		}
		void Data(std::span<T> data)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
			glBufferData(GL_SHADER_STORAGE_BUFFER, data.size_bytes(), data.data(), GL_DYNAMIC_DRAW);
			this->size = data.size_bytes();
		}

		void Data(std::vector<T>& data)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
			glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_DYNAMIC_DRAW);
			this->size = data.size() * sizeof(T);
		}

		void SetSize(size_t size)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
			glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
			this->size = size;
		}
		void GetData(std::span<T> dest)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, dest.size_bytes(), dest.data());
		}
		void GetData(void* dest, uint32_t size)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, dest);
		}

		GLuint id;
		size_t size;
	};
}