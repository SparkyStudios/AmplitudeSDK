
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

#pragma once

#ifndef _AM_CORE_MEMORY_H
#define _AM_CORE_MEMORY_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

/**
 * @brief Shortcut access to the Amplitude's memory manager instance.
 *
 * @ingroup memory
 */
#define amMemory SparkyStudios::Audio::Amplitude::MemoryManager::GetInstance()

/**
 * @brief Allocates a block of memory from the specified memory pool.
 *
 * @param _pool_ The memory pool to allocate from.
 * @param _size_ The size of the memory to allocate.
 *
 * @ingroup memory
 */
#define ampoolmalloc(_pool_, _size_) amMemory->Malloc(_pool_, _size_, __FILE__, __LINE__)

/**
 * @brief Allocates an aligned block of memory from the specified memory pool.
 *
 * @param _pool_ The memory pool to deallocate from.
 * @param _size_ The size of the memory to allocate.
 * @param _alignment_ The alignment of the memory to allocate.
 *
 * @ingroup memory
 */
#define ampoolmalign(_pool_, _size_, _alignment_) amMemory->Malign(_pool_, _size_, _alignment_, __FILE__, __LINE__)

/**
 * @brief Reallocates a block of memory from the specified memory pool.
 *
 * @param _pool_ The memory pool to reallocate from. Should be the same as the one used to allocate the memory.
 * @param _ptr_ The pointer to reallocate.
 * @param _size_ The new size of the memory.
 *
 * @ingroup memory
 */
#define ampoolrealloc(_pool_, _ptr_, _size_) amMemory->Realloc(_pool_, _ptr_, _size_, __FILE__, __LINE__)

/**
 * @brief Reallocates an aligned block of memory from the specified memory pool.
 *
 * @param _pool_ The memory pool to reallocate from. Should be the same as the one used to allocate the memory.
 * @param _ptr_ The pointer to reallocate.
 * @param _size_ The new size of the memory.
 * @param _alignment_ The alignment of the memory to reallocate.
 *
 * @ingroup memory
 */
#define ampoolrealign(_pool_, _ptr_, _size_, _alignment_) amMemory->Realign(_pool_, _ptr_, _size_, _alignment_, __FILE__, __LINE__)

/**
 * @brief Deallocates a block of memory from the specified memory pool.
 *
 * @param _pool_ The memory pool to deallocate from.
 * @param _ptr_ The pointer to deallocate.
 *
 * @ingroup memory
 */
#define ampoolfree(_pool_, _ptr_) amMemory->Free(_pool_, _ptr_)

/**
 * @brief Allocates a block of memory from the default memory pool.
 *
 * @param _size_ The size of the memory to allocate.
 *
 * @see @ref ampoolmalloc ampoolmalloc
 *
 * @ingroup memory
 */
#define ammalloc(_size_) ampoolmalloc(SparkyStudios::Audio::Amplitude::eMemoryPoolKind_Default, _size_)

/**
 * @brief Allocates a block of memory from the default memory pool.
 *
 * @param _size_ The size of the memory to allocate.
 * @param _alignment_ The alignment of the memory to allocate.
 *
 * @see @ref ampoolmalign ampoolmalign
 *
 * @ingroup memory
 */
#define ammalign(_size_, _alignment_) ampoolmalign(SparkyStudios::Audio::Amplitude::eMemoryPoolKind_Default, _size_, _alignment_)

/**
 * @brief Reallocates a block of memory from the default memory pool.
 *
 * @param _ptr_ The pointer to reallocate.
 * @param _size_ The new size of the memory.
 *
 * @see @ref ampoolrealloc ampoolrealloc
 *
 * @ingroup memory
 */
#define amrealloc(_ptr_, _size_) ampoolrealloc(SparkyStudios::Audio::Amplitude::eMemoryPoolKind_Default, _ptr_, _size_)

/**
 * @brief Reallocates an aligned block of memory from the default memory pool.
 *
 * @param _ptr_ The pointer to reallocate.
 * @param _size_ The new size of the memory.
 * @param _alignment_ The alignment of the memory to reallocate.
 *
 * @see @ref ampoolrealign ampoolrealign
 *
 * @ingroup memory
 */
#define amrealign(_ptr_, _size_, _alignment_)                                                                                              \
    ampoolrealign(SparkyStudios::Audio::Amplitude::eMemoryPoolKind_Default, _ptr_, _size_, _alignment_)

/**
 * @brief Deallocates a block of memory from the default memory pool.
 *
 * @param _ptr_ The pointer to deallocate.
 *
 * @see @ref ampoolfree ampoolfree
 *
 * @ingroup memory
 */
#define amfree(_ptr_) ampoolfree(SparkyStudios::Audio::Amplitude::eMemoryPoolKind_Default, _ptr_)

/**
 * @brief Allocates memory for a new object in the given memory pool.
 *
 * This will create a new memory allocation in the given pool. The allocated
 * memory will be freed when the object is destroyed using @ref ampooldelete ampooldelete.
 *
 * @param _pool_ The memory pool to allocate from.
 * @param _type_ The type of the object to allocate.
 * @param ... Additional arguments to pass to the constructor of the object.
 *
 * @see @ref ampooldelete ampooldelete
 *
 * @ingroup memory
 */
#define ampoolnew(_pool_, _type_, ...) new (ampoolmalign((_pool_), sizeof(_type_), alignof(_type_))) _type_(__VA_ARGS__)

/**
 * @brief Deallocates a memory allocated with @ref ampoolnew ampoolnew.
 *
 * This will call the object's destructor before to free the memory.
 *
 * @param _pool_ The memory pool to deallocate from.
 * @param _type_ The type of the object to deallocate.
 * @param _ptr_ The pointer to the object to deallocate.
 *
 * @see @ref ampoolnew ampoolnew
 *
 * @ingroup memory
 */
#define ampooldelete(_pool_, _type_, _ptr_)                                                                                                \
    {                                                                                                                                      \
        if ((_ptr_) != nullptr)                                                                                                            \
        {                                                                                                                                  \
            (_ptr_)->~_type_();                                                                                                            \
            ampoolfree((_pool_), (_ptr_));                                                                                                 \
        }                                                                                                                                  \
    }

/**
 * @brief Allocates memory for a new object in the Default pool using the memory manager.
 *
 * This will create a new memory allocation in the Default pool. The allocated
 * memory will be freed when the object is destroyed using @ref amdelete amdelete.
 *
 * @param _type_ The type of the object to allocate.
 * @param ... Additional arguments to pass to the constructor of the object.
 *
 * @see @ref amdelete amdelete
 *
 * @ingroup memory
 */
#define amnew(_type_, ...) ampoolnew(SparkyStudios::Audio::Amplitude::eMemoryPoolKind_Default, _type_, __VA_ARGS__)

/**
 * @brief Deallocates a memory allocated with @ref amnew amnew.
 *
 * This will call the object's destructor before the memory is freed.
 *
 * @param _type_ The type of the object to deallocate.
 * @param _ptr_ The pointer to the object to deallocate.
 *
 * @see @ref amnew amnew
 *
 * @ingroup memory
 */
#define amdelete(_type_, _ptr_) ampooldelete(SparkyStudios::Audio::Amplitude::eMemoryPoolKind_Default, _type_, _ptr_)

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Available memory pools.
     *
     * @ingroup memory
     */
    enum eMemoryPoolKind : AmUInt8
    {
        /**
         * @brief Amplitude Engine allocations.
         */
        eMemoryPoolKind_Engine,

        /**
         * @brief Amplimix allocations.
         */
        eMemoryPoolKind_Amplimix,

        /**
         * @brief Sound data and streams.
         */
        eMemoryPoolKind_SoundData,

        /**
         * @brief Filters related allocations.
         */
        eMemoryPoolKind_Filtering,

        /**
         * @brief Encoding/Decoding allocations.
         *
         */
        eMemoryPoolKind_Codec,

        /**
         * @brief I/O and filesystem related allocations.
         */
        eMemoryPoolKind_IO,

        /**
         * @brief Default allocations pool. Use this when the allocated memory pool is not available.
         *
         * @note `amnew` use this pool to allocate memory from the memory manager.
         */
        eMemoryPoolKind_Default,

        /**
         * @brief The total number of memory pools.
         */
        eMemoryPoolKind_COUNT,
    };

#if !defined(AM_NO_MEMORY_STATS)
    /**
     * @brief Collects the statistics about the memory allocations
     * for a specific pool
     *
     * @ingroup memory
     */
    struct AM_API_PUBLIC MemoryPoolStats
    {
        /**
         * @brief The pool for which this statistics is for.
         */
        eMemoryPoolKind pool;

        /**
         * @brief The maximum total memory used by this pool.
         */
        std::atomic<AmSize> maxMemoryUsed{};

        /**
         * @brief The total count of allocations made on this pool.
         */
        std::atomic<AmUInt64> allocCount{};

        /**
         * @brief The total count of frees made on this pool.
         */
        std::atomic<AmUInt64> freeCount{};

        /**
         * @brief Default constructor.
         */
        MemoryPoolStats()
            : MemoryPoolStats(eMemoryPoolKind_COUNT)
        {}

        /**
         * @brief Creates a new `MemoryPoolStats` object.
         *
         * @param[in] pool The pool to get the statistics for.
         */
        explicit MemoryPoolStats(eMemoryPoolKind pool);

        /**
         * @brief Copy constructor.
         *
         * @param[in] copy The `MemoryPoolStats` object to copy from.
         */
        MemoryPoolStats(const MemoryPoolStats& copy);

        /**
         * @brief Assignment operator.
         *
         * @param[in] other The `MemoryPoolStats` object to assign from.
         */
        MemoryPoolStats& operator=(const MemoryPoolStats& other);
    };
#endif

    /**
     * @brief Memory Allocator Interface.
     *
     * @ingroup memory
     */
    class MemoryAllocator
    {
    public:
        /**
         * @brief Default Destructor.
         */
        virtual ~MemoryAllocator() = default;

        /**
         * @brief Allocates a block of memory.
         *
         * @param[in] pool The memory pool to allocate from.
         * @param[in] size The size of the memory to allocate.
         *
         * @return A pointer to the allocated memory.
         */
        virtual AmVoidPtr Malloc(eMemoryPoolKind pool, AmSize size) = 0;

        /**
         * @brief Reallocates a block of memory.
         *
         * @param[in] pool The memory pool to reallocate from.
         * @param[in] address The pointer to the memory to reallocate.
         * @param[in] size The new size of the memory.
         *
         * @return A pointer to the reallocated memory.
         */
        virtual AmVoidPtr Realloc(eMemoryPoolKind pool, AmVoidPtr address, AmSize size) = 0;

        /**
         * @brief Allocates an aligned block of memory.
         *
         * @param[in] pool The memory pool to allocate from.
         * @param[in] size The size of the memory to allocate.
         * @param[in] alignment The alignment of the memory to allocate.
         *
         * @return A pointer to the allocated memory.
         */
        virtual AmVoidPtr Malign(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment) = 0;

        /**
         * @brief Reallocates an aligned block of memory.
         *
         * @param[in] pool The memory pool to reallocate from.
         * @param[in] address The pointer to the memory to reallocate.
         * @param[in] size The new size of the memory.
         * @param[in] alignment The alignment of the memory to reallocate.
         *
         * @return A pointer to the reallocated memory.
         */
        virtual AmVoidPtr Realign(eMemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment) = 0;

        /**
         * @brief Deallocates a block of memory.
         *
         * @param[in] pool The memory pool to deallocate from.
         * @param[in] address The pointer to the memory to deallocate.
         */
        virtual void Free(eMemoryPoolKind pool, AmVoidPtr address) = 0;

        /**
         * @brief Gets the size of the memory at the given address.
         *
         * @param[in] pool The memory pool to get the size from.
         * @param[in] address The address of the memory.
         *
         * @return The size of the memory at the given address.
         */
        virtual AmSize SizeOf(eMemoryPoolKind pool, AmVoidPtr address) = 0;
    };

    /**
     * @brief Default memory allocator.
     *
     * This implementation uses a fast and efficient "proxy" allocator designed to handle many small allocations/deallocations in heavy
     * multithreaded scenarios.
     *
     * @ingroup memory
     */
    class DefaultMemoryAllocator final : public MemoryAllocator
    {
    public:
        /**
         * @brief Initializes a new default memory allocator.
         *
         * This constructor will create the given number of allocator buckets, each of the
         * given size.
         *
         * @param[in] bucketsCount The number of allocator buckets to create.
         * @param[in] bucketSizeInBytes The size of each allocator bucket in bytes.
         */
        DefaultMemoryAllocator(AmUInt32 bucketsCount, AmSize bucketSizeInBytes);

        /**
         * @brief Destroy the allocator.
         */
        ~DefaultMemoryAllocator() override;

        /**
         * @inherit
         */
        AmVoidPtr Malloc(eMemoryPoolKind pool, AmSize size) override;

        /**
         * @inherit
         */
        AmVoidPtr Realloc(eMemoryPoolKind pool, AmVoidPtr address, AmSize size) override;

        /**
         * @inherit
         */
        AmVoidPtr Malign(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment) override;

        /**
         * @inherit
         */
        AmVoidPtr Realign(eMemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment) override;

        /**
         * @inherit
         */
        void Free(eMemoryPoolKind pool, AmVoidPtr address) override;

        /**
         * @inherit
         */
        AmSize SizeOf(eMemoryPoolKind pool, AmVoidPtr address) override;

    private:
        /***
         * @brief Internal allocator spaces, for each pool.
         *
         * @internal
         */
        void* _allocators[eMemoryPoolKind_COUNT];
    };

    /**
     * @brief Manages memory allocations inside the engine.
     *
     * @ingroup memory
     */
    class AM_API_PUBLIC MemoryManager
    {
    public:
        /**
         * @brief A single memory allocation.
         *
         * This struct describes a single memory allocation. It is used to track memory allocations made by
         * the engine for each pool, and inspect memory leaks.
         */
        struct Allocation
        {
            /**
             * @brief The memory pool.
             */
            eMemoryPoolKind pool;

            /**
             * @brief The address of the allocation.
             */
            AmVoidPtr address;

            /**
             * @brief The size of the allocation.
             */
            AmSize size;

            /**
             * @brief The file in which the allocation was made.
             */
            const char* file;

            /**
             * @brief The line in which the allocation was made.
             */
            AmUInt32 line;

            /**
             * @brief Explicit conversion to the address of the allocation.
             *
             * @return The address of the allocation.
             */
            [[nodiscard]] AM_INLINE explicit operator AmVoidPtr() const
            {
                return address;
            }

            /**
             * @brief Checks if the address matches the provided pointer.
             *
             * @param[in] ptr The pointer to compare with.
             *
             * @return `true` if the addresses match, `false` otherwise.
             */
            [[nodiscard]] AM_INLINE bool operator==(const AmVoidPtr& ptr) const
            {
                return address == ptr;
            }

            /**
             * @brief Checks if the address matches the provided pointer.
             *
             * @param[in] ptr The pointer to compare with.
             *
             * @return `true` if the addresses match, `false` otherwise.
             */
            [[nodiscard]] AM_INLINE bool operator==(const Allocation& other) const
            {
                return pool == other.pool && address == other.address;
            }

            /**
             * @brief Checks if the address is less than the provided address.
             *
             * @param[in] other The other allocation to compare with.
             *
             * @return `true` if the addresses are less than, `false` otherwise.
             */
            [[nodiscard]] AM_INLINE bool operator<(const Allocation& other) const
            {
                return address < other.address;
            }
        };

        /**
         * @brief Initializes the memory manager.
         *
         * @remarks This should be done prior to any call of @ref GetInstance `GetInstance()`
         * or any usage of `amMemory`.
         */
        static void Initialize(std::unique_ptr<MemoryAllocator> allocator = nullptr);

        /**
         * @brief Unloads the memory manager.
         */
        static void Deinitialize();

        /**
         * @brief Checks whether the memory manager is initialized.
         *
         * @return `true` if the memory manager is initialized, `false` otherwise.
         */
        [[maybe_unused]] static bool IsInitialized();

        /**
         * @brief Gets the actual instance of the memory manager.
         */
        static MemoryManager* GetInstance();

        /**
         * @brief Allocates a block of memory.
         *
         * @param[in] pool The memory pool to allocate from.
         * @param[in] size The size of the block to allocate.
         * @param[in] file The file in which the allocation was made.
         * @param[in] line The line in which the allocation was made.
         *
         * @return A pointer to the allocated block.
         */
        [[nodiscard]] AmVoidPtr Malloc(eMemoryPoolKind pool, AmSize size, const char* file, AmUInt32 line);

        /**
         * @brief Allocates an aligned  block of memory.
         *
         * @param[in] pool The memory pool to allocate from.
         * @param[in] size The size of the block to allocate.
         * @param[in] alignment The alignment of the block to allocate.
         * @param[in] file The file in which the allocation was made.
         * @param[in] line The line in which the allocation was made.
         *
         * @return A pointer to the allocated block.
         */
        [[nodiscard]] AmVoidPtr Malign(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line);

        /**
         * @brief Updates the size of a previously allocated memory.
         *
         * @param[in] pool The memory pool to update.
         * @param[in] address The address of the memory to update.
         * @param[in] size The new size of the memory.
         * @param[in] file The file in which the allocation was made.
         * @param[in] line The line in which the allocation was made.
         *
         * @return A pointer to the allocated block. Maybe equal to `address` if the original pointer had enough memory.
         */
        [[nodiscard]] AmVoidPtr Realloc(eMemoryPoolKind pool, AmVoidPtr address, AmSize size, const char* file, AmUInt32 line);

        /**
         * @brief Updates the size of a previously allocated aligned memory.
         *
         * @param[in] pool The memory pool to update.
         * @param[in] address The address of the aligned memory to update.
         * @param[in] size The new size of the aligned memory.
         * @param[in] alignment The new alignment of the aligned memory.
         * @param[in] file The file in which the allocation was made.
         * @param[in] line The line in which the allocation was made.
         *
         * @return A pointer to the allocated block. Maybe equal to `address` if the original pointer had enough memory.
         */
        [[nodiscard]] AmVoidPtr Realign(
            eMemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line);

        /**
         * @brief Releases an allocated memory block.
         *
         * @param[in] pool The memory pool to release from.
         * @param[in] address The address of the memory to release.
         */
        void Free(eMemoryPoolKind pool, AmVoidPtr address);

        /**
         * @brief Gets the total allocated size of the specified pool.
         *
         * @param[in] pool The memory pool to get the total allocated size from.
         *
         * @return The total currently allocated size in the specified pool.
         */
        [[nodiscard]] AmSize TotalReservedMemorySize(eMemoryPoolKind pool) const;

        /**
         * @brief Gets the total allocated size.
         *
         * @return The total currently allocated size.
         */
        [[nodiscard]] AmSize TotalReservedMemorySize() const;

        /**
         * @brief Gets the size of the given memory block.
         *
         * @param[in] pool The memory pool to get the size from.
         * @param[in] address The address of the memory block.
         *
         * @return The size of the given memory block.
         */
        [[nodiscard]] AmSize SizeOf(eMemoryPoolKind pool, AmVoidPtr address) const;

#if !defined(AM_NO_MEMORY_STATS)
        /**
         * @brief Gets the name of the given memory pool.
         *
         * @param[in] pool The memory pool to get the name for.
         *
         * @return The name of the memory pool.
         */
        static AmString GetMemoryPoolName(eMemoryPoolKind pool);

        /**
         * @brief Returns the memory allocation statistics for the given pool.
         *
         * @param[in] pool The pool to get the statistics for.
         */
        [[nodiscard]] const MemoryPoolStats& GetStats(eMemoryPoolKind pool) const;

        /**
         * @brief Inspects the memory manager for memory leaks.
         *
         * @tip This function is most useful after the engine has been deinitialized. Calling it before may just
         * report a lot of false positives (allocated memories which are still in use).
         *
         * @return A string containing a report for the detected memory leaks.
         */
        [[nodiscard]] AmString InspectMemoryLeaks() const;
#endif

    private:
        explicit MemoryManager(std::unique_ptr<MemoryAllocator> allocator);
        ~MemoryManager();

        std::unique_ptr<MemoryAllocator> _allocator;

        std::set<Allocation> _memAllocations;

#if !defined(AM_NO_MEMORY_STATS)
        std::map<eMemoryPoolKind, MemoryPoolStats> _memPoolsStats;
#endif
    };

    /**
     * @brief Allocates a block of memory with the given size in the given pool.
     *
     * That allocation will be restricted to the current scope, and will be freed
     * automatically when the scope ends.
     *
     * @ingroup memory
     */
    class AM_API_PUBLIC ScopedMemoryAllocation
    {
    public:
        /**
         * @brief Default constructor.
         */
        ScopedMemoryAllocation() = default;

        /**
         * @brief Creates a new scoped memory allocation.
         *
         * @param[in] pool The memory pool to allocate from.
         * @param[in] size The size of the block to allocate.
         * @param[in] file The file in which the allocation was made.
         * @param[in] line The line in which the allocation was made.
         */
        ScopedMemoryAllocation(eMemoryPoolKind pool, AmSize size, const char* file, AmUInt32 line);

        /**
         * @brief Creates a new scoped aligned memory allocation.
         *
         * @param[in] pool The memory pool to allocate from.
         * @param[in] size The size of the block to allocate.
         * @param[in] alignment The alignment of the block to allocate.
         * @param[in] file The file in which the allocation was made.
         * @param[in] line The line in which the allocation was made.
         */
        ScopedMemoryAllocation(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line);

        /**
         * @brief Releases the allocated memory.
         */
        ~ScopedMemoryAllocation();

        /**
         * @brief Gets the allocated memory address.
         *
         * @return The allocated memory address.
         */
        template<typename T>
        [[nodiscard]] AM_INLINE T* PointerOf() const
        {
            return static_cast<T*>(_address);
        }

        /**
         * @brief Converts the allocated memory address to a different type.
         *
         * @return The allocated memory address converted to the specified type.
         */
        template<typename T, typename std::enable_if_t<std::is_pointer_v<T>, bool> = false>
        [[nodiscard]] AM_INLINE T As() const
        {
            return reinterpret_cast<T>(_address);
        }

        /**
         * @brief Gets the allocated memory address.
         *
         * @return The allocated memory address.
         */
        [[nodiscard]] AM_INLINE AmVoidPtr Address() const
        {
            return _address;
        }

    private:
        eMemoryPoolKind _pool = eMemoryPoolKind_Default;
        void* _address = nullptr;
    };

    /**
     * @brief Deleter for unique pointers.
     *
     * @tparam Pool The memory pool to delete the pointer from.
     * @tparam T The type of the pointer to delete.
     *
     * @note This deleter uses the `ampooldelete` function to delete the pointer.
     *       It is templated to ensure that the correct pool is used.
     *       This allows for a single implementation of the deleter for all pointer types.
     *       The `std::unique_ptr` will automatically call this deleter when the pointer is deleted.
     *
     * @see ampooldelete, AmUniquePtr
     *
     * @ingroup memory
     */
    template<eMemoryPoolKind Pool, class T>
    struct am_delete
    {
        constexpr am_delete() noexcept = default;

        AM_INLINE void operator()(T* ptr) const noexcept
        {
            static_assert(!std::is_void_v<T>, "Cannot delete a void pointer.");
            ampooldelete(Pool, T, ptr);
        }
    };

    /**
     * @brief Unique pointer type.
     *
     * @tparam Pool The memory pool to allocate the pointer from.
     * @tparam T The type of the pointer to allocate.
     *
     * @ingroup memory
     */
    template<eMemoryPoolKind Pool, class T>
    using AmUniquePtr = std::unique_ptr<T, am_delete<Pool, T>>;
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_MEMORY_H