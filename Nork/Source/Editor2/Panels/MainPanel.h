#pragma once
#include "Base/Panel.h"

namespace Nork::Editor2
{
	class MainPanel : public Panel
	{
	public:
		MainPanel(EditorData& d);
	protected:
		virtual void Begin() override;
		virtual void End() override;
		virtual void DrawContent() override;
	private:
		void LoadScene();
		void SaveScene();
	};
}

