export module Nork.Renderer:SkyRenderer;

export import :Texture;
export import :Shader;
export import :DrawUtils;
export import :Capabilities;

export namespace Nork::Renderer {
	class SkyRenderer
	{
	public:
		static void RenderSkybox(TextureCube& texture, Shader& shader)
		{
			Capabilities()
				.Enable().DepthTest(DepthFunc::LessOrEqual)
				.Disable().CullFace();
			texture.Bind();
			shader.Use();
			shader.SetInt("skyBox", 0);

			DrawUtils::DrawCube();
		}
	};
}