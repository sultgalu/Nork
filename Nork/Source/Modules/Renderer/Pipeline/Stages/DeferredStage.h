#pragma once

#include "../../Objects/Framebuffer/GeometryFramebuffer.h"
#include "../../Objects/Shader/Shader.h"
#include "../../Model/DrawBatch.h"
#include "Stage.h"

namespace Nork::Renderer {
	class RenderPipeline
	{
	public:
		RenderPipeline(uint32_t width = 1920, uint32_t height = 1080, Renderer::TextureFormat = Renderer::TextureFormat::RGBA16F);

		void Render();

		std::shared_ptr<Renderer::Texture> FinalTexture() const { return fb->Color(); }

		std::shared_ptr<Renderer::MainFramebuffer> fb;
		std::vector<Stage> stages;

		bool active = true;
	};

	class DrawCommandProvider
	{
	public:
		virtual const std::vector<DrawCommandMultiIndirect>& operator()() = 0;
	};

	class DeferredStage : public Stage
	{
	public:
		DeferredStage(std::shared_ptr<Texture2D> depth, std::shared_ptr<Shader> gShader, std::shared_ptr<Shader> lShader, DrawCommandProvider* provider);
		bool Execute(Framebuffer& source, Framebuffer& destination) override;
	private:
		void GeometryPass();
		void LightPass(Framebuffer&);
	public:
		std::shared_ptr<GeometryFramebuffer> geometryFb;
		std::shared_ptr<Shader> gShader;
		std::shared_ptr<Shader> lShader;
		DrawCommandProvider* drawCommandProvider;
	};
}