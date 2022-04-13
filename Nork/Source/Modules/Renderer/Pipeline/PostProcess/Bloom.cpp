#include "pch.h"
#include "Bloom.h"
#include "../../Objects/Buffer/BufferBuilder.h"
#include "../../Objects/Texture/TextureBuilder.h"
#include "../../Objects/Framebuffer/FramebufferBuilder.h"
#include "../../DrawUtils.h"
#include "../../State/Capabilities.h"

namespace Nork::Renderer {
	static constexpr size_t maxTexCount = 20;

	Bloom::Bloom()
	{
		// using enum BufferStorageFlags;
		// ubo = BufferBuilder().Target(BufferTarget::UBO)
		// 	.Flags(WriteAccess | Coherent | Persistent)
		// 	.Data(nullptr, 20 * sizeof(uint64_t))
		// 	.Create();
		// ubo->BindBase(10).Map(BufferAccess::Write);
	}
	void Bloom::InitTextures(uint32_t size, uint32_t baseX, uint32_t baseY)
	{
		uint32_t x = baseX;
		uint32_t y = baseY;
		for (size_t i = 0; i < size; i++)
		{
			auto params = TextureParams::FramebufferTex2DParams();
			//params.magLinear = false;
			//params.filter = TextureFilter::Nearest;
			auto tex = TextureBuilder()
				.Params(params)
				.Attributes(TextureAttributes{ .width = x, .height = y, .format = TextureFormat::RGB16F })
				.Create2DEmpty();

			fbs.push_back(FramebufferBuilder().Attachments(FramebufferAttachments().Color(tex, 0))
				.Create());
			x /= 2;
			y /= 2;
		}

		x = baseX;
		y = baseY;
		for (size_t i = 0; i < size; i++)
		{
			auto params = TextureParams::FramebufferTex2DParams();
			//params.magLinear = false;
			auto tex = TextureBuilder()
				.Params(params)
				.Attributes(TextureAttributes{ .width = x, .height = y, .format = TextureFormat::RGB16F })
				.Create2DEmpty();

			fbs2.push_back(FramebufferBuilder().Attachments(FramebufferAttachments().Color(tex, 0))
				.Create());
			x /= 2;
			y /= 2;
		}

		auto params = TextureParams::FramebufferTex2DParams();
		//params.magLinear = false;
		auto tex = TextureBuilder()
			.Params(params)
			.Attributes(TextureAttributes{ .width = baseX, .height = baseY, .format = TextureFormat::RGB16F })
			.Create2DEmpty();

		dest = FramebufferBuilder().Attachments(FramebufferAttachments().Color(tex, 0))
			.Create();
	}
	void Bloom::Apply(std::shared_ptr<Texture2D> sourceTex, std::shared_ptr<Shader> shader1, std::shared_ptr<Shader> shader3, std::shared_ptr<Shader> shader2)
	{
		Capabilities().Disable().DepthTest().Blend();

		shader1->Use()
			.SetInt("tex", 0);
		sourceTex->Bind();
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

		dest->Bind().Clear().SetViewport();
		sourceTex->Bind(1);
		DrawUtils::DrawQuad();
	}
}