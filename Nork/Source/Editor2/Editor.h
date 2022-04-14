#pragma once

#include "Core/Engine.h"
#include "EditorData.h"

namespace Nork::Editor2
{
	class Editor
	{
	public:
		Editor(Engine& engine);
		void Render();
		void SetDisplayTexture(std::shared_ptr<Renderer::Texture> tex);
		void Update();
		void UpdateImguiInputs();
	private:
		EditorData data;
	};
}