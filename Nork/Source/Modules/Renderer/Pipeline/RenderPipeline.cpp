#include "Pipeline.h"
#include "../Objects/Framebuffer/FramebufferBuilder.h"
#include "../Objects/Texture/TextureBuilder.h"

namespace Nork::Renderer {
	Pipeline::Pipeline(uint32_t width, uint32_t height, Renderer::TextureFormat texFormat)
	{
		using namespace Renderer;
		static auto depth = TextureBuilder()
			.Attributes(TextureAttributes{ .width = width, .height = height, .format = TextureFormat::Depth32 })
			.Params(TextureParams::FramebufferTex2DParams())
			.Create2DEmpty();
			
		auto texBuilder = TextureBuilder()
			.Attributes(TextureAttributes{ .width = width, .height = height, .format = texFormat })
			.Params(TextureParams::FramebufferTex2DParams());
		source = FramebufferBuilder().Attachments(FramebufferAttachments()
			.Color(texBuilder.Create2DEmpty(), 0)
			.Depth(depth))
			.Create();
		destination = FramebufferBuilder().Attachments(FramebufferAttachments()
			.Color(texBuilder.Create2DEmpty(), 0)
			.Depth(depth))
			.Create();
	}

	void Pipeline::Run()
	{
		source->Bind().Clear();
		destination->Bind().Clear();
		for (auto& stage : stages)
		{
			if (stage->Execute(*source, *destination))
			{
				source.swap(destination);
			}
		}
		source->Bind().ClearDepth();
	}
}