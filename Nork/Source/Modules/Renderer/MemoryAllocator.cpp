#include "MemoryAllocator.h"

namespace Nork::Renderer {
    MemoryAllocator::MemoryAllocator(const Vulkan::PhysicalDevice& physicalDevice)
    {
        auto& memprops = physicalDevice.memProperties;
        for (auto& heap : memprops.memoryHeaps)
        {
            if (heap.size != 0)
                heaps.push_back(Heap{ .size = heap.size });
        }
        for (uint32_t i = 0; i < memprops.memoryTypeCount; i++)
        {
            auto& t = memprops.memoryTypes[i];
            if (!t.propertyFlags)
                continue;
            types.push_back(MemoryType{
                .index = i,
                .flags = t.propertyFlags,
                .heap = heaps[t.heapIndex],
                });
        }
        instance = this;
    }
    DeviceMemory MemoryAllocator::Allocate(vk::MemoryRequirements req,
        vk::MemoryPropertyFlags requiredFlags, const std::vector<vk::MemoryPropertyFlags>& desiredFlags)
    {
        auto allocation = std::make_shared<Allocation>();
        allocation->size = req.size;

        uint32_t suitableTypeBits = FindSuitableMemoryTypes(req.memoryTypeBits, requiredFlags, desiredFlags);
        allocation->suitableMemoryTypeBits = suitableTypeBits;

        uint32_t typeIdx = GetMemoryTypeWithMostAvailableMemory(allocation->suitableMemoryTypeBits);
        auto poolOpt = FindPool(typeIdx, allocation->size);
        std::shared_ptr<Pool> pool;
        if (poolOpt)
            pool = poolOpt.value();
        else // should check if memType has enough memory for another pool allocation
            // should also check if allocation->size is not greater than poolSize and handle if it is   
        {
            pool = std::make_shared<Pool>(types[typeIdx], poolSize);
            types[typeIdx].allocated += pool->size;
            types[typeIdx].pools.push_back(pool);
        }
        vk::DeviceSize alignmentOffset = req.alignment - pool->offset % req.alignment;
        if (alignmentOffset == req.alignment)
            alignmentOffset = 0;
        pool->offset += alignmentOffset;
        allocation->poolOffset = pool->offset;
        pool->offset += allocation->size;

        allocation->pool = pool;
        pool->allocations.push_back(allocation);
        return DeviceMemory(allocation);
    }
    std::optional<std::shared_ptr<MemoryAllocator::Pool>> MemoryAllocator::FindPool(uint32_t typeIndex, vk::DeviceSize size)
    {
        for (auto& pool : types[typeIndex].pools)
        {
            if (pool->size - pool->offset >= size)
                return pool;
        }
        return std::nullopt;
    }
    uint32_t MemoryAllocator::GetMemoryTypeWithMostAvailableMemory(uint32_t typeBits)
    {
        uint32_t result = 0;
        uint32_t mostMemory = 0;
        for (uint32_t i = 0; i < types.size(); i++)
        {
            if ((typeBits & (1 << i)) && types[i].heap.size > mostMemory)
            {
                result = i;
                mostMemory = types[i].heap.size;
            }
        }
        return result;
    }
    uint32_t MemoryAllocator::FindSuitableMemoryTypes(uint32_t typeBitsFilter, vk::MemoryPropertyFlags requiredFlags,
        const std::vector<vk::MemoryPropertyFlags>& desiredFlags)
    {
        uint32_t typeBitsRequired = GetMemoryTypeBits(typeBitsFilter, requiredFlags);
        std::vector<uint32_t> typeBitsVec;
        for (auto& flag : desiredFlags)
            typeBitsVec.push_back(GetMemoryTypeBits(typeBitsFilter, flag) & typeBitsRequired);
        uint32_t suitableTypeBits = typeBitsRequired;
        for (auto& typeBits : typeBitsVec)
        {
            if ((suitableTypeBits & typeBits) != 0)
                suitableTypeBits &= typeBits;
        }
        if (suitableTypeBits == 0)
            std::unreachable();
        return suitableTypeBits;
    }
    uint32_t MemoryAllocator::GetMemoryTypeBits(uint32_t typeBits, vk::MemoryPropertyFlags flags)
    {
        uint32_t result = 0;
        for (uint32_t i = 0; i < types.size(); i++)
        {
            if ((typeBits & (1 << i)) && (types[i].flags & flags) == flags)
            {
                result |= (1 << i);
            }
        }
        return result;
    }
}