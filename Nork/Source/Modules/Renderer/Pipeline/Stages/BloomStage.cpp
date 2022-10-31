#include "BloomStage.h"
#include "../../Objects/Texture/TextureBuilder.h"
#include "../../Objects/Framebuffer/FramebufferBuilder.h"
#include "../../DrawUtils.h"
#include "../../State/Capabilities.h"

namespace Nork::Renderer {
	static constexpr size_t maxTexCount = 20;

	BloomStage::BloomStage(std::shared_ptr<Shader> filterShader, std::shared_ptr<Shader> downscaleShader,
		std::shared_ptr<Shader> upscaleShader, uint32_t height)
		: filterShader(filterShader), downscaleShader(downscaleShader), upscaleShader(upscaleShader)
	{
		highResY = height;

		if (divider < 1.1f || lowResY < 1)
		{
			MetaLogger().Error("Bad Params");
			return;
		}

		uint32_t x = highResY * ratio;
		uint32_t y = highResY;
		while (y > lowResY && y > 0)
		{
			auto params = TextureParams::FramebufferTex2DParams();
			auto tex = TextureBuilder()
				.Params(params)
				.Attributes(TextureAttributes{ .width = x, .height = y, .format = TextureFormat::RGB16F })
				.Create2DEmpty();
			auto tex2 = TextureBuilder()
				.Params(params)
				.Attributes(TextureAttributes{ .width = x, .height = y, .format = TextureFormat::RGB16F })
				.Create2DEmpty();

			fbs.push_back(FramebufferBuilder().Attachments(FramebufferAttachments().Color(tex, 0))
				.Create());
			fbs2.push_back(FramebufferBuilder().Attachments(FramebufferAttachments().Color(tex2, 0))
				.Create());
			x /= divider;
			y /= divider;
		}
	}
	bool BloomStage::Execute(Framebuffer& source, Framebuffer& destination)
	{
		// No need to clear textures, since DrawQuad() overrides every texel
		Capabilities().Disable().DepthTest().Blend();

		filterShader->Use()
			.SetInt("tex", 0);
		source.Color()->Bind2D();
		fbs[0]->Bind().SetViewport();
		DrawUtils::DrawQuad();
		fbs[0]->GetAttachments().colors[0].first->Bind2D();

		downscaleShader->Use()
			.SetInt("tex", 0);
		for (size_t i = 1; i < fbs.size(); i++)
		{
			fbs[i]->Bind().SetViewport();
			DrawUtils::DrawQuad();
			fbs[i]->GetAttachments().colors[0].first->Bind2D();
		}
		upscaleShader->Use()
			.SetInt("tex", 0)
			.SetInt("tex2", 1);
		for (int i = fbs.size() - 2; i >= 0; i--)
		{
			fbs[i]->GetAttachments().colors[0].first->Bind2D(1);
			fbs2[i]->Bind().SetViewport();
			DrawUtils::DrawQuad();
			fbs2[i]->GetAttachments().colors[0].first->Bind2D();
		}

		destination.Bind().SetViewport();
		source.Color()->Bind2D(1);
		DrawUtils::DrawQuad();
		return true;
	}
}