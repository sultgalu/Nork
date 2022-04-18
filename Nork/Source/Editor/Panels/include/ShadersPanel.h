#pragma once

#include "Panel.h"
#include "../../Views/include/ShaderView.h"

namespace Nork::Editor {

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