#include "pch.h"
#include "Editor.h"
#include "App/Application.h"
#include "Panels/include/All.h"
#include "Menus/include/All.h"
#include "Modules/Renderer/Vulkan/Window.h"

namespace Nork::Editor
{
	static CommonData* _commonData;
	static Editor* _editor;
	CommonData& _GetCommonData()
	{
		return *_commonData;
	}
	Editor& _GetEditor()
	{
		return *_editor;
	}

	class CustomPanel : public Panel
	{
	public:
		using Panel::Panel;
		void Content() override
		{
			for (auto& view : views)
			{
				ImGui::Separator();
				view->Content();
				ImGui::Separator();
			}
		}
		CustomPanel& SetName(const char* n) { name = n; return *this; }
		CustomPanel& AddView(std::shared_ptr<View> view) { views.push_back(view); return *this; }
		const char* GetName() override { return name; }
	private:
		std::vector<std::shared_ptr<View>> views;
		const char* name;
	};

	static void ReadImGuiIniFile(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line)
	{
		if (handler->TypeName != std::string("Cameras"))
		{
			return;
		}
		JsonObject root;
		try
		{
			root = JsonObject::Parse(line);
		} catch (std::exception& e)
		{
			return;
		}
		if (!root.Contains("cameras"))
		{
			return;
		}
		
		auto& data = _GetCommonData();
		auto& editor = _GetEditor();

		if (root.Contains("camera")) {
			auto json = root.Get<JsonObject>("camera");
			auto& cam = *editor.GetPanel<ViewportPanel>()->viewportView.camera;
			json.Get<JsonArray>("position").Get(&cam.position.x, 3);
			json.Get<JsonArray>("up").Get(&cam.up.x, 3);
			json.Get("nearClip", cam.nearClip);
			json.Get("farClip", cam.farClip);
			json.Get("yaw", cam.yaw);
			json.Get("pitch", cam.pitch);
			json.Get("ratio", cam.ratio);
			// json.Get("zoomSpeed", comp.zoomSpeed);
			// json.Get("moveSpeed", comp.moveSpeed);
			// json.Get("rotationSpeed", comp.rotationSpeed);
			json.Get("FOV", cam.FOV);
			cam.Update();
		}
	}
	static void WriteImGuiIniFile(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf)
	{
		auto& data = _GetCommonData();

		auto& component = *Editor::Get().GetPanel<ViewportPanel>()->viewportView.camera;
		auto jsonCam = JsonObject()
			.Property("position", JsonArray().Elements(&component.position.x, 3))
			.Property("up", JsonArray().Elements(&component.up.x, 3))
			.Property("farClip", component.farClip)
			.Property("nearClip", component.nearClip)
			.Property("pitch", component.pitch)
			.Property("ratio", component.ratio)
			.Property("yaw", component.yaw)
			// .Property("moveSpeed", component.moveSpeed)
			// .Property("rotationSpeed", component.rotationSpeed)
			// .Property("zoomSpeed", component.zoomSpeed);
			.Property("FOV", component.FOV);

		out_buf->appendf("[Cameras][stuff]\n");
		out_buf->append(JsonObject().Property("camera", jsonCam).ToString().c_str());
	}
	static void* OpenImGuiIniFile(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name)
	{
		static Components::Camera cam;
		Logger::Info("OPENnnnnnnnNNNNNNNNNNNNNNed: ", name);
		return &cam;
	}
	void InitImgui()
	{
		ImGui::CreateContext();

		auto g = ImGui::GetCurrentContext();
		ImGuiSettingsHandler ini_handler;
		ini_handler.TypeName = "Cameras";
		ini_handler.TypeHash = ImHashStr("Cameras");
		ini_handler.ReadOpenFn = OpenImGuiIniFile;
		ini_handler.ReadLineFn = ReadImGuiIniFile;
		ini_handler.WriteAllFn = WriteImGuiIniFile;
		g->SettingsHandlers.push_back(ini_handler);
		ImGui::StyleColorsClassic();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking	
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
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

		//ImGui_ImplOpenGL3_Init();
		//ImGui_ImplGlfw_InitForOpenGL(Application::Get().engine.window.Underlying().GetContext().glfwWinPtr, false);
	}
	Editor::~Editor()
	{
		ImGui_ImplGlfw_Shutdown();
	}
	Editor::Editor()
	{
		_commonData = &data;
		_editor = this;
		
		InitImgui();
		ImGui_ImplGlfw_InitForVulkan(Renderer::Vulkan::Window::Instance().glfwWindow, true);
		
		panels.push_back(std::make_shared<HierarchyPanel>());
		panels.push_back(std::make_shared<InspectorPanel>());
		panels.push_back(std::make_shared<PhysicsSettingsPanel>());
		panels.push_back(std::make_shared<GraphicsSettingsPanel>());
		panels.push_back(std::make_shared<BloomPanel>());
		panels.push_back(std::make_shared<ViewportPanel>());
		menus.push_back(std::make_unique<FileMenu>());
		ImGui::SetColorEditOptions(ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
	}
	Editor& Editor::Get()
	{
		return *_editor;
	}
	
	void Editor::Render()
	{
		//ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGui::DockSpace(3, ImVec2(100, 1000));
		if (ImGui::BeginMainMenuBar())
		{
			for (int i = 0; i < menus.size(); i++)
			{
				menus[i]->Draw();
			}
			DrawPanelManager();
			static Timer t;
			auto delta = t.Reset();
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Text(("Delta: " + std::to_string(delta) + " ms (" +
				std::to_string(1.0f / (delta / 1000.0f)) + " fps)").c_str());
			ImGui::EndMainMenuBar();
		}

		constexpr ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport(viewport, dockFlags);

		for (int i = 0; i < panels.size(); i++)
		{
			if (!panels[i]->Draw() && panels[i]->DeleteOnClose())
			{
				panels.erase(panels.begin() + i);
			}
		}

		// ImGui::Render();
		//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		if (Input::Instance().IsJustPressed(Key::F1))
		{
			data.gameMode = !data.gameMode;
			if (data.gameMode)
			{
				//glfwSetInputMode(Application::Get().engine.window.Underlying().GetContext().glfwWinPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				Engine::Get().StartPhysics();
			}
			else
			{
				//glfwSetInputMode(Application::Get().engine.window.Underlying().GetContext().glfwWinPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				Engine::Get().StopPhysics();
			}
		}
		ImGui::EndFrame();
	}
	void Editor::Update()
	{
		UpdateImguiInputs();
		Render();
		// engine.Update();
	}
	void Editor::UpdateImguiInputs()
	{
		// auto& imIO = ImGui::GetIO();
		// auto ptr = &imIO;
		auto& input = Input::Instance();
		// imIO.MouseWheel += (float)input.ScrollOffs();
		// 
		// imIO.KeyShift = input.IsDown(Key::Shift);
		// imIO.KeyCtrl = input.IsDown(Key::Ctrl);
		// imIO.KeyAlt = input.IsDown(Key::Alt);
		// imIO.KeySuper = input.IsDown(Key::Super);
		// for (size_t i = 0; i < input.KeysDown().size(); i++)
		// {
		// 	imIO.KeysDown[i] = input.KeysDown()[i];
		// }
		// 
		// for (auto c : input.TypedCharacters())
		// {
		// 	imIO.AddInputCharacter(c);
		// }

		if (input.IsJustPressed(Key::F2))
		{
			if (Engine::Get().physicsUpdate)
				Engine::Get().StopPhysics();
			else
				Engine::Get().StartPhysics(false);
		}
	}
	void Editor::DrawPanelManager()
	{
		if (ImGui::BeginMenu("Panels"))
		{
			for (auto& panel : panels)
			{
				if (!panel->panelState.isOpen)
				{
					if (ImGui::MenuItem(panel->GetName()))
					{
						panel->panelState.isOpen = true;
					}
				}
			}
			ImGui::EndMenu();
		}
	}
	void Editor::AddPanel(std::shared_ptr<Panel> panel)
	{
		panels.push_back(panel);
	}
}