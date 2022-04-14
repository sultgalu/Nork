#pragma once
#include "Base/Panel.h"

namespace Nork::Editor2
{
	class MeshEditorPanel : public Panel
	{
	public:
		MeshEditorPanel(EditorData& d);
		~MeshEditorPanel() = default;
	private:
	protected:
		virtual void DrawContent() override;
		//MeshWorld<Engine::Vertex>& meshes = data.engine.meshes;
	};
}