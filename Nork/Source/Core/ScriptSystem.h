#pragma once
import Nork.Scene;

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