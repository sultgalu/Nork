#pragma once

namespace Nork::Editor
{
	class Panel
	{
	public:
		Panel(const char* name, bool visible = true) : isVisible(visible), name(name) {}
		void Draw();
		inline bool IsVisible() { return isVisible; }
		inline const std::string_view GetName() { return name; }
	protected:
		virtual void DrawContent() = 0;
	private:
		bool isVisible;
		std::string name;
	};
}

