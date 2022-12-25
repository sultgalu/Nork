#pragma once

#include "Vulkan/DeviceMemory.h"
#include "Vulkan/PhysicalDevice.h"

namespace Nork::Renderer {
    class MemoryAllocator
    {
    public:
        struct Heap
        {
            vk::DeviceSize size;
            vk::DeviceSize allocated = 0;
        };
        struct Pool;
        struct MemoryType
        {
            uint32_t index;
            vk::MemoryPropertyFlags flags;
            Heap& heap;
            bool IsHostCoherent() const { return IsBitSet(vk::MemoryPropertyFlagBits::eHostCoherent); }
            bool IsHostCached() const { return IsBitSet(vk::MemoryPropertyFlagBits::eHostCached); }
            bool IsHostVisible() const { return IsBitSet(vk::MemoryPropertyFlagBits::eHostVisible); }
            bool IsDeviceLocal() const { return IsBitSet(vk::MemoryPropertyFlagBits::eDeviceLocal); }
        private:
            bool IsBitSet(vk::MemoryPropertyFlagBits bit) const { return (flags & bit) == bit; }
        public:
            vk::DeviceSize allocated = 0;
            std::vector<std::shared_ptr<Pool>> pools;
        };
        struct Allocation;
        struct Pool
        {
            MemoryType& memType;
            std::shared_ptr<Vulkan::DeviceMemory> memory;
            vk::DeviceSize size;
        public:
            Pool(MemoryType& memType, vk::DeviceSize size)
                : memType(memType), size(size)
            {
                memory = std::make_shared<Vulkan::DeviceMemory>(
                    vk::MemoryAllocateInfo(size, memType.index));
            }
        public:
            vk::DeviceSize offset = 0;
            std::vector<std::weak_ptr<Allocation>> allocations;
            // weak -> avoid circular dependecies
        };
        struct Allocation
        {
            vk::DeviceSize offset;
            vk::DeviceSize size;
            std::shared_ptr<Pool> pool;
            uint32_t suitableMemoryTypeBits;
        };
        MemoryAllocator(const Vulkan::PhysicalDevice& physicalDevice)
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
        // desiredFlags are in priority order
        std::shared_ptr<Allocation> Allocate(vk::MemoryRequirements req, vk::MemoryPropertyFlags requiredFlags,
            const std::vector<vk::MemoryPropertyFlags>& desiredFlags = {})
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
            pool->offset += alignmentOffset;
            allocation->offset = pool->offset;
            pool->offset += allocation->size;

            allocation->pool = pool;
            pool->allocations.push_back(allocation);
            return allocation;
        }
        std::optional<std::shared_ptr<Pool>> FindPool(uint32_t typeIndex, vk::DeviceSize size)
        {
            for (auto& pool : types[typeIndex].pools)
            {
                if (pool->size - pool->offset >= size)
                    return pool;
            }
            return std::nullopt;
        }
        uint32_t GetMemoryTypeWithMostAvailableMemory(uint32_t typeBits)
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
        uint32_t FindSuitableMemoryTypes(uint32_t typeBitsFilter, vk::MemoryPropertyFlags requiredFlags,
            const std::vector<vk::MemoryPropertyFlags>& desiredFlags = {})
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
        uint32_t GetMemoryTypeBits(uint32_t typeBits, vk::MemoryPropertyFlags flags)
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
    public:
        std::vector<Heap> heaps;
        std::vector<MemoryType> types;
        static constexpr vk::DeviceSize poolSize = 256 * 1024 * 1024;
        
        static MemoryAllocator& Instance()
        {
            return *instance;
        }
        ~MemoryAllocator()
        {
            instance = nullptr;
        }
    private:
        static MemoryAllocator* instance;
    };
    
    using MemoryAllocation = std::shared_ptr<MemoryAllocator::Allocation>;
    struct MemoryAllocationWrapper
    {
        MemoryAllocationWrapper() = default;
        MemoryAllocationWrapper(const MemoryAllocation& alloc)
            : allocation(alloc)
        {}
        MemoryAllocation allocation;
        Vulkan::DeviceMemory& DeviceMemory()
        {
            return *allocation->pool->memory;
        }
        vk::DeviceSize Offset()
        {
            return allocation->offset;
        }
        vk::DeviceSize Size()
        {
            return allocation->size;
        }
        template<class T = void>
        T* Map(uint32_t offset, uint32_t size)
        {
            if (IsMapped()) std::unreachable();
            if (!allocation->pool->memory->IsMapped())
                allocation->pool->memory->Map();
            ptr = (uint8_t*)allocation->pool->memory->ptr + allocation->offset + offset;
            return reinterpret_cast<T*>(ptr);
        }
        template<class T = void>
        T* Map(uint32_t offset = 0)
        {
            return Map<T>(offset, allocation->size);
        }
        void* Ptr()
        {
            if (!IsMapped()) std::unreachable();
            return ptr;
        }
        template<class T = void>
        T* Ptr()
        {
            if (!IsMapped()) std::unreachable();
            return reinterpret_cast<T*>(ptr);
        }
        bool IsMapped()
        {
            return ptr != nullptr;
        }
        void* ptr = nullptr;
    };
}