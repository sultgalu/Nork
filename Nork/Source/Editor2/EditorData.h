#pragma once

#include "Core/Engine.h"

namespace Nork::Editor2
{
	namespace IdQueryMode
	{
		enum Enum
		{
			DoubleClick = 0, Click, MouseMoveClicked, MouseMoveReleased, 
			COUNT
		};
	}

	class EditorData
	{
	public:
		inline EditorData(Engine& en) : engine(en) {}
	public:
		Engine& engine;
		SceneNode* selectedNode = nullptr;
		std::bitset<IdQueryMode::COUNT> idQueryMode;
		Polygon* selectedPoly = nullptr;
	};
}