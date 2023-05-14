#pragma once

#include "Resources.h"
#include "RenderPass.h"
#include "Model/Mesh.h"

namespace Nork::Renderer {

struct Client
{
	// return the number of elements written to 'commands'
	virtual std::vector<Object> GetObjectsToDraw() = 0;
	virtual void FillLightBuffers(std::span<uint32_t> dIdxs, std::span<uint32_t> pIdxs,
		uint32_t& dlCount, uint32_t& dsCount, uint32_t& plCount, uint32_t& psCount) = 0;
	virtual void ProvideShadowMapsForUpdate(std::vector<std::shared_ptr<DirShadowMap>>&, std::vector<std::shared_ptr<PointShadowMap>>&) = 0;
	// do stuff that requires a commandBuffer
	virtual void OnCommands() = 0;
};

class Renderer
{
public:
	Renderer();
	~Renderer();
	uint32_t BeginFrame();
	void EndFrame(uint32_t imgIdx);
	void DrawFrame();
	void RefreshShaders();
	void CreateSyncObjects();
private:
	void SetupDrawBuffers(std::vector<Object>&& objects, std::span<DrawParams> params,
		std::span<vk::DrawIndexedIndirectCommand> commands);
public:
	MemoryAllocator allocator;
	Commands commands = Commands(MAX_FRAMES_IN_FLIGHT);
	MemoryTransfer transfer;
	std::unique_ptr<Resources> resources;
	std::vector<std::shared_ptr<DeviceData>> deviceDatas;
	std::vector<std::shared_ptr<RenderPass>> renderPasses;
	std::shared_ptr<Image> mainImage;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	uint32_t currentFrame = 0;
	bool framebufferResized = false;

	Client* client = nullptr;

	inline static uint32_t MAX_FRAMES_IN_FLIGHT = 2;

public:
	static Renderer& Instance()
	{
		return *instance;
	}
	static Renderer* instance;
};
}