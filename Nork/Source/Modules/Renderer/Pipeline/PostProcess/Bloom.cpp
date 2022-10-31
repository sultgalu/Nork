#include "pch.h"
#include "Bloom.h"
#include "../../Objects/Buffer/BufferBuilder.h"
#include "../../Objects/Texture/TextureBuilder.h"
#include "../../Objects/Framebuffer/FramebufferBuilder.h"
#include "../../DrawUtils.h"
#include "../../State/Capabilities.h"

namespace Nork::Renderer {
	static constexpr size_t maxTexCount = 20;

	Bloom::Bloom(uint32_t width)
	{
		highResY = width;

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

		blackTex = TextureBuilder()
			.Params(TextureParams::FramebufferTex2DParams())
			.Attributes(TextureAttributes{ .width = 1, .height = 1, .format = TextureFormat::RGB8 })
			.Create2DEmpty();
	}
	void Bloom::Apply(std::shared_ptr<MainFramebuffer> mainFb, std::shared_ptr<Shader> shader1, std::shared_ptr<Shader> shader3, std::shared_ptr<Shader> shader2)
	{
		Capabilities().Disable().DepthTest().Blend();

		shader1->Use()
			.SetInt("tex", 0);
		mainFb->Color()->Bind();
		fbs[0]->Bind().Clear().SetViewport();
		DrawUtils::DrawQuad();
		fbs[0]->GetAttachments().colors[0].first->Bind2D();

		shader3->Use()
			.SetInt("tex", 0);
		for (size_t i = 1; i < fbs.size(); i++)
		{
			fbs[i]->Bind().Clear().SetViewport();
			DrawUtils::DrawQuad();
			fbs[i]->GetAttachments().colors[0].first->Bind2D();
		}
		shader2->Use()
			.SetInt("tex", 0)
			.SetInt("tex2", 1);
		for (int i = fbs.size() - 2; i >= 0; i--)
		{
			fbs[i]->GetAttachments().colors[0].first->Bind2D(1);
			fbs2[i]->Bind().Clear().SetViewport();
			DrawUtils::DrawQuad();
			fbs2[i]->GetAttachments().colors[0].first->Bind2D();
		}

		mainFb->Bind().SetViewport();
		blackTex->Bind(1);
		DrawUtils::DrawQuad();
	}
}