export module Nork.Editor.Views:Viewport;

export import :View;

export namespace Nork::Editor {
	class ViewportView: public View
	{
	public:
		ViewportView();
		void Content() override;
	public:
		std::shared_ptr<Viewport> viewport;
	};
}