#pragma once
#include "../Objects/Framebuffer/GeometryFramebuffer.h"
#include "Stages/Stage.h"

namespace Nork::Renderer {

	class Pipeline
	{
	public:
		Pipeline(uint32_t width = 1920, uint32_t height = 1080, Renderer::TextureFormat = Renderer::TextureFormat::RGBA16F);
		void Run();
		std::shared_ptr<Renderer::Texture> FinalTexture() const { return source->Color(); }
		template<std::derived_from<Stage> T> std::shared_ptr<T> Get()
		{
			for (auto& stage : stages)
			{
				if (auto casted = std::dynamic_pointer_cast<T>(stage))
					return casted;
			}
			return nullptr;
		}
		template<std::derived_from<Stage> T> bool Has()
		{
			return Get<T>() != nullptr;
		}
	public:
		std::shared_ptr<Renderer::Framebuffer> source;
		std::shared_ptr<Renderer::Framebuffer> destination;
		std::list<std::shared_ptr<Stage>> stages;
	};
}