#include "Core/CameraController.h"
#include "Utils/Logger.h"
#include "Components/Common.h"
#include "Core/Engine.h"
#include "App/Application.h"

using namespace Nork;

Renderer::Vulkan::Device* Renderer::Vulkan::Device::instance = nullptr;
Renderer::Vulkan::PhysicalDevice* Renderer::Vulkan::PhysicalDevice::instance = nullptr;
Renderer::Vulkan::SwapChain* Renderer::Vulkan::SwapChain::instance = nullptr;
Renderer::Vulkan::Instance* Renderer::Vulkan::Instance::staticInstance = nullptr;
Renderer::Vulkan::Window* Renderer::Vulkan::Window::staticInstance = nullptr;
Renderer::MemoryAllocator* Renderer::MemoryAllocator::instance = nullptr;
Renderer::Resources* Renderer::Resources::instance = nullptr;
Renderer::Commands* Renderer::Commands::instance = nullptr;
Renderer::MemoryTransfer* Renderer::MemoryTransfer::instance = nullptr;
Renderer::Vulkan::DescriptorSet::Writer_ Renderer::Vulkan::DescriptorSet::Writer()
{
	return Writer_(*this);
}