#pragma once

#include "Image.h"
#include "Buffer.h"

using namespace Nork::Renderer::Vulkan;
class DescriptorSetLayout
{
public:
	class Builder;
	DescriptorSetLayout(const DescriptorSetLayout&) = delete; // no move constructor will be declared implicitly
	DescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> bindings, const std::vector<bool>& bindlesses)
		: bindings(bindings)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		//layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = bindings.size();
		layoutInfo.pBindings = bindings.data();

		const VkDescriptorBindingFlagsEXT flags_ =
			VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | // VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
		std::vector<VkDescriptorBindingFlagsEXT> flags;
		for (size_t i = 0; i < bindings.size(); i++)
			flags.push_back(bindlesses[i] ? flags_ : 0);

		VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags = {};
		binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		binding_flags.bindingCount = flags.size();
		binding_flags.pBindingFlags = flags.data();
		layoutInfo.pNext = &binding_flags;

		vkCreateDescriptorSetLayout(*Device::Instance(), &layoutInfo, nullptr, &handle) == VkSuccess();
	}
	~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(*Device::Instance(), handle, nullptr);
	}
public:
	VkDescriptorSetLayout handle;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
private:
};

class DescriptorSetLayout::Builder
{
public:
	using Self = Builder;
	Self& Binding(uint32_t idx, VkDescriptorType type, VkShaderStageFlags shaderStage, uint32_t arraySize = 1, bool bindless = false)
	{
		bindings.push_back(VkDescriptorSetLayoutBinding{
			.binding = idx,
			.descriptorType = type,
			.descriptorCount = arraySize,
			.stageFlags = shaderStage,
			.pImmutableSamplers = nullptr,
			});
		bindlesses.push_back(bindless);
		return *this;
	}
	std::shared_ptr<DescriptorSetLayout> Build()
	{
		return std::make_shared<DescriptorSetLayout>(bindings, bindlesses);
	}
public:
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	std::vector<bool> bindlesses;
};

class DescriptorPool
{
public:
	DescriptorPool(const std::unordered_map<VkDescriptorType, uint32_t> descriptorCounts, uint32_t maxSets = 1)
	{
		std::vector<VkDescriptorPoolSize> poolSizes;
		poolSizes.reserve(descriptorCounts.size());
		for (auto& [type, count] : descriptorCounts)
		{
			poolSizes.push_back(VkDescriptorPoolSize{ .type = type, .descriptorCount = count });
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = maxSets;

		vkCreateDescriptorPool(*Device::Instance(), &poolInfo, nullptr, &handle) == VkSuccess();
	}
	DescriptorPool(const DescriptorSetLayout& layout, uint32_t maxSets = 1)
	{
		std::vector<VkDescriptorPoolSize> descriptorCounts;
		descriptorCounts.reserve(layout.bindings.size());
		for (auto& binding : layout.bindings)
		{
			descriptorCounts.push_back(VkDescriptorPoolSize{ .type = binding.descriptorType, .descriptorCount = binding.descriptorCount });
		}
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = descriptorCounts.size();
		poolInfo.pPoolSizes = descriptorCounts.data();
		poolInfo.maxSets = maxSets;

		vkCreateDescriptorPool(*Device::Instance(), &poolInfo, nullptr, &handle) == VkSuccess();
	}
	~DescriptorPool()
	{
		vkDestroyDescriptorPool(*Device::Instance(), handle, nullptr);
	}
public:
	VkDescriptorPool handle;
};

class DescriptorSet
{
public:
	class Writer_;
	DescriptorSet(const DescriptorPool& pool, const DescriptorSetLayout& layout, uint32_t dynamicDescSize = 0)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool.handle;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout.handle;

		if (dynamicDescSize != 0)
		{
			VkDescriptorSetVariableDescriptorCountAllocateInfo variable_info = {};
			variable_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
			variable_info.descriptorSetCount = 1;
			variable_info.pDescriptorCounts = &dynamicDescSize;
			allocInfo.pNext = &variable_info;
		}

		vkAllocateDescriptorSets(*Device::Instance(), &allocInfo, &handle) == VkSuccess();
	}
	Writer_ Writer();
public:
	VkDescriptorSet handle;
};

class DescriptorSet::Writer_
{
public:
	using Self = Writer_;
	Writer_(DescriptorSet& dSet)
		: dSet(dSet)
	{
	}
	struct Buffer_
	{
		uint32_t bindIdx;
		const Buffer& buf; 
		VkDeviceSize offset;
		VkDeviceSize size;
		bool dynamic;
	};
	Self& Buffer(uint32_t bindIdx, const Buffer& buf, VkDeviceSize offset, VkDeviceSize size, bool dynamic = false)
	{
		buffers.push_back(Buffer_{
			.bindIdx = bindIdx,
			.buf = buf,
			.offset = offset,
			.size = size,
			.dynamic = dynamic,
			});
		return *this;
	}
	struct Image_
	{
		uint32_t arrIdx;
		uint32_t bindIdx;
		VkDescriptorImageInfo info;
		VkDescriptorType type;
	};
	Self& Image(uint32_t bindIdx, ImageView& img, VkImageLayout layout, VkDescriptorType type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uint32_t arrIdx = 0)
	{
		images.push_back(Image_{
			.arrIdx = arrIdx,
			.bindIdx = bindIdx,
			.info = VkDescriptorImageInfo{
				.sampler = img.sampler->handle,
				.imageView = *img,
				.imageLayout = layout,
			},
			.type = type
			});
		return *this;
	}
	DescriptorSet& Write()
	{
		std::vector<VkWriteDescriptorSet> writes;
		writes.reserve(buffers.size() + images.size());
		std::vector<VkDescriptorImageInfo> imageInfos;
		imageInfos.reserve(images.size());
		std::vector<VkDescriptorBufferInfo> bufferInfos;
		bufferInfos.reserve(buffers.size());

		for (auto& img : images)
		{
			imageInfos.push_back(img.info);

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = dSet.handle;
			write.dstBinding = img.bindIdx;
			write.dstArrayElement = img.arrIdx;
			write.descriptorType = img.type;
			write.descriptorCount = 1;
			write.pImageInfo = &imageInfos.back();

			writes.push_back(write);
		}
		for (auto& buf : buffers)
		{
			VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
			if (buf.buf.bufferInfo.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
				type = buf.dynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			else if (buf.buf.bufferInfo.usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
				type = buf.dynamic ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			else
				std::unreachable();

			bufferInfos.push_back(VkDescriptorBufferInfo{
				.buffer = buf.buf.handle,
				.offset = buf.offset,
				.range = buf.size,
				});

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = dSet.handle;
			write.dstBinding = buf.bindIdx;
			write.dstArrayElement = 0;
			write.descriptorCount = 1;
			write.pBufferInfo = &bufferInfos.back();
			write.descriptorType = type;

			writes.push_back(write);
		}

		vkUpdateDescriptorSets(*Device::Instance(), writes.size(), writes.data(), 0, nullptr);
		return dSet;
	}
public:
	DescriptorSet& dSet;
	std::vector<Buffer_> buffers;
	std::vector<Image_> images;
};
