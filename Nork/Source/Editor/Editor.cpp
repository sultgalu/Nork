#include "pch.h"
#include "Editor.h"
#include "Panels/Base/Panel.h"
#include "Panels/AssetsPanel.h"
#include "Panels/MainPanel.h"
#include "Panels/LogPanel.h"
#include "Panels/ViewportPanel.h"
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>

namespace Nork::Editor
{
	std::vector<Panel*> panels;

	void SetCallbacks(EventManager& evMan)
	{
		auto& imIO = ImGui::GetIO();
		/*evMan.Subscribe<Events::MouseMove>([&imIO](const Event& ev)
			{
				auto& e = ev.As<Events::MouseMove>();
			});*/
		evMan.Subscribe<Events::MouseScroll>([&imIO](const Event& ev)
			{
				imIO.MouseWheel += (float)ev.As<Events::MouseScroll>().offset;
			});
		//evMan.Subscribe<Events::MouseDown>([&imIO](const Event& ev)
		//	{
		//		imIO.MouseDown[Events::ToInt(ev.As<Events::MouseDown>().button)] = true; // left, right, middle for imgui
		//	});
		//evMan.Subscribe<Events::MouseUp>([&imIO](const Event& ev)
		//	{
		//		imIO.MouseDown[Events::ToInt(ev.As<Events::MouseUp>().button)] = false; // left, right, middle for imgui
		//	});
		evMan.Subscribe<Events::KeyUp>([&imIO](const Event& ev)
			{
				Input::Key key = ev.As<Events::KeyUp>().key;
				imIO.KeysDown[Events::ToInt(key)] = false;
				using enum Input::Key;
				switch (key)
				{
				case Shift:
					imIO.KeyShift = false;
					break;
				case Ctrl:
					imIO.KeyCtrl = false;
					break;
				case Alt:
					imIO.KeyAlt = false;
					break;
				case Super: [[unlikely]]
					imIO.KeySuper = false;
					break;
				}
			});
		evMan.Subscribe<Events::KeyDown>([&imIO](const Event& ev)
			{
				Input::Key key = ev.As<Events::KeyDown>().key;
				imIO.KeysDown[Events::ToInt(key)] = true;
				using enum Input::Key;
				switch (key)
				{
				case Shift:
					imIO.KeyShift = true;
					break;
				case Ctrl:
					imIO.KeyCtrl = true;
					break;
				case Alt:
					imIO.KeyAlt = true;
					break;
				case Super: [[unlikely]]
					imIO.KeySuper = true;
					break;
				}
			}); 
		evMan.Subscribe<Events::Type>([&imIO](const Event& ev)
			{
				imIO.AddInputCharacter(ev.As<Events::Type>().character);
			});
		evMan.Subscribe<Events::WindowInFocus>([&imIO](const Event& ev)
			{
				imIO.AddFocusEvent(true);
			});
		evMan.Subscribe<Events::WindowOutOfFocus>([&imIO](const Event& ev)
			{
				imIO.AddFocusEvent(false);
			});
		//bd->PrevUserCallbackWindowFocus = glfwSetWindowFocusCallback(window, ImGui_ImplGlfw_WindowFocusCallback);
		//bd->PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
		//bd->PrevUserCallbackScroll = glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
		//bd->PrevUserCallbackKey = glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
		//bd->PrevUserCallbackCursorEnter = glfwSetCursorEnterCallback(window, ImGui_ImplGlfw_CursorEnterCallback);
		//bd->PrevUserCallbackChar = glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
		//bd->PrevUserCallbackMonitor = glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);
	}

	Editor::Editor(Window& win)
	{
		ImGui::CreateContext();
		ImGui::StyleColorsClassic();

		ImGuiIO& io = ImGui::GetIO();
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking	
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoDefaultParent = true;
		//io.ConfigDockingNoSplit = true;
		io.ConfigWindowsMoveFromTitleBarOnly = true;
		io.ConfigDockingAlwaysTabBar = true;

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 1.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplOpenGL3_Init();
		ImGui_ImplGlfw_InitForOpenGL(&win.GetData(), false);
		SetCallbacks(win.GetInput().GetEventManager());

		panels = std::vector<Panel*>{ new MainPanel(), new AssetsPanel(), new LogPanel(), new ViewportPanel() };
	}

	void Editor::Render()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGui::DockSpace(3, ImVec2(100, 1000));

		for (int i = 0; i < panels.size(); i++)
		{
			if (panels[i]->IsVisible())
			{
				panels[i]->Draw();
			}
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}
	void Editor::SetDisplayTexture(GLuint tex)
	{
		for (auto& panel : panels)
		{
			if (typeid(*panel).hash_code() == typeid(ViewportPanel).hash_code())
			{
				(*((ViewportPanel*)panel)).SetTexture(tex);
			}
		}
	}
}