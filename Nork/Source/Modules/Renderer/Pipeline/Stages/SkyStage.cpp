#include "SkyStage.h"
#include "../../State/Capabilities.h"
#include "../../DrawUtils.h"

Nork::Renderer::SkyStage::SkyStage(std::shared_ptr<Shader> shader)
	: shader(shader)
{}

bool Nork::Renderer::SkyStage::Execute(Framebuffer& source, Framebuffer& destination)
{
	source.Bind().SetViewport();
	Capabilities()
		.Enable().DepthTest(Renderer::DepthFunc::LessOrEqual)
		.Disable().CullFace();
	shader->Use();
	Renderer::DrawUtils::DrawCube();
	return false;
}
