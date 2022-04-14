#include "pch.h"
#include "Editor.h"
#include "Panels/All.h"
#include "App/Application.h"

namespace Nork::Editor2
{
	std::vector<Panel*> panels;

	void InitImGui()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsClassic();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
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
		ImGui_ImplGlfw_InitForOpenGL(Application::Get().window.Underlying().GetContext().glfwWinPtr, false);
	}

	Editor::Editor(Engine& engine)
		: data(engine)
	{
		InitImGui();
		panels = std::vector<Panel*>{ new MainPanel(data), new AssetsPanel(data), new LogPanel(data),
			new ViewportPanel(data), new InspectorPanel(data), new HierarchyPanel(data), new MeshEditorPanel(data)};
		SetDisplayTexture(engine.renderingSystem.GetTargetFramebuffers()[0]->GetAttachments().colors[0].first);
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
	void Editor::SetDisplayTexture(std::shared_ptr<Renderer::Texture> tex)
	{
		for (auto& panel : panels)
		{
			if (typeid(*panel).hash_code() == typeid(ViewportPanel).hash_code())
			{
				(*((ViewportPanel*)panel)).SetTexture(tex);
			}
		}
	}
	void Editor::Update()
	{
		UpdateImguiInputs();
		Render();
		data.engine.Update();
	}
	void Editor::UpdateImguiInputs()
	{
		auto& imIO = ImGui::GetIO();
		auto ptr = &imIO;
		auto& input = Application::Get().window.Input();
		imIO.MouseWheel += (float)input.ScrollOffs();

		//evMan.Subscribe<Events::MouseDown>([&imIO](const Event& ev)
		//	{
		//		imIO.MouseDown[Events::ToInt(ev.As<Events::MouseDown>().button)] = true; // left, right, middle for imgui
		//	});
		//evMan.Subscribe<Events::MouseUp>([&imIO](const Event& ev)
		//	{
		//		imIO.MouseDown[Events::ToInt(ev.As<Events::MouseUp>().button)] = false; // left, right, middle for imgui
		//	});
		imIO.KeyShift = input.IsDown(Key::Shift);
		imIO.KeyCtrl = input.IsDown(Key::Ctrl);
		imIO.KeyAlt = input.IsDown(Key::Alt);
		imIO.KeySuper = input.IsDown(Key::Super);
		for (size_t i = 0; i < input.KeysDown().size(); i++)
		{
			imIO.KeysDown[i] = input.KeysDown()[i];
		}

		for (auto c : input.TypedCharacters())
		{
			imIO.AddInputCharacter(c);
		}
				//imIO.AddFocusEvent(true);
				//imIO.AddFocusEvent(false);
	}
}