#pragma once
#include "Base/Panel.h"

namespace Nork::Editor
{
	class LogPanel : public Panel
	{
	public:
		LogPanel();
		~LogPanel();
	protected:
		virtual void DrawContent() override;
	private:
	};
}