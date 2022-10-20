export module Nork.Editor.Panels:ShadersPanel;

export import :Panel;
import Nork.Editor.Views;

export namespace Nork::Editor {

	class ShadersPanel : public Panel
	{
	public:
		ShadersPanel(std::shared_ptr<Renderer::Shader>, const std::string& name);
		void Content() override;
		const char* GetName() override;
		bool DeleteOnClose() const override { return true; }
	public:
		ShaderView shaderView;
		std::string name;
	};
}