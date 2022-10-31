#include "PostProcessStage.h"
#include "../../State/Capabilities.h"
#include "../../DrawUtils.h"
#include "../../Objects/Framebuffer/FramebufferBuilder.h"
#include "../../Objects/Texture/TextureBuilder.h"

namespace Nork::Renderer {
	PostProcessStage::PostProcessStage(std::shared_ptr<Shader> shader)
		:shader(shader)
	{}

	bool PostProcessStage::Execute(Framebuffer& source, Framebuffer& destination)
	{
		// static auto fb = FramebufferBuilder().Attachments(
		//  	FramebufferAttachments().Color(TextureBuilder()
		//  		.Attributes(mainFb.Color()->GetAttributes())
		//  		.Params(TextureParams::FramebufferTex2DParams())
		//  		.Create2DEmpty(), 0))
		// 	.Create();
		//  
		// mainFb.Bind(GL_READ_FRAMEBUFFER);
		// fb->Bind(GL_DRAW_FRAMEBUFFER);
		// glBlitFramebuffer(0, 0, mainFb.Width(), mainFb.Height(), 0, 0, fb->Width(), fb->Height(),
		// 	GL_COLOR_BUFFER_BIT, GL_LINEAR);
		source.Color()->Bind2D();
		destination.Bind().SetViewport();
		shader->Use();
		Capabilities()
			.Disable().DepthTest().Blend();
		Renderer::DrawUtils::DrawQuad();
		return true;
	}
}
