export module Nork.Editor.Panels:GraphicsSettingsPanel;

import :Panel;

export namespace Nork::Editor {

	class GraphicsSettingsPanel : public Panel
	{
	public:
		GraphicsSettingsPanel();
		void Content() override;
		const char* GetName() override { return "Graphics Settings"; };
	};
}