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
		static std::vector<vk::DescriptorPoolSize> DescriptorCounts(const std::vector<std::shared_ptr<Vulkan::DescriptorSetLayout>>& layouts)
		{
			std::vector<vk::DescriptorPoolSize> result;
			for (auto& layout : layouts)
				for (auto& binding : layout->createInfo.bindings)
					result.push_back({ binding.descriptorType, binding.descriptorCount });
			return result;
		}
		DescriptorPoolCreateInfo(DescriptorPoolCreateInfo&&) = default;
		DescriptorPoolCreateInfo(const std::vector<std::shared_ptr<Vulkan::DescriptorSetLayout>>& layouts, uint32_t maxSets)
			: DescriptorPoolCreateInfo(DescriptorCounts(layouts), maxSets)
		{}
		DescriptorPoolCreateInfo(const std::vector<vk::DescriptorPoolSize>& poolSizes, uint32_t maxSets)
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
			const std::shared_ptr<DescriptorSetLayout>& layout, std::vector<uint32_t> dynamicDescSizes = {})
			: pool(pool), layout(layout), dynamicDescSizes(dynamicDescSizes)
		{
			this->descriptorPool = **pool;
			this->descriptorSetCount = 1;
			setSetLayouts(**layout);

			if (!dynamicDescSizes.empty())
			{
				variableDescInfo.descriptorSetCount = this->dynamicDescSizes.size();
				variableDescInfo.pDescriptorCounts = this->dynamicDescSizes.data();
				setPNext(&variableDescInfo);
			}
		}
		std::shared_ptr<DescriptorPool> pool;
		std::shared_ptr<DescriptorSetLayout> layout;
		std::vector<uint32_t> dynamicDescSizes;
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
		Self& Buffer(uint32_t bindIdx, const Buffer& buf, vk::DeviceSize offset, vk::DeviceSize size, 
			vk::DescriptorType type, uint32_t arrIdx = 0)
		{
			buffers.push_back(vk::DescriptorBufferInfo(*buf, offset, size));
			bufferWrites.push_back(vk::WriteDescriptorSet()
				.setDstSet(*dSet).setDstBinding(bindIdx)
				.setDstArrayElement(arrIdx).setDescriptorCount(1)
				.setDescriptorType(type));
			return *this;
		}
		Self& Image(uint32_t bindIdx, ImageView& img, vk::ImageLayout layout, Sampler& sampler, 
			vk::DescriptorType type = vk::DescriptorType::eCombinedImageSampler, uint32_t arrIdx = 0)
		{
			images.push_back(vk::DescriptorImageInfo(*sampler, *img, layout));
			imageWrites.push_back(vk::WriteDescriptorSet()
				.setDstSet(*dSet).setDstBinding(bindIdx)
				.setDstArrayElement(arrIdx).setDescriptorCount(1)
				.setDescriptorType(type));
			return *this;
		}
		DescriptorSet& Write()
		{
			for (size_t i = 0; i < imageWrites.size(); i++)
			{
				imageWrites[i].setImageInfo(images[i]);
			}
			for (size_t i = 0; i < bufferWrites.size(); i++)
			{
				bufferWrites[i].setBufferInfo(buffers[i]);
			}

			auto writes = bufferWrites;
			writes.insert(writes.end(), imageWrites.begin(), imageWrites.end());
			Device::Instance().updateDescriptorSets(writes, {});
			return dSet;
		}
	public:
		DescriptorSet& dSet;
		std::vector<vk::WriteDescriptorSet> bufferWrites;
		std::vector<vk::WriteDescriptorSet> imageWrites;
		std::vector<vk::DescriptorBufferInfo> buffers;
		std::vector<vk::DescriptorImageInfo> images;
	};
}
