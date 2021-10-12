#pragma once

#include "Core/Engine.h"

namespace Nork::Editor
{
	class EditorData
	{
	public:
		inline EditorData(Engine& en) : engine(en) {}
	public:
		Engine& engine;
		entt::entity selectedEnt = entt::null;
	};
}