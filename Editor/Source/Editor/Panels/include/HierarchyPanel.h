#pragma once
#include "Panel.h"

namespace Nork::Editor {
	class HierarchyPanel : public Panel
	{
	public:
		HierarchyPanel();
		void Content() override;
		void Duplicate(std::shared_ptr<SceneNode> from, std::shared_ptr<SceneNode> to);
		const char* GetName() override { return "Hierarchy"; };
	private:
		void RecursiveDraw(std::shared_ptr<SceneNode> node);
	};
}