export module Nork.Editor.Panels:InspectorPanel;

import :Panel;

export namespace Nork::Editor {

	class InspectorPanel : public Panel
	{
	public:
		InspectorPanel();
		void Content() override;
		const char* GetName() override { return "Inspector"; };
	private:
	};
}