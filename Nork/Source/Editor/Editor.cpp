#include "pch.h"
#include "Editor.h"
#include "Panels/All.h"

namespace Nork::Editor
{
	std::vector<Panel*> panels;

	void SetCallbacks(Event::Receiver& disp)
	{
		auto& imIO = ImGui::GetIO();
		auto ptr = &imIO;
		/*evMan.Subscribe<Events::MouseMove>([&imIO](const Event& ev)
			{
				auto& e = ev.As<Events::MouseMove>();
			});*/
		disp.Subscribe<Event::Types::MouseScroll>([&imIO](const Event::Types::Base& ev)
			{
				imIO.MouseWheel += (float)ev.As<Event::Types::MouseScroll>().offset;
			});
		//evMan.Subscribe<Events::MouseDown>([&imIO](const Event& ev)
		//	{
		//		imIO.MouseDown[Events::ToInt(ev.As<Events::MouseDown>().button)] = true; // left, right, middle for imgui
		//	});
		//evMan.Subscribe<Events::MouseUp>([&imIO](const Event& ev)
		//	{
		//		imIO.MouseDown[Events::ToInt(ev.As<Events::MouseUp>().button)] = false; // left, right, middle for imgui
		//	});
		disp.Subscribe<Event::Types::KeyUp>([&imIO](const Event::Types::KeyUp& ev)
			{
				imIO.KeysDown[ev.AsInt()] = false;
				using enum Input::KeyType;
				switch (ev.key)
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
		disp.Subscribe<Event::Types::KeyDown>([&imIO](const Event::Types::KeyDown& ev)
			{
				imIO.KeysDown[ev.AsInt()] = false;
				using enum Input::KeyType;
				switch (ev.key)
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
		disp.Subscribe<Event::Types::Type>([&imIO](const Event::Types::Type& ev)
			{
				imIO.AddInputCharacter(ev.As<Event::Types::Type>().character);
			});
		disp.Subscribe<Event::Types::WindowInFocus>([&imIO](const Event::Types::WindowInFocus& ev)
			{
				imIO.AddFocusEvent(true);
			});
		disp.Subscribe<Event::Types::WindowOutOfFocus>([&imIO](const Event::Types::WindowOutOfFocus& ev)
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

	void InitImGui(Window& win)
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
		SetCallbacks(win.GetInputEvents());
	}

	Editor::Editor(Engine& engine)
		: data(engine)
	{
		InitImGui(engine.window);
		panels = std::vector<Panel*>{ new MainPanel(data), new AssetsPanel(data), new LogPanel(data),
			new ViewportPanel(data), new InspectorPanel(data), new HierarchyPanel(data) };
	}

	void Editor::Render()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGui::DockSpace(3, ImVec2(100, 1000));

		for (int i = 0; i < panels.size(); i++)
		{
			panels[i]->Draw();
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