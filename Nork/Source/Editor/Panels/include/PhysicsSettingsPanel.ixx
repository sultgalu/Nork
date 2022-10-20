export module Nork.Editor.Panels:PhysicsSettingsPanel;

export import :Panel;

export namespace Nork::Editor {
	class PhysicsSettingsPanel : public Panel
	{
	public:
		PhysicsSettingsPanel();
		void Content() override;
		const char* GetName() override { return "Physics Settings"; };
	};
}