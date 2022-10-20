export module Nork.Editor.Panels:BloomPanel;

export import :Panel;

export namespace Nork::Editor {

	class BloomPanel : public Panel
	{
	public:
		BloomPanel();
		void Content() override;
		const char* GetName() override { return "Bloom"; };
	};
}