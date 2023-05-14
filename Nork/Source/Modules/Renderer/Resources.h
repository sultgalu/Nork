#pragma once

#include "DeviceData.h"

#include "Data/Material.h"
#include "Data/Vertex.h"
#include "Data/Lights.h"
#include "Model/ShadowMap.h"
#include "Model/Object.h"
#include "ImageDescriptorArray.h"

namespace Nork::Renderer {

struct DrawCounts {
	uint32_t defaults = 0;
	uint32_t blend = 0;
	uint32_t unlit = 0;
	uint32_t BlendOffs() {
		return defaults;
	}
	uint32_t UnlitOffs() {
		return defaults + blend;
	}
	uint32_t AllCount() {
		return defaults + blend + unlit;
	}
};
struct DrawParams
{
	uint32_t mmIdx;
	uint32_t matIdx;
};

class Resources
{
public:
	Resources();
	class TextureDescriptors: public ImageDescriptorArray
	{
	public:
		TextureDescriptors(std::shared_ptr<Vulkan::DescriptorSet>& descriptorSet);
		std::shared_ptr<Texture> AddTexture(std::shared_ptr<Image>& img);
	private:
		using ImageDescriptorArray::AddImage;
	public:
		std::shared_ptr<Vulkan::Sampler> defaultSampler;
		std::shared_ptr<Texture> diffuse, normal, metallicRoughness, occlusion, emissive; // default textures
	};
	uint32_t DynamicOffset(const Buffer& buffer)
	{
		return DynamicSize(buffer) * currentFrame;
	}
	vk::DeviceSize DynamicSize(const Buffer& buffer)
	{
		return buffer.memory.Size() / framesInFlight;
	}
	void OnNewFrame(uint32_t frameIdx);
	TextureDescriptors& Textures()
	{
		return *textureDescriptors;
	}
	std::shared_ptr<DirShadowMap> CreateShadowMap2D(uint32_t width, uint32_t height);
	std::shared_ptr<PointShadowMap> CreateShadowMapCube(uint32_t size);
	std::shared_ptr<MeshData> CreateMesh(uint32_t vertexCount, uint32_t indexCount);
	std::shared_ptr<MeshData> CreateMesh(const std::vector<Data::Vertex>& vertices, const std::vector<uint32_t> indices);
	std::shared_ptr<Material> CreateMaterial();
	std::shared_ptr<Object> CreateObject();
public:
	// make type Descriptor, in it store params passed to desclayout, store these in descSet, 
	// write them through descSet, then flush writes just like in Buffer
	std::shared_ptr<DeviceArrays<uint32_t>> indexBuffer;
	std::shared_ptr<DeviceArrays<Data::Vertex>> vertexBuffer;

	std::shared_ptr<DeviceElements<glm::mat4>> modelMatrices;
	std::shared_ptr<DeviceElements<Data::Material>> materials;
	std::shared_ptr<HostVisibleBuffer> drawParams;
	std::shared_ptr<HostVisibleBuffer> drawCommands;
	std::shared_ptr<DeviceElements<Data::DirLight>> dirLights;
	std::shared_ptr<DeviceElements<Data::DirShadow>> dirShadows;
	std::shared_ptr<DeviceElements<Data::PointLight>> pointLights;
	std::shared_ptr<DeviceElements<Data::PointShadow>> pointShadows;
	std::shared_ptr<HostVisibleBuffer> dirLightParams;
	std::shared_ptr<HostVisibleBuffer> pointLightParams;
	std::shared_ptr<HostVisibleBuffer> pShadowVps;
	// eg. double-buffer only when light data is written
	// only d-buffing when written should be applied to drawParams/drawCommands, 
	// then it is not required to refill the buffers every frame 

	std::unique_ptr<TextureDescriptors> textureDescriptors;
	std::unique_ptr<ImageDescriptorArray> dirShadowDescriptors;
	std::unique_ptr<ImageDescriptorArray> pointShadowDescriptors;
	std::vector<std::shared_ptr<DirShadowMap>> shadowMaps; // should be elsewhere, it gets filled by r.system every frame
	std::vector<std::shared_ptr<PointShadowMap>> shadowMapsCube; // same
	std::vector<std::shared_ptr<Object>> objects;

	std::shared_ptr<Vulkan::DescriptorPool> descriptorPool;
	std::shared_ptr<Vulkan::DescriptorSetLayout> descriptorSetLayout;
	std::shared_ptr<Vulkan::DescriptorSet> descriptorSet;
	std::shared_ptr<Vulkan::DescriptorSetLayout> descriptorSetLayoutLights;
	std::shared_ptr<Vulkan::DescriptorSet> descriptorSetLights;

	glm::mat4 vp; // should be put somewhere else
	glm::vec3 viewPos; // same
	DrawCounts drawCommandCount;

	uint32_t currentFrame = 0;
	static constexpr auto framesInFlight = 2;

	static Resources& Instance()
	{
		return *instance;
	}
private:
	static Resources* instance;
};
}