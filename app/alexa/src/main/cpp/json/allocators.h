#ifndef RAPIDJSON_ALLOCATORS_H_
#define RAPIDJSON_ALLOCATORS_H_

#include "rapidjson.h"

RAPIDJSON_NAMESPACE_BEGIN

#ifndef RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY
#define RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY (64 * 1024)
#endif

class CrtAllocator {
    public:
        static const bool kNeedFree = true;
        void* Malloc(size_t size);
        void* Realloc(void* originalPtr, size_t newSize);
        static void Free(void *ptr);
};
template <typename BaseAllocator = CrtAllocator> class MemoryPoolAllocator {
    public:
        static const bool kNeedFree = false;
        MemoryPoolAllocator(size_t chunkSize = kDefaultChunkCapacity, BaseAllocator* baseAllocator = 0);
        MemoryPoolAllocator(void *buffer, size_t size, size_t chunkSize = kDefaultChunkCapacity, BaseAllocator* baseAllocator = 0);
        ~MemoryPoolAllocator();
        void Clear();
        size_t Capacity() const;
        size_t Size() const;
        void* Malloc(size_t size);
        void* Realloc(void* originalPtr, size_t originalSize, size_t newSize);
        static void Free(void *ptr) { RAPIDJSON_FREE(ptr); }
    private:
        MemoryPoolAllocator(const MemoryPoolAllocator& rhs);
        MemoryPoolAllocator& operator=(const MemoryPoolAllocator& rhs);
        bool AddChunk(size_t capacity);
        static const int kDefaultChunkCapacity = RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY;
        struct ChunkHeader {
            size_t capacity;
            size_t size;
            ChunkHeader *next;
        };
        void *userBuffer_;
        ChunkHeader *chunkHead_;
        size_t chunk_capacity_;
        BaseAllocator* baseAllocator_;
        BaseAllocator* ownBaseAllocator_;
};
RAPIDJSON_NAMESPACE_END
#endif