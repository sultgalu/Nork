#include "pch.h"
#include "Editor.h"
#include "App/Application.h"
#include "Panels/include/All.h"
#include "Menus/include/All.h"
#include "Modules/Renderer/Vulkan/Window.h"
#include "Main/Framebuffer.h"

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
		auto arr = root.Get<JsonArray>("cameras");
		for (auto& json : arr.Get<JsonObject>())
		{
			editor.AddViewportPanel();
			auto& comp = *data.editorCameras.back();
			json.Get<JsonArray>("position").Get(&comp.position.x, 3);
			json.Get<JsonArray>("up").Get(&comp.up.x, 3);
			json.Get("nearClip", comp.nearClip);
			json.Get("farClip", comp.farClip);
			json.Get("yaw", comp.yaw);
			json.Get("pitch", comp.pitch);
			json.Get("FOV", comp.FOV);
			json.Get("ratio", comp.ratio);
			json.Get("zoomSpeed", comp.zoomSpeed);
			json.Get("moveSpeed", comp.moveSpeed);
			json.Get("rotationSpeed", comp.rotationSpeed);
			
			comp.Update();
		}
	}
	static void WriteImGuiIniFile(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf)
	{
		auto& data = _GetCommonData();
		JsonArray cameras;
		for (auto& cam : data.editorCameras)
		{
			auto& component = *cam;
			auto jsonCam = JsonObject()
				.Property("position", JsonArray().Elements(&component.position.x, 3))
				.Property("up", JsonArray().Elements(&component.up.x, 3))
				.Property("farClip", component.farClip)
				.Property("FOV", component.FOV)
				.Property("moveSpeed", component.moveSpeed)
				.Property("nearClip", component.nearClip)
				.Property("pitch", component.pitch)
				.Property("ratio", component.ratio)
				.Property("rotationSpeed", component.rotationSpeed)
				.Property("yaw", component.yaw)
				.Property("zoomSpeed", component.zoomSpeed);
			cameras.Element(jsonCam);
		}
		out_buf->appendf("[Cameras][stuff]\n");
		out_buf->append(JsonObject().Property("cameras", cameras).ToString().c_str());
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
	void InitImguiForVulkanAndGlfw();
	Editor::Editor()
	{
		_commonData = &data;
		_editor = this;

		InitImgui();
		InitImguiForVulkanAndGlfw();
		panels.push_back(std::make_shared<HierarchyPanel>());
		panels.push_back(std::make_shared<InspectorPanel>());
		panels.push_back(std::make_shared<PhysicsSettingsPanel>());
		panels.push_back(std::make_shared<GraphicsSettingsPanel>());
		panels.push_back(std::make_shared<BloomPanel>());
		// AddViewportPanel();
		menus.push_back(std::make_unique<FileMenu>());
		ImGui::SetColorEditOptions(ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
	}
	Editor& Editor::Get()
	{
		return *_editor;
	}
	void Editor::createRenderPassUI()
	{
		uint32_t colAtt = 0, depthAtt = 1;
		RenderPass::Config config(1, 1);
		config.Attachment(colAtt, AttachmentDescription::ColorForLaterCopy((VkFormat)Format::rgba8Unorm, true));

		auto sPass = SubPass(0)
			.ColorAttachment(colAtt);

		config.AddSubPass(sPass);

		config.DependencyExternalSrc(sPass, SubPassDependency()
			.SrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
			.DstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
			.SrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
			.DstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT));

		renderPassUI = std::make_shared<RenderPass>(config);
	}
	void Editor::InitImguiForVulkanAndGlfw()
	{
		textureSampler = std::make_shared<Sampler>();
		auto w = SwapChain::Instance().Width();
		auto h = SwapChain::Instance().Height();
		{
			using enum vk::ImageUsageFlagBits;
			auto fbColor_ = std::make_shared<Image>(ImageCreateInfo(w, h, Format::rgba8Unorm, eColorAttachment | eInputAttachment | eTransferSrc | eSampled),
				vk::MemoryPropertyFlagBits::eDeviceLocal);
			fbColor = std::make_shared<ImageView>(ImageViewCreateInfo(fbColor_, vk::ImageAspectFlagBits::eColor), textureSampler);
		}

		createRenderPassUI();
		
		auto uiImg = std::make_shared<Image>(ImageCreateInfo(w, h, Format::rgba8Unorm, 
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc), 
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		auto uiImgView = std::make_shared<ImageView>(ImageViewCreateInfo(uiImg, vk::ImageAspectFlagBits::eColor), nullptr);
		fbUI = std::make_shared<Framebuffer>(w, h, *renderPassUI, std::vector<std::shared_ptr<ImageView>>{ uiImgView });
		
		commandPool = std::make_shared<CommandPool>();

		using namespace Nork::Renderer::Vulkan;
		//1: create descriptor pool for IMGUI
		// the size of the pool is very oversize, but it's copied from imgui demo itself.
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		vkCreateDescriptorPool(*Device::Instance(), &pool_info, nullptr, &imguiPool);

		// 2: initialize imgui library

		//this initializes the core structures of imgui
		// ImGui::CreateContext();

		//this initializes imgui for SDL
		ImGui_ImplGlfw_InitForVulkan(Window::Instance().glfwWindow, true);
		// ImGui_ImplSDL2_InitForVulkan(_window);

		//this initializes imgui for Vulkan
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = *Instance::StaticInstance();
		init_info.PhysicalDevice = *PhysicalDevice::Instance();
		init_info.Device = *Device::Instance();
		init_info.Queue = *Device::Instance().graphicsQueue;
		init_info.DescriptorPool = imguiPool;
		init_info.MinImageCount = 3;
		init_info.ImageCount = 3;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Subpass = 0;

		ImGui_ImplVulkan_Init(&init_info, renderPassUI->handle);

		//execute a gpu command to upload imgui font textures
		auto cmdBuf = CommandBuilder(*commandPool)
			.BeginCommands();
		ImGui_ImplVulkan_CreateFontsTexture(cmdBuf.cmdBuf.handle);
		cmdBuf.EndCommands();
		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		vk::CommandBuffer cmdbuf = cmdBuf.cmdBuf.handle;
		submitInfo.pCommandBuffers = &cmdbuf;
		Device::Instance().graphicsQueue.submit(submitInfo);
		Device::Instance().graphicsQueue.waitIdle();
		commandPool->FreeCommandBuffer(cmdBuf.cmdBuf);

		//could use smt like below
		//immediate_submit([&](VkCommandBuffer cmd)
		//    {
		//    });

		//clear font textures from cpu data
		ImGui_ImplVulkan_DestroyFontUploadObjects();

		viewportImgDs = ImGui_ImplVulkan_AddTexture(textureSampler->handle, **fbColor, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	void Editor::RenderPassUI(CommandBuilder& builder)
	{
		auto rpb = builder.BeginRenderPass(*fbUI, *renderPassUI);
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), builder.cmdBuf.handle);
		rpb.EndRenderPass();
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
		auto& imIO = ImGui::GetIO();
		auto ptr = &imIO;
		auto& input = Input::Instance();
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
			if (ImGui::MenuItem("New Viewport"))
			{
				AddViewportPanel();
			}
			ImGui::EndMenu();
		}
	}
	void Editor::AddViewportPanel()
	{
		auto panel = std::make_shared<ViewportPanel>();
		panel->SetIndex(viewportPanelCount++);
		panels.push_back(std::move(panel));
	}
	void Editor::AddPanel(std::shared_ptr<Panel> panel)
	{
		panels.push_back(panel);
	}
}