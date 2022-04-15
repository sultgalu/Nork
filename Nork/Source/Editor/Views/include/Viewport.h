#pragma once

#include "../../View.h"

namespace Nork::Editor {
	class ViewportView: public View
	{
	public:
		ViewportView();
		void Content() override;
	public:
		std::shared_ptr<Viewport> viewport;
	};
}