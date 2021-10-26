#pragma once
#include "Base/Panel.h"

namespace Nork::Editor
{
	class MeshEditorPanel : public Panel
	{
	public:
		MeshEditorPanel(EditorData& d);
		~MeshEditorPanel() = default;
	private:
		void SelectVertex(uint32_t i);
	protected:
		virtual void DrawContent() override;
		std::vector<MeshWorld<Engine::Vertex>>& colliders = data.engine.colliders;
		//MeshWorld<Engine::Vertex>& meshes = data.engine.meshes;
	};
}