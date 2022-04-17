#include "include/InspectorPanel.h"
#include "../Views/include/Components.h"

namespace Nork::Editor {
	InspectorPanel::InspectorPanel()
	{
	}

	void InspectorPanel::Content()
	{
		if (GetCommonData().selectedNode)
		{
			using namespace Components;
			SceneNodeView(GetCommonData().selectedNode).Content();
		}
		else
		{
			ImGui::Text("Select a component");
		}
	}
}