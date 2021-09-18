#pragma once

#include <Platform/Interface/Windows.h>

namespace Nork::Editor
{
	template<typename T>
	class Editor
	{
	public:
		Editor(Window<T>&);
		void Render();
	};
}
