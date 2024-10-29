// Copyright (c) 2021-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <Utils/SmMalloc/smmalloc.h>

#if defined(AM_SIMD_INTRINSICS)
#include <xsimd/xsimd.hpp>
#endif // defined(AM_SIMD_INTRINSICS)

#include <sstream>

namespace SparkyStudios::Audio::Amplitude
{
    static MemoryManager* gMemManager = nullptr;

#if !defined(AM_NO_MEMORY_STATS)
    static std::map<eMemoryPoolKind, std::string> gMemoryPoolNames = {
        { eMemoryPoolKind_Amplimix, "Amplimix" },   { eMemoryPoolKind_Codec, "Codec" },         { eMemoryPoolKind_Engine, "Engine" },
        { eMemoryPoolKind_Filtering, "Filtering" }, { eMemoryPoolKind_SoundData, "SoundData" }, { eMemoryPoolKind_IO, "IO" },
        { eMemoryPoolKind_Default, "Default" },
    };

    MemoryPoolStats::MemoryPoolStats(eMemoryPoolKind kind)
        : pool(kind)
    {
        maxMemoryUsed.store(0);
        allocCount.store(0);
        freeCount.store(0);
    }

    MemoryPoolStats::MemoryPoolStats(const MemoryPoolStats& copy)
        : MemoryPoolStats(copy.pool)
    {
        maxMemoryUsed.store(copy.maxMemoryUsed.load());
        allocCount.store(copy.allocCount.load());
        freeCount.store(copy.freeCount.load());
    }

    MemoryPoolStats& MemoryPoolStats::operator=(const MemoryPoolStats& other)
    {
        pool = other.pool;
        maxMemoryUsed.store(other.maxMemoryUsed.load());
        allocCount.store(other.allocCount.load());
        freeCount.store(other.freeCount.load());

        return *this;
    }
#endif

    DefaultMemoryAllocator::DefaultMemoryAllocator(AmUInt32 bucketsCount, AmSize bucketSizeInBytes)
        : MemoryAllocator()
        , _allocators()
    {
        for (auto& _allocator : _allocators)
            _allocator = _sm_allocator_create(bucketsCount, bucketSizeInBytes);
    }

    DefaultMemoryAllocator::~DefaultMemoryAllocator()
    {
        for (auto& allocator : _allocators)
        {
            _sm_allocator_destroy(static_cast<sm_allocator>(allocator));
            allocator = nullptr;
        }
    }

    AmVoidPtr DefaultMemoryAllocator::Malloc(eMemoryPoolKind pool, AmSize size)
    {
#if defined(AM_SIMD_INTRINSICS)
        return _sm_malloc(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), size, xsimd::best_arch::alignment());
#else
        return _sm_malloc(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), size, 16);
#endif
    }

    AmVoidPtr DefaultMemoryAllocator::Realloc(eMemoryPoolKind pool, AmVoidPtr address, AmSize size)
    {
#if defined(AM_SIMD_INTRINSICS)
        return _sm_realloc(
            static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), address, size, xsimd::best_arch::alignment());
#else
        return _sm_realloc(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), address, size, 16);
#endif
    }

    AmVoidPtr DefaultMemoryAllocator::Malign(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment)
    {
        return _sm_malloc(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), size, alignment);
    }

    AmVoidPtr DefaultMemoryAllocator::Realign(eMemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment)
    {
        return _sm_realloc(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), address, size, alignment);
    }

    void DefaultMemoryAllocator::Free(eMemoryPoolKind pool, AmVoidPtr address)
    {
        _sm_free(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), address);
    }

    AmSize DefaultMemoryAllocator::SizeOf(eMemoryPoolKind pool, AmVoidPtr address)
    {
        return _sm_msize(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), address);
    }

    void MemoryManager::Initialize(std::unique_ptr<MemoryAllocator> allocator)
    {
        if (!IsInitialized())
            gMemManager = new MemoryManager(std::move(allocator));

#if !defined(AM_NO_MEMORY_STATS)
        gMemManager->_memPoolsStats[eMemoryPoolKind_Amplimix] = MemoryPoolStats(eMemoryPoolKind_Amplimix);
        gMemManager->_memPoolsStats[eMemoryPoolKind_Codec] = MemoryPoolStats(eMemoryPoolKind_Codec);
        gMemManager->_memPoolsStats[eMemoryPoolKind_Default] = MemoryPoolStats(eMemoryPoolKind_Default);
        gMemManager->_memPoolsStats[eMemoryPoolKind_Engine] = MemoryPoolStats(eMemoryPoolKind_Engine);
        gMemManager->_memPoolsStats[eMemoryPoolKind_Filtering] = MemoryPoolStats(eMemoryPoolKind_Filtering);
        gMemManager->_memPoolsStats[eMemoryPoolKind_IO] = MemoryPoolStats(eMemoryPoolKind_IO);
        gMemManager->_memPoolsStats[eMemoryPoolKind_SoundData] = MemoryPoolStats(eMemoryPoolKind_SoundData);
#endif
    }

    void MemoryManager::Deinitialize()
    {
        if (gMemManager != nullptr)
        {
            delete gMemManager;
            gMemManager = nullptr;
        }
    }

    bool MemoryManager::IsInitialized()
    {
        return gMemManager != nullptr;
    }

    MemoryManager* MemoryManager::GetInstance()
    {
        AMPLITUDE_ASSERT(gMemManager != nullptr);
        return gMemManager;
    }

    MemoryManager::MemoryManager(std::unique_ptr<MemoryAllocator> allocator)
        : _allocator(std::move(allocator))
        , _memAllocations()
#if !defined(AM_NO_MEMORY_STATS)
        , _memPoolsStats()
#endif
    {
        if (_allocator == nullptr)
            _allocator = std::make_unique<DefaultMemoryAllocator>(4, 16 * 1024 * 1024);
    }

    MemoryManager::~MemoryManager()
    {
        _allocator.reset(nullptr);
    }

    AmVoidPtr MemoryManager::Malloc(eMemoryPoolKind pool, AmSize size, const char* file, AmUInt32 line)
    {
#if !defined(AM_NO_MEMORY_STATS)
        _memPoolsStats[pool].maxMemoryUsed.fetch_add(size, std::memory_order_relaxed);
        _memPoolsStats[pool].allocCount.fetch_add(1, std::memory_order_relaxed);
#endif

        AmVoidPtr ptr = _allocator->Malloc(pool, size);

        _memAllocations.insert({ pool, ptr, SizeOf(pool, ptr), file, line });
        return ptr;
    }

    AmVoidPtr MemoryManager::Malign(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line)
    {
#if !defined(AM_NO_MEMORY_STATS)
        _memPoolsStats[pool].maxMemoryUsed.fetch_add(size, std::memory_order_relaxed);
        _memPoolsStats[pool].allocCount.fetch_add(1, std::memory_order_relaxed);
#endif

        AmVoidPtr ptr = _allocator->Malign(pool, size, alignment);

        _memAllocations.insert({ pool, ptr, SizeOf(pool, ptr), file, line });

        return ptr;
    }

    AmVoidPtr MemoryManager::Realloc(eMemoryPoolKind pool, AmVoidPtr address, AmSize size, const char* file, AmUInt32 line)
    {
#if !defined(AM_NO_MEMORY_STATS)
        if (address == nullptr)
        {
            _memPoolsStats[pool].maxMemoryUsed.fetch_add(size, std::memory_order_relaxed);
            _memPoolsStats[pool].allocCount.fetch_add(1, std::memory_order_relaxed);
        }
#endif

        AmVoidPtr ptr = _allocator->Realloc(pool, address, size);

        _memAllocations.insert({ pool, ptr, SizeOf(pool, ptr), file, line });

        return ptr;
    }

    AmVoidPtr MemoryManager::Realign(
        eMemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line)
    {
#if !defined(AM_NO_MEMORY_STATS)
        if (address == nullptr)
        {
            _memPoolsStats[pool].maxMemoryUsed.fetch_add(size, std::memory_order_relaxed);
            _memPoolsStats[pool].allocCount.fetch_add(1, std::memory_order_relaxed);
        }
#endif

        AmVoidPtr ptr = _allocator->Realign(pool, address, size, alignment);

        _memAllocations.insert({ pool, ptr, SizeOf(pool, ptr), file, line });

        return ptr;
    }

    void MemoryManager::Free(eMemoryPoolKind pool, AmVoidPtr address)
    {
#if !defined(AM_NO_MEMORY_STATS)
        _memPoolsStats[pool].freeCount.fetch_add(1, std::memory_order_relaxed);
#endif

        _allocator->Free(pool, address);

        if (const auto it = _memAllocations.find({ pool, address }); it != _memAllocations.end())
            _memAllocations.erase(it);
    }

    AmSize MemoryManager::TotalReservedMemorySize(eMemoryPoolKind pool) const
    {
        AmSize total = 0;
        for (const auto& allocation : _memAllocations)
            if (allocation.pool == pool)
                total += allocation.size;

        return total;
    }

    AmSize MemoryManager::TotalReservedMemorySize() const
    {
        AmSize total = 0;
        for (const auto& allocation : _memAllocations)
            total += allocation.size;

        return total;
    }

    AmSize MemoryManager::SizeOf(eMemoryPoolKind pool, AmVoidPtr address) const
    {
        return _allocator->SizeOf(pool, address);
    }

#if !defined(AM_NO_MEMORY_STATS)
    AmString MemoryManager::GetMemoryPoolName(const eMemoryPoolKind pool)
    {
        return gMemoryPoolNames[pool];
    }

    const MemoryPoolStats& MemoryManager::GetStats(eMemoryPoolKind pool) const
    {
        return _memPoolsStats.at(pool);
    }

    AmString MemoryManager::InspectMemoryLeaks() const
    {
        if (_memAllocations.empty())
            return "No memory leaks detected";

        std::stringstream ss;
        ss << "=== Memory leaks detected ===\n\n";

        for (auto&& allocation : _memAllocations)
        {
            ss << "Pool: " << gMemoryPoolNames[allocation.pool] << std::endl;
            ss << "  Address: " << allocation.address << std::endl;
            ss << "  Size: " << allocation.size << std::endl;
            ss << "  File: " << allocation.file << std::endl;
            ss << "  Line: " << allocation.line << std::endl << std::endl;
        }

        return ss.str();
    }
#endif

    ScopedMemoryAllocation::ScopedMemoryAllocation(eMemoryPoolKind pool, AmSize size, const char* file, AmUInt32 line)
    {
        _pool = pool;
        _address = amMemory->Malloc(_pool, size, file, line);
    }

    ScopedMemoryAllocation::ScopedMemoryAllocation(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line)
    {
        _pool = pool;
        _address = amMemory->Malign(_pool, size, alignment, file, line);
    }

    ScopedMemoryAllocation::~ScopedMemoryAllocation()
    {
        if (_address == nullptr)
            return;

        ampoolfree(_pool, _address);
        _address = nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude
