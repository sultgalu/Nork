#include "pch.h"
#include "Editor.h"
#include "App/Application.h"
#include "Panels/include/All.h"
#include "Menus/include/All.h"
#include "Modules/Renderer/Vulkan/Window.h"
#include "Platform/FileDialog.h"

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
		if (line == std::string(""))
			return;
		if ((const char*)entry == std::string("project")) {
			auto projectPath = fs::path(line);
			if (!projectPath.empty() || fs::exists(projectPath)) {
				Editor::Get().OpenProject(projectPath);
			}
		}
		else if ((const char*)entry == std::string("camera"))
		{
			JsonObject root;
			try
			{
				root = JsonObject::Parse(line);
			} catch (std::exception& e)
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

				auto controller = json.Get<JsonObject>("controller");
				if (controller.Contains("type")) {
					if (controller.Get<std::string>("type") == "editor") {
						glm::vec3 center;
						size_t focused = entt::null;
						controller.Get<JsonArray>("center").Get(center);
						controller.GetIfContains("focused", focused);

						auto cc = std::make_shared<EditorCameraController>(center);
						editor.GetPanel<ViewportPanel>()->viewportView.camController = cc;
						if (focused != entt::null) {
							if (auto node = Engine::Get().scene.GetNodeById((entt::entity)focused)) {
								editor.GetPanel<ViewportPanel>()->focusedNode = node;
							}
						}
					}
					else if (controller.Get<std::string>("type") == "fps") {
						auto& cam = editor.GetPanel<ViewportPanel>()->viewportView.camController = std::make_shared<FpsCameraController>();
					}
				}
			}
		}
	}
	static void WriteImGuiIniFile(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf)
	{
		auto& data = _GetCommonData();

		const auto& camController = Editor::Get().GetPanel<ViewportPanel>()->viewportView.camController;
		auto jsonCC = JsonObject();
		std::string ccType = "";
		if (auto cc = std::dynamic_pointer_cast<EditorCameraController>(camController)) {
			jsonCC
				.Property("type", "editor")
				.Property("center", JsonArray().Elements(cc->center));
			if (auto focusedNode = Editor::Get().GetPanel<ViewportPanel>()->focusedNode.lock()) {
				jsonCC.Property("focus", (size_t)focusedNode->GetEntity().Id());
			}
		}
		else if (auto cc = std::dynamic_pointer_cast<FpsCameraController>(camController)) {
			jsonCC
				.Property("type", "fps");
		}

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
			.Property("FOV", component.FOV)
			.Property("controller", jsonCC);

		out_buf->append("[Editor][camera]\n");
		out_buf->append(JsonObject().Property("camera", jsonCam).ToString().c_str());
		out_buf->append("\n[Editor][project]\n");
		if (!data.projectPath.empty()) {
			out_buf->append((data.projectPath / "project.nork").string().c_str());
		}
	}
	static void* OpenImGuiIniFile(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name)
	{
		return (void*)name;
	}
	void InitImgui()
	{
		ImGui::CreateContext();

		auto g = ImGui::GetCurrentContext();
		ImGuiSettingsHandler ini_handler;
		ini_handler.TypeName = "Editor";
		ini_handler.TypeHash = ImHashStr("Editor");
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
		io.Fonts->AddFontFromFileTTF("SchibstedGrotesk-Medium.ttf", 18);
		ImGui::StyleColorsDark();
		//ImGui_ImplOpenGL3_Init();
		//ImGui_ImplGlfw_InitForOpenGL(Application::Get().engine.window.Underlying().GetContext().glfwWinPtr, false);
	}
	Editor::~Editor()
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui::Shutdown();
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

	void Editor::BeforeEngineShutdown()
	{
		Engine::Get().scene.Save();
	}
	
	void Editor::BuildFrame()
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
		BuildFrame();
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
		static std::stringstream ss;
		if (input.IsJustPressed(Key::F2))
		{
			if (data.sceneRunning) {
				Engine::Get().StopPhysics();
				Engine::Get().scene.Deserialize(ss);
				ss.clear();
			}
			else {
				Engine::Get().scene.Serialize(ss);
				Engine::Get().StartPhysics(false);
			}
			data.sceneRunning = !data.sceneRunning;
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
	void Editor::CreateProject()
	{
		using namespace FileDialog;
		fs::path dir = FileDialog::OpenFile(EngineFileTypes::Folder, L"Select New Project Folder ", L"Select");
		if (dir.empty()){
			return;
		}
		AssetLoader::Instance().SetProjectRoot(dir);

		std::ofstream projectFile(dir / "project.nork");
		projectFile << JsonObject()
			.Property("startScene", "start_scene.json")
			.ToStringFormatted();

		fs::create_directory(dir / "scenes");
		Engine::Get().scene.Save();
		Engine::Get().scene.Create(dir / "scenes" / "start_scene.json");

		data.projectPath = dir;
		data.selectedNode = nullptr;
	}
	void Editor::OpenProject(const fs::path& projectFilePath)
	{
		auto projectFile = FileUtils::ReadAsString(projectFilePath.string());
		auto startScene = JsonObject::ParseFormatted(projectFile)
			.Get<std::string>("startScene");

		AssetLoader::Instance().SetProjectRoot(projectFilePath.parent_path()); // clears cache, put before scene load
		Engine::Get().scene.Load(projectFilePath.parent_path() / "scenes" / startScene);

		data.projectPath = projectFilePath.parent_path();
		data.selectedNode = nullptr;
	}
}