export module Nork.Editor.Views:ShaderView;

import :View;

export namespace Nork::Editor {
	class ShaderView : public View
	{
	public:
		ShaderView(std::shared_ptr<Renderer::Shader>);
		void Content() override;
		void UpdateUniformValues();
		void FlushUniformValues();
		std::shared_ptr<Renderer::Shader> Shader() { return shader; }
	private:
		std::shared_ptr<Renderer::Shader> shader;

		std::unordered_map<std::string, float> uniformsFloat;
		std::unordered_map<std::string, glm::vec2> uniformsVec2;
		std::unordered_map<std::string, glm::vec3> uniformsVec3;
		std::unordered_map<std::string, glm::vec4> uniformsVec4;
		std::unordered_map<std::string, int> uniformsInt;
		bool immediateMode = true;
		bool keepValues = true;
		bool colorEdit = true;
	};
}