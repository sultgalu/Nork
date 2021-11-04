#pragma once

#include "Core/Engine.h"

namespace Nork::Editor
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
		entt::entity selectedEnt = entt::null;
		std::bitset<IdQueryMode::COUNT> idQueryMode;
		Poly* selectedPoly = nullptr;
	};
}