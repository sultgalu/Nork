#pragma once

#include "DeviceResource.h"
#include "Image.h"

#include "Data/Material.h"
#include "Data/Vertex.h"
#include "Data/Lights.h"
#include "Model/ShadowMap.h"

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
// TODO: make an abstraction around multiple buffers for the same type of resource(eg. Materials).
// when materials reach the maximum size of a buffer (DeviceMemory pool size), new buffers need to be created.
// store these buffers in a descriptor array (PartiallyBound feature), then the indexes that point to the materials
// should include the desc. array index too. 
// you have a 32 bit uint, use the upper 5 bits for the desc. idx, that leaves 27 bits for indexing into the buffer
// that gives you 134 mil. unique indexes, which is 512MB of indexable space if you use a single float for a resource,
// which is the smallest type you can use in a shader, and this is for a single buffer, so this should be enough
// it leaves you with a maximum number of 32 buffers in a descriptor array, should also be enough
class Resources
{
public:
	Resources();
	class ImageDescriptorArray
	{
	public:
		ImageDescriptorArray(std::shared_ptr<Vulkan::DescriptorSet>& descriptorSet, uint32_t descriptorIdx);
		uint32_t AddImage(std::shared_ptr<Image>& img);
		void RemoveImage(uint32_t idx);
	public:
		std::vector<std::shared_ptr<Image>> images;
		std::set<uint32_t> freeImageIdxs;
		std::shared_ptr<Vulkan::DescriptorSet> descriptorSet;
		uint32_t descriptorIdx;
	};
	class TextureDesriptors: public ImageDescriptorArray
	{
	public:
		TextureDesriptors(std::shared_ptr<Vulkan::DescriptorSet>& descriptorSet);
		std::shared_ptr<Texture> AddTexture(std::shared_ptr<Image>& img);
	private:
		using ImageDescriptorArray::AddImage;
	public:
		std::shared_ptr<Vulkan::Sampler> defaultSampler;
		std::shared_ptr<Texture> diffuse, normal, metallicRoughness; // default textures
	};
	class ShadowMapDesriptors : public ImageDescriptorArray
	{
	public:
		using ImageDescriptorArray::ImageDescriptorArray;
	public:
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
	std::shared_ptr<DirShadowMap> CreateShadowMap2D(uint32_t width, uint32_t height);
	std::shared_ptr<PointShadowMap> CreateShadowMapCube(uint32_t size);
public:
	// make type Descriptor, in it store params passed to desclayout, store these in descSet, 
	// write them through descSet, then flush writes just like in Buffer
	std::shared_ptr<DeviceArrays<uint32_t>> indexBuffer;
	std::shared_ptr<DeviceArrays<Data::Vertex>> vertexBuffer;
	uint32_t vbOffset = 0, ibOffset = 0; // abstract away these

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

	std::unique_ptr<TextureDesriptors> textureDescriptors;
	std::unique_ptr<ShadowMapDesriptors> dirShadowDescriptors;
	std::unique_ptr<ShadowMapDesriptors> pointShadowDescriptors;
	std::vector<std::shared_ptr<DirShadowMap>> shadowMaps; // should be elsewhere, it gets filled by r.system every frame
	std::vector<std::shared_ptr<PointShadowMap>> shadowMapsCube; // same

	std::shared_ptr<Vulkan::DescriptorPool> descriptorPool;
	std::shared_ptr<Vulkan::DescriptorSetLayout> descriptorSetLayout;
	std::shared_ptr<Vulkan::DescriptorSet> descriptorSet;
	std::shared_ptr<Vulkan::DescriptorSetLayout> descriptorSetLayoutLights;
	std::shared_ptr<Vulkan::DescriptorSet> descriptorSetLights;

	glm::mat4 vp; // should be put somewhere else
	glm::vec3 viewPos; // same
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