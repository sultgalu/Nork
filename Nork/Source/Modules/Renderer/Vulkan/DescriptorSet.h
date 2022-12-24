#pragma once

#include "Image.h"
#include "Buffer.h"

namespace Nork::Renderer::Vulkan {
	struct DescriptorSetLayoutCreateInfo : vk::DescriptorSetLayoutCreateInfo
	{
		DescriptorSetLayoutCreateInfo(DescriptorSetLayoutCreateInfo&&) = default;
		DescriptorSetLayoutCreateInfo()
		{
			setPNext(&bindingFlagsCreateInfo);
		}
		DescriptorSetLayoutCreateInfo& Binding(uint32_t idx, vk::DescriptorType type, vk::ShaderStageFlags shaderStage, uint32_t arraySize = 1, bool bindless = false)
		{
			bindings.push_back(vk::DescriptorSetLayoutBinding(idx, type, arraySize, shaderStage));
			bindingFlags.push_back(bindless ? bindlessFlagBits : (vk::DescriptorBindingFlagsEXT)0);
			UpdateArrayPointers();
			return *this;
		}
	private:
		void UpdateArrayPointers()
		{
			setBindings(bindings);
			bindingFlagsCreateInfo.setBindingFlags(bindingFlags);
		}
	public:
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		std::vector<vk::DescriptorBindingFlagsEXT> bindingFlags;
		vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo;
		static constexpr vk::DescriptorBindingFlagsEXT bindlessFlagBits =
			vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
			vk::DescriptorBindingFlagBits::ePartiallyBound;
			// vk::DescriptorBindingFlagBits::eUpdateAfterBind |
			// vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending;
	};
	class DescriptorSetLayout: public vk::raii::DescriptorSetLayout
	{
	public:
		DescriptorSetLayout(DescriptorSetLayoutCreateInfo& createInfo)
			: vk::raii::DescriptorSetLayout(Device::Instance(), createInfo),
			createInfo(std::move(createInfo))
		{}
	public:
		DescriptorSetLayoutCreateInfo createInfo;
	};

	struct DescriptorPoolCreateInfo: vk::DescriptorPoolCreateInfo
	{
		DescriptorPoolCreateInfo(DescriptorPoolCreateInfo&&) = default;
		DescriptorPoolCreateInfo(const std::vector<vk::DescriptorPoolSize> poolSizes, uint32_t maxSets)
			: poolSizes(poolSizes)
		{
			setPoolSizes(this->poolSizes);
			setMaxSets(maxSets);
			setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
		}
		std::vector<vk::DescriptorPoolSize> poolSizes;
	};

	class DescriptorPool: public vk::raii::DescriptorPool
	{
	public:
		DescriptorPool(DescriptorPoolCreateInfo&& createInfo)
			: vk::raii::DescriptorPool(Device::Instance(), createInfo),
			createInfo(std::move(createInfo))
		{}
		std::vector<vk::DescriptorSet> AllocateDescriptorSets(const vk::DescriptorSetAllocateInfo& allocInfo)
		{
			auto sets = Device::Instance().allocateDescriptorSets(allocInfo);
			std::vector<vk::DescriptorSet> handles;
			for (auto& set : sets)
				handles.push_back(set.release());
			return handles;
		}
	public:
		DescriptorPoolCreateInfo createInfo;
	};

	struct DescriptorSetAllocateInfo : vk::DescriptorSetAllocateInfo
	{
		DescriptorSetAllocateInfo(DescriptorSetAllocateInfo&&) = default;
		DescriptorSetAllocateInfo(const std::shared_ptr<DescriptorPool>& pool, 
			const std::shared_ptr<DescriptorSetLayout>& layout, uint32_t dynamicDescSize = 0)
			: pool(pool), layout(layout)
		{
			this->descriptorPool = **pool;
			this->descriptorSetCount = 1;
			setSetLayouts(**layout);

			if (dynamicDescSize != 0)
			{
				variableDescInfo.descriptorSetCount = 1;
				variableDescInfo.pDescriptorCounts = &dynamicDescSize;
				setPNext(&variableDescInfo);
			}
		}
		std::shared_ptr<DescriptorPool> pool;
		std::shared_ptr<DescriptorSetLayout> layout;
		vk::DescriptorSetVariableDescriptorCountAllocateInfo variableDescInfo;
	};

	class DescriptorSet: public vk::raii::DescriptorSet
	{
	public:
		class Writer_;
		DescriptorSet(DescriptorSetAllocateInfo&& createInfo)
			: vk::raii::DescriptorSet(Device::Instance(), createInfo.pool->AllocateDescriptorSets(createInfo)[0], **createInfo.pool),
			createInfo(std::move(createInfo))
		{}
		Writer_ Writer();
	public:
		DescriptorSetAllocateInfo createInfo;
	};

	class DescriptorSet::Writer_
	{
	public:
		using Self = Writer_;
		Writer_(DescriptorSet& dSet)
			: dSet(dSet)
		{}
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
					.sampler = **img.sampler,
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
				write.dstSet = *dSet;
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
				if (buf.buf.createInfo.usage & vk::BufferUsageFlagBits::eUniformBuffer)
					type = buf.dynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				else if (buf.buf.createInfo.usage & vk::BufferUsageFlagBits::eStorageBuffer)
					type = buf.dynamic ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				else
					std::unreachable();

				bufferInfos.push_back(VkDescriptorBufferInfo{
					.buffer = *buf.buf,
					.offset = buf.offset,
					.range = buf.size,
					});

				VkWriteDescriptorSet write{};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = *dSet;
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
}
