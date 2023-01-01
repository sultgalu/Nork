#include "pch.h"
#include "Core/CameraController.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Components/Common.h"
#include "Editor/Editor.h"
#include "Core/Engine.h"
#include "App/Application.h"
#include "Modules/Renderer/Vulkan/Shaderc.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/Vulkan/Window.h"
#include "Modules/Renderer/Vulkan/Buffer.h"
#include "Modules/Renderer/Vulkan/DeviceMemory.h"
#include "Modules/Renderer/Vulkan/Image.h"
#include "Modules/Renderer/Vulkan/Sampler.h"
#include "Modules/Renderer/Vulkan/Pipeline.h"
#include "Modules/Renderer/Vulkan/DescriptorSet.h"
#include "Modules/Renderer/Vulkan/CommandBuffer.h"
#include "Editor/Panels/include/ViewportPanel.h"
#include "Modules/Renderer/Frame.h"
#include "Modules/Renderer/MemoryAllocator.h"
#include "Modules/Renderer/Resources.h"

using namespace Nork;
using namespace Nork::Renderer::Vulkan;

Device* Device::instance = nullptr;
PhysicalDevice* PhysicalDevice::instance = nullptr;
SwapChain* SwapChain::instance = nullptr;
Instance* Instance::staticInstance = nullptr;
Window* Window::staticInstance = nullptr;
Renderer::MemoryAllocator* Renderer::MemoryAllocator::instance = nullptr;
Renderer::Resources* Renderer::Resources::instance = nullptr;
Renderer::Commands* Renderer::Commands::instance = nullptr;
Renderer::MemoryTransfer* Renderer::MemoryTransfer::instance = nullptr;
DescriptorSet::Writer_ DescriptorSet::Writer()
{
    return Writer_(*this);
}

int main2()
{
    Logger::PushStream(std::cout);

    auto window = Renderer::Vulkan::Window(1920 * 0.8f, 1080 * 0.8f);
    std::unique_ptr<Nork::Input> input = std::make_unique<Nork::Input>(window.glfwWindow);
    Editor::Editor editor;
    Engine engine;

    Renderer::Demo::RenderLoop renderer;
    std::shared_ptr<ImageView> vpImg = nullptr;
    for (auto& pass : renderer.renderPasses)
    {
        if (auto dp = std::dynamic_pointer_cast<Renderer::Demo::DeferredPass>(pass))
        {
            vpImg = dp->fbColor->view;
            break;
        }
    }
    if (!vpImg)
        std::unreachable();
    auto sampler = std::make_shared<Renderer::Vulkan::Sampler>();
    try
    {
        Timer timer;
        uint32_t frames = 0;
        while (!glfwWindowShouldClose(Window::Instance().glfwWindow))
        {
            Input::Instance().Update();
            editor.Render();
            for (auto& panel : editor.panels) // laziness...
            {
                if (auto vpp = std::dynamic_pointer_cast<Editor::ViewportPanel>(panel))
                {
                    if (vpp->viewportView.image != vpImg)
                        vpp->viewportView.SetImage(vpImg, sampler);
                    break;
                }
            }
            renderer.DrawFrame();
            //drawFrame();
            frames++;
            auto seconds = timer.ElapsedSeconds();
            if (seconds > 1.0f)
            {
                auto fps = frames / seconds;
                Logger::Info(fps, " fps");
                timer = Timer();
                frames = 0;
            }
        }
        Device::Instance().waitIdle();
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}