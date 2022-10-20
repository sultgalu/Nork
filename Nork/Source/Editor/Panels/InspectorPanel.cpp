module Nork.Editor.Panels;

import Nork.Editor.Views;

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