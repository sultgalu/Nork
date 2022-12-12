#pragma once

#include "Buffer.h"

class DescriptorSetLayout
{
public:
	class Builder;
	DescriptorSetLayout(const DescriptorSetLayout&) = delete; // no move constructor will be declared implicitly
	DescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> bindings)
		: bindings(bindings)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		vkCreateDescriptorSetLayout(Device::Instance().device, &layoutInfo, nullptr, &handle) == VkSuccess();
	}
	~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(Device::Instance().device, handle, nullptr);
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
	Self& Binding(uint32_t idx, VkDescriptorType type, VkShaderStageFlags shaderStage)
	{
		bindings.push_back(VkDescriptorSetLayoutBinding{
			.binding = idx,
			.descriptorType = type,
			.descriptorCount = 1,
			.stageFlags = shaderStage,
			.pImmutableSamplers = nullptr,
			});
		return *this;
	}
	std::shared_ptr<DescriptorSetLayout> Build()
	{
		return std::make_shared<DescriptorSetLayout>(bindings);
	}
public:
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};

class DescriptorPool
{
public:
	DescriptorPool(const DescriptorSetLayout& layout, uint32_t maxSets = 1)
	{
		std::vector<VkDescriptorPoolSize> descriptorCounts;
		descriptorCounts.reserve(layout.bindings.size());
		for (auto& binding : layout.bindings)
		{
			descriptorCounts.push_back(VkDescriptorPoolSize{ .type = binding.descriptorType, .descriptorCount = maxSets });
		}
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = descriptorCounts.size();
		poolInfo.pPoolSizes = descriptorCounts.data();
		poolInfo.maxSets = maxSets;

		vkCreateDescriptorPool(Device::Instance().device, &poolInfo, nullptr, &handle) == VkSuccess();
	}
	~DescriptorPool()
	{
		vkDestroyDescriptorPool(Device::Instance().device, handle, nullptr);
	}
public:
	VkDescriptorPool handle;
};

class DescriptorSet
{
public:
	class Writer_;
	DescriptorSet(const DescriptorPool& pool, const DescriptorSetLayout& layout)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool.handle;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout.handle;

		vkAllocateDescriptorSets(Device::Instance().device, &allocInfo, &handle) == VkSuccess();
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
		uint32_t bindIdx;
		VkDescriptorImageInfo info;
		VkDescriptorType type;
	};
	Self& Image(uint32_t bindIdx, Image& img, VkImageLayout layout, VkDescriptorType type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
	{
		images.push_back(Image_{
			.bindIdx = bindIdx,
			.info = VkDescriptorImageInfo{
				.sampler = img.Sampler()->handle,
				.imageView = img.ImageView(),
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
			write.dstArrayElement = 0;
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
			else if (buf.buf.bufferInfo.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
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

		vkUpdateDescriptorSets(Device::Instance().device, writes.size(), writes.data(), 0, nullptr);
		return dSet;
	}
public:
	DescriptorSet& dSet;
	std::vector<Buffer_> buffers;
	std::vector<Image_> images;
};

DescriptorSet::Writer_ DescriptorSet::Writer()
{
	return Writer_(*this);
}