#pragma once
#include "../../View.h"
import Nork.Components;

namespace Nork::Editor {
	class SceneNodeView: public View
	{
	public:
		SceneNodeView(std::shared_ptr<SceneNode> node);
		template<class... T>
		void Content();
		void Content() override;

		template<class... T>
		void EditComponents();
		template<class T>
		bool EditComponent();
		template<class T>
		void ShowComponent(T&, bool&);

		template<class... T>
		void ListComponentsForAddition();
	private:
		std::shared_ptr<SceneNode> node;
	private:
		template<class T, class... Rest>
		void _EditComponents();
	};
}