#pragma once

#include "DeviceResource.h"
#include "Image.h"

#include "Data/Material.h"
#include "Data/Vertex.h"
#include "Data/Lights.h"

namespace Nork::Renderer {

struct DrawParams
{
	uint32_t mmIdx;
	uint32_t matIdx;
};
struct Texture
{
	std::shared_ptr<Image> image;
	const uint32_t descriptorIdx;
	~Texture();
};
class Resources
{
public:
	Resources();
	class TextureDesriptors
	{
	public:
		TextureDesriptors(std::shared_ptr<Vulkan::DescriptorSet>& descriptorSet);
		std::shared_ptr<Texture> AddTexture(std::shared_ptr<Image>& img);
		void RemoveTexture(uint32_t idx);
	public:
		std::shared_ptr<Vulkan::Sampler> defaultSampler;
		std::vector<std::shared_ptr<Image>> textures;
		std::set<uint32_t> freeTextureIdxs;
		std::shared_ptr<Vulkan::DescriptorSet> descriptorSet;
		std::shared_ptr<Texture> diffuse, normal, metallicRoughness; // default textures
	};
	uint32_t DynamicOffset(const Buffer& buffer)
	{
		return DynamicSize(buffer) * currentFrame;
	}
	vk::DeviceSize DynamicSize(const Buffer& buffer)
	{
		return buffer.memory.Size() / framesInFlight;
	}
	void OnNewFrame(uint32_t frameIdx)
	{
		currentFrame = frameIdx;
		modelMatrices->OnNewFrame();
		materials->OnNewFrame();
	}
	void FlushWrites()
	{
		indexBuffer->FlushWrites();
		vertexBuffer->FlushWrites();
		modelMatrices->FlushWrites();
		materials->FlushWrites();
		dirLights->FlushWrites();
		pointLights->FlushWrites();
		dirShadows->FlushWrites();
		pointShadows->FlushWrites();
	}
	TextureDesriptors& Textures()
	{
		return *textureDescriptors;
	}
public:
	// make type Descriptor, in it store params passed to desclayout, store these in descSet, 
	// write them through descSet, then flush writes just like in Buffer
	std::shared_ptr<DeviceArrays<uint32_t>> indexBuffer;
	std::shared_ptr<DeviceArrays<Data::Vertex>> vertexBuffer;
	uint32_t vbOffset = 0, ibOffset = 0; // abstract away these

	std::shared_ptr<DeviceElements<glm::mat4>> modelMatrices;
	std::shared_ptr<DeviceElements<Data::Material>> materials;
	std::shared_ptr<HostWritableBuffer> drawParams;
	std::shared_ptr<HostWritableBuffer> drawCommands;
	std::shared_ptr<DeviceElements<Data::DirLight>> dirLights;
	std::shared_ptr<DeviceElements<Data::DirShadow>> dirShadows;
	std::shared_ptr<DeviceElements<Data::PointLight>> pointLights;
	std::shared_ptr<DeviceElements<Data::PointShadow>> pointShadows;
	std::shared_ptr<HostWritableBuffer> dirLightParams;
	std::shared_ptr<HostWritableBuffer> pointLightParams;
	// eg. double-buffer only when light data is written
	// only d-buffing when written should be applied to drawParams/drawCommands, 
	// then it is not required to refill the buffers every frame 

	std::unique_ptr<TextureDesriptors> textureDescriptors;

	std::shared_ptr<Vulkan::DescriptorPool> descriptorPool;
	std::shared_ptr<Vulkan::DescriptorSetLayout> descriptorSetLayout;
	std::shared_ptr<Vulkan::DescriptorSet> descriptorSet;
	std::shared_ptr<Vulkan::DescriptorSetLayout> descriptorSetLayoutLights;
	std::shared_ptr<Vulkan::DescriptorSet> descriptorSetLights;

	glm::mat4 vp; // should be put somewhere else
	glm::vec3 viewPos; // should be put somewhere else
	uint32_t drawCommandCount;

	uint32_t currentFrame = 0;
	static constexpr auto framesInFlight = 2;
	static constexpr auto img_arr_size = 100;

	static Resources& Instance()
	{
		return *instance;
	}
private:
	static Resources* instance;
};
}