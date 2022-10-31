#include "SkyboxStage.h"
#include "../../Objects/Texture/TextureBuilder.h"
#include "../../State/Capabilities.h"
#include "../../DrawUtils.h"

namespace Nork::Renderer {
	SkyboxStage::SkyboxStage(std::shared_ptr<Shader> shader, const std::array<Image, 6>& images)
		: shader(shader)
	{
		std::array<const void*, 6> data;
		for (size_t i = 0; i < data.size(); i++)
		{
			data[i] = images[i].data.data();
		}
		skybox = TextureBuilder()
			.Attributes(TextureAttributes{ .width = images[0].width, .height = images[0].height, .format = images[0].format })
			.Params(TextureParams::CubeMapParams())
			.CreateCubeWithData(data);
	}

	bool SkyboxStage::Execute(Framebuffer& source, Framebuffer& destination)
	{
		source.Bind().SetViewport();
		Capabilities()
			.Enable().DepthTest(DepthFunc::LessOrEqual)
			.Disable().CullFace();
		skybox->Bind();
		shader->Use();
		shader->SetInt("skyBox", 0);

		DrawUtils::DrawCube();
		return false;
	}
}
