export module Nork.Editor.Menus:FileMenu;

export import :Menu;

export namespace Nork::Editor {
	class FileMenu: public Menu
	{
	public:
		using Menu::Menu;
		void Content() override;
		const char* GetName() override { return "File"; }
	private:
		void LoadScene();
		void SaveScene();
	};
}