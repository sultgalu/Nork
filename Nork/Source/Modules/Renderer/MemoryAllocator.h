#pragma once

#include "Vulkan/DeviceMemory.h"
#include "Vulkan/PhysicalDevice.h"

namespace Nork::Renderer {
    struct MemoryFlags
    {
        using enum vk::MemoryPropertyFlagBits;
        vk::MemoryPropertyFlags required;
        std::vector<vk::MemoryPropertyFlags> optional; // in priority-order
    };
    class DeviceMemory;
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
            vk::DeviceSize poolOffset;
            vk::DeviceSize size;
            std::shared_ptr<Pool> pool;
            uint32_t suitableMemoryTypeBits;
        };
    public:
        MemoryAllocator(const Vulkan::PhysicalDevice& physicalDevice);
        DeviceMemory Allocate(vk::MemoryRequirements req, const MemoryFlags& flags); // desiredFlags are in priority order
    private:
        std::optional<std::shared_ptr<Pool>> FindPool(uint32_t typeIndex, vk::DeviceSize size);
        uint32_t GetMemoryTypeWithMostAvailableMemory(uint32_t typeBits);
        uint32_t FindSuitableMemoryTypes(uint32_t typeBitsFilter, const MemoryFlags& flags);
        uint32_t GetMemoryTypeBits(uint32_t typeBits, vk::MemoryPropertyFlags flags);
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
    class DeviceMemory // wrapper around MemoryAllocation
    {
    public:
        DeviceMemory() = default;
        DeviceMemory(const MemoryAllocation& alloc)
            : allocation(alloc)
        {}
        
        std::shared_ptr<Vulkan::DeviceMemory> Underlying()
        {
            return allocation->pool->memory;
        }
        vk::DeviceSize PoolOffset()
        {
            return allocation->poolOffset;
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
            ptr = (uint8_t*)allocation->pool->memory->ptr + allocation->poolOffset + offset;
            return reinterpret_cast<T*>(ptr);
        }
        template<class T = void>
        T* Map(uint32_t offset = 0)
        {
            return Map<T>(offset, allocation->size);
        }
        void* Ptr()
        {
            if (!IsMapped()) 
                Map();
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
        bool IsHostCoherent() const { return IsBitSet(vk::MemoryPropertyFlagBits::eHostCoherent); }
        bool IsHostCached() const { return IsBitSet(vk::MemoryPropertyFlagBits::eHostCached); }
        bool IsHostVisible() const { return IsBitSet(vk::MemoryPropertyFlagBits::eHostVisible); }
        bool IsDeviceLocal() const { return IsBitSet(vk::MemoryPropertyFlagBits::eDeviceLocal); }
    private:
        bool IsBitSet(vk::MemoryPropertyFlagBits bit) const { return (allocation->pool->memType.flags & bit) == bit; }
    public:
        MemoryAllocation allocation;
        // should implement reference counting on pool's ptr, so can unmap pool's memory when possible
        void* ptr = nullptr;
    };
}