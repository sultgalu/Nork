#pragma once
#include "Scene/Scene.h"

namespace Nork {
	class ScriptSystem
	{
	public:
		ScriptSystem(Scene& scene)
			:scene(scene)
		{}
		void Update();
		Scene& scene;
	};
}