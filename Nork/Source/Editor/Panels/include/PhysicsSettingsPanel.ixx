export module Nork.Editor.Panels:PhysicsSettingsPanel;

import :Panel;

export namespace Nork::Editor {
	class PhysicsSettingsPanel : public Panel
	{
	public:
		PhysicsSettingsPanel();
		void Content() override;
		const char* GetName() override { return "Physics Settings"; };
	};
}