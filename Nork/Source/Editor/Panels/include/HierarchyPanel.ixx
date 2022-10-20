export module Nork.Editor.Panels:HierarchyPanel;

export import :Panel;

export namespace Nork::Editor {
	class HierarchyPanel : public Panel
	{
	public:
		HierarchyPanel();
		void Content() override;
		const char* GetName() override { return "Hierarchy"; };
	private:
		void RecursiveDraw(std::shared_ptr<SceneNode> node);
	};
}