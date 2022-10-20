module Nork.Editor.Panels;

namespace Nork::Editor {
	ShadersPanel::ShadersPanel(std::shared_ptr<Renderer::Shader> shader, const std::string& name)
		: shaderView(shader), name(name)
	{}
	void ShadersPanel::Content()
	{
		shaderView.Content();
	}
	const char* ShadersPanel::GetName()
	{
		return name.c_str();
	}
}