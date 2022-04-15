#include "pch.h"
#include "include/ViewportPanel.h"

namespace Nork::Editor {
	ViewportPanel::ViewportPanel()
	{
		camera = std::make_shared<Components::Camera>();
		GetCommonData().editorCameras.push_back(camera);

		viewport = std::make_shared<Viewport>(camera);
		GetRenderer().viewports.push_back(viewport);

		viewportView.viewport = viewport;

		panelState.windowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollWithMouse;
			//| ImGuiWindowFlags_NoSavedSettings;
	}
	ViewportPanel::~ViewportPanel()
	{
		for (size_t i = 0; i < GetRenderer().viewports.size(); i++)
		{
			if (GetRenderer().viewports[i] == viewport)
			{
				GetRenderer().viewports.erase(GetRenderer().viewports.begin() + i);
			}
		}
		for (size_t i = 0; i < GetCommonData().editorCameras.size(); i++)
		{
			if (GetCommonData().editorCameras[i] == camera)
			{
				GetCommonData().editorCameras.erase(GetCommonData().editorCameras.begin() + i);
			}
		}
	}
	void ViewportPanel::Content()
	{
		viewportView.viewport->active = true;
		viewportView.Content();
	}
	void ViewportPanel::OnContentSkipped()
	{
		viewportView.viewport->active = false;
	}
}
