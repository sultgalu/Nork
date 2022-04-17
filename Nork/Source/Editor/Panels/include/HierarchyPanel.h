#pragma once
#include "Panel.h"

namespace Nork::Editor {
	class HierarchyPanel : public Panel
	{
	public:
		HierarchyPanel();
		void Content() override;
		const char* GetName() override { return "Hierarchy"; };
	private:
		void RecursiveDraw(std::shared_ptr<SceneNode> node);
	};
}