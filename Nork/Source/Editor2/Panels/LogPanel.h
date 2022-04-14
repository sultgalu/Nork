#pragma once
#include "Base/Panel.h"

namespace Nork::Editor2
{
	class LogPanel : public Panel
	{
	public:
		LogPanel(EditorData& d);
		~LogPanel();
	protected:
		virtual void DrawContent() override;
	};
}