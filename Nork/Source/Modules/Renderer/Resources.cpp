#include "Resources.h"
#include "ShadowMapPass.h"

namespace Nork::Renderer {
static constexpr auto vb_count = MemoryAllocator::poolSize / sizeof(Data::Vertex);
static constexpr auto ib_count = MemoryAllocator::poolSize / sizeof(uint32_t);
static constexpr auto mm_count = MemoryAllocator::poolSize / sizeof(glm::mat4);
static constexpr auto mat_count = MemoryAllocator::poolSize / sizeof(Data::Material);
static constexpr auto dl_count = 10; // !account for the number of frames too!
static constexpr auto ds_count = 10;
static constexpr auto pl_count = 100;
static constexpr auto ps_count = 100;
static constexpr auto sm_count = ds_count + ps_count;
static constexpr auto dlp_size = MemoryAllocator::poolSize / 4; // TODO: should be less...
static constexpr auto plp_size = MemoryAllocator::poolSize / 4;
static constexpr auto dps_size = MemoryAllocator::poolSize / 4;
static constexpr auto dcs_size = MemoryAllocator::poolSize / 4;
Texture::~Texture()
{
	Resources::Instance().textureDescriptors->RemoveTexture(descriptorIdx);
}
Resources::Resources()
{
	instance = this;

	vertexBuffer = std::make_shared<DeviceArrays<Data::Vertex>>(
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		MemoryFlags{ .required = MemoryFlags::eDeviceLocal }, vb_count, framesInFlight,
		vk::PipelineStageFlagBits2::eVertexInput, vk::AccessFlagBits2::eVertexAttributeRead);
	indexBuffer = std::make_shared<DeviceArrays<uint32_t>>(
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		MemoryFlags{ .required = MemoryFlags::eDeviceLocal }, ib_count, framesInFlight,
		vk::PipelineStageFlagBits2::eIndexInput, vk::AccessFlagBits2::eIndexRead);

	modelMatrices = std::make_shared<DeviceElements<glm::mat4>>(
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
		MemoryFlags{ .required = MemoryFlags::eDeviceLocal }, mm_count, framesInFlight,
		vk::PipelineStageFlagBits2::eVertexShader, vk::AccessFlagBits2::eShaderStorageRead);
	materials = std::make_shared<DeviceElements<Data::Material>>(
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
		MemoryFlags{ .required = MemoryFlags::eDeviceLocal }, mat_count, framesInFlight,
		vk::PipelineStageFlagBits2::eVertexShader, vk::AccessFlagBits2::eShaderStorageRead);

	auto lightStage = vk::PipelineStageFlagBits2::eFragmentShader;
	dirLights = std::make_shared<DeviceElements<Data::DirLight>>(
		vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
		MemoryFlags{ .required = MemoryFlags::eDeviceLocal }, dl_count, framesInFlight,
		lightStage, vk::AccessFlagBits2::eUniformRead);
	dirShadows = std::make_shared<DeviceElements<Data::DirShadow>>(
		vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
		MemoryFlags{ .required = MemoryFlags::eDeviceLocal }, ds_count, framesInFlight,
		lightStage, vk::AccessFlagBits2::eUniformRead);
	pointLights = std::make_shared<DeviceElements<Data::PointLight>>(
		vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
		MemoryFlags{ .required = MemoryFlags::eDeviceLocal }, pl_count, framesInFlight,
		lightStage, vk::AccessFlagBits2::eUniformRead);
	pointShadows = std::make_shared<DeviceElements<Data::PointShadow>>(
		vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
		MemoryFlags{ .required = MemoryFlags::eDeviceLocal }, ps_count, framesInFlight,
		lightStage, vk::AccessFlagBits2::eUniformRead);

	using enum vk::MemoryPropertyFlagBits;
	dirLightParams = Buffer::CreateHostWritable(
		vk::BufferUsageFlagBits::eUniformBuffer, dcs_size,
		{ .required = eHostVisible | eHostCoherent | eDeviceLocal });
	pointLightParams = Buffer::CreateHostWritable(
		vk::BufferUsageFlagBits::eUniformBuffer, dps_size,
		{ .required = eHostVisible | eHostCoherent | eDeviceLocal });

	drawParams = Buffer::CreateHostWritable(
		vk::BufferUsageFlagBits::eUniformBuffer, dps_size,
		{ .required = eHostVisible | eHostCoherent | eDeviceLocal });
	drawCommands = Buffer::CreateHostWritable(
		vk::BufferUsageFlagBits::eIndirectBuffer, dcs_size,
		{ .required = eHostVisible | eHostCoherent | eDeviceLocal });

	descriptorSetLayout = std::make_shared<Vulkan::DescriptorSetLayout>(Vulkan::DescriptorSetLayoutCreateInfo()
		.Binding(0, vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eVertex)
		.Binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
		.Binding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
		.Binding(3, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, img_arr_size, true));
	descriptorSetLayoutLights = std::make_shared<Vulkan::DescriptorSetLayout>(Vulkan::DescriptorSetLayoutCreateInfo()
		.Binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
		.Binding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
		.Binding(2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
		.Binding(3, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
		.Binding(4, vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eFragment)
		.Binding(5, vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eFragment)
		.Binding(6, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, sm_count, true)
	);
	descriptorPool = std::make_shared<Vulkan::DescriptorPool>(
		Vulkan::DescriptorPoolCreateInfo({ descriptorSetLayout, descriptorSetLayoutLights }, 2));
	descriptorSet = std::make_shared<Vulkan::DescriptorSet>( // could be less than img_arr_size, create bigger descriptor set on-demand from layout
		Vulkan::DescriptorSetAllocateInfo(descriptorPool, descriptorSetLayout, { img_arr_size }));
	descriptorSetLights = std::make_shared<Vulkan::DescriptorSet>(
		Vulkan::DescriptorSetAllocateInfo(descriptorPool, descriptorSetLayoutLights, { sm_count }));

	descriptorSet->Writer()
		.Buffer(0, *drawParams->Underlying(), 0, DynamicSize(*drawParams), vk::DescriptorType::eUniformBufferDynamic)
		.Buffer(1, *modelMatrices->buffer->Underlying(), 0, modelMatrices->buffer->memory.Size(), vk::DescriptorType::eStorageBuffer)
		.Buffer(2, *materials->buffer->Underlying(), 0, materials->buffer->memory.Size(), vk::DescriptorType::eStorageBuffer)
		.Write();

	descriptorSetLights->Writer()
		.Buffer(0, *dirLights->buffer->Underlying(), 0, dirLights->buffer->memory.Size(), vk::DescriptorType::eUniformBuffer)
		.Buffer(1, *dirShadows->buffer->Underlying(), 0, dirShadows->buffer->memory.Size(), vk::DescriptorType::eUniformBuffer)
		.Buffer(2, *pointLights->buffer->Underlying(), 0, pointLights->buffer->memory.Size(), vk::DescriptorType::eUniformBuffer)
		.Buffer(3, *pointShadows->buffer->Underlying(), 0, pointShadows->buffer->memory.Size(), vk::DescriptorType::eUniformBuffer)
		.Buffer(4, *dirLightParams->Underlying(), 0, DynamicSize(*dirLightParams), vk::DescriptorType::eUniformBufferDynamic)
		.Buffer(5, *pointLightParams->Underlying(), 0, DynamicSize(*pointLightParams), vk::DescriptorType::eUniformBufferDynamic)
		.Write();

	textureDescriptors = std::make_unique<TextureDesriptors>(descriptorSet);
}
// TODO: should be handled same way as textures
std::shared_ptr<ShadowMap> Resources::CreateShadowMap2D(uint32_t width, uint32_t height)
{
	auto idx = shadowMaps.size();
	auto shadowMap = ShadowMapPass::Instance().CreateShadowMap2D(width, height);
	shadowMap->shadow = std::make_shared<DeviceDataProxy<Data::DirShadow>>(dirShadows->New());
	shadowMap->SetIndex(idx);
	shadowMaps.push_back(shadowMap);

	if (!shadowMap->image->sampler)
		shadowMap->image->sampler = Textures().defaultSampler;

	descriptorSetLights->Writer()
		.Image(6, *shadowMap->image->view, vk::ImageLayout::eShaderReadOnlyOptimal,
			*shadowMap->image->sampler, vk::DescriptorType::eCombinedImageSampler, idx)
		.Write();

	return shadowMap;
}

// ----------------- TEXTURES ----------------

Resources::TextureDesriptors::TextureDesriptors(std::shared_ptr<Vulkan::DescriptorSet>& descriptorSet)
	: descriptorSet(descriptorSet)
{
	defaultSampler = std::make_shared<Vulkan::Sampler>();
	for (size_t i = 0; i < img_arr_size; i++)
	{
		freeTextureIdxs.insert(i);
	}
	textures.resize(img_arr_size);

	auto createTexture = [&](vk::Format format, const std::vector<float>& data)
	{
		auto texImg = std::make_shared<Image>(1, 1, format,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
			vk::PipelineStageFlagBits2::eVertexShader, vk::AccessFlagBits2::eShaderSampledRead);

		texImg->Write(data.data(), data.size() * sizeof(data[0]), vk::ImageLayout::eShaderReadOnlyOptimal);
		return AddTexture(texImg);
	};
	diffuse = createTexture(Vulkan::Format::rgba32f, { 1.0f, 1.0f, 1.0f, 1.0f });
	normal = createTexture(Vulkan::Format::rgba32f, { 0.5f, 0.5f, 1.0f, 0.0f }); // 'a' unused
	metallicRoughness = createTexture(Vulkan::Format::rgba32f, { 0.0f, 1.0f, 1.0f, 0.0f }); // g=roughness, b=metallic
}
std::shared_ptr<Texture> Resources::TextureDesriptors::AddTexture(std::shared_ptr<Image>& img)
{
	uint32_t idx = *freeTextureIdxs.begin();
	freeTextureIdxs.erase(freeTextureIdxs.begin());
	if (!img->sampler)
		img->sampler = defaultSampler;

	descriptorSet->Writer()
		.Image(3, *img->view, vk::ImageLayout::eShaderReadOnlyOptimal,
			*img->sampler, vk::DescriptorType::eCombinedImageSampler, idx)
		.Write();
	textures[idx] = img;
	return std::make_shared<Texture>(img, idx);
}
void Resources::TextureDesriptors::RemoveTexture(uint32_t idx)
{
	textures[idx] = nullptr;
	freeTextureIdxs.insert(idx);
}
}