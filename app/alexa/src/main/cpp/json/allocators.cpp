#include "allocators.h"

void *rapidjson::CrtAllocator::Malloc(size_t size) {
    if (size) return RAPIDJSON_MALLOC(size);
    else return NULL;
}
void* rapidjson::CrtAllocator::Realloc(void* originalPtr, size_t newSize) {
    if (newSize <= 0) {
        RAPIDJSON_FREE(originalPtr);
        return NULL;
    }
    return RAPIDJSON_REALLOC(originalPtr, newSize);
}
void Free(void *ptr) { RAPIDJSON_FREE(ptr); }
template<typename BaseAllocator> rapidjson::MemoryPoolAllocator<BaseAllocator>::MemoryPoolAllocator(size_t chunkSize, BaseAllocator* baseAllocator) :
    chunkHead_(0), chunk_capacity_(chunkSize), userBuffer_(NULL), baseAllocator_(baseAllocator), ownBaseAllocator_(0) { }
template<typename BaseAllocator> rapidjson::MemoryPoolAllocator<BaseAllocator>::MemoryPoolAllocator(void *buffer, size_t size, size_t chunkSize, BaseAllocator* baseAllocator) :
        chunkHead_(0), chunk_capacity_(chunkSize), userBuffer_(buffer), baseAllocator_(baseAllocator), ownBaseAllocator_(0) {
    RAPIDJSON_ASSERT(buffer != 0);
    RAPIDJSON_ASSERT(size > sizeof(ChunkHeader));
    chunkHead_ = reinterpret_cast<ChunkHeader*>(buffer);
    chunkHead_->capacity = size - sizeof(ChunkHeader);
    chunkHead_->size = 0;
    chunkHead_->next = 0;
}
template<typename BaseAllocator> rapidjson::MemoryPoolAllocator<BaseAllocator>::~MemoryPoolAllocator() {
    Clear();
    RAPIDJSON_DELETE(baseAllocator_);
    RAPIDJSON_DELETE(ownBaseAllocator_);
}
template<typename BaseAllocator> void rapidjson::MemoryPoolAllocator<BaseAllocator>::Clear() {
    while (chunkHead_ != NULL) {
        ChunkHeader* next = chunkHead_->next;
        baseAllocator_->Free(chunkHead_);
        chunkHead_ = next;
    }
    chunkHead_->size = 0;
}
template<typename BaseAllocator> size_t rapidjson::MemoryPoolAllocator<BaseAllocator>::Capacity() const {
    size_t capacity = 0;
    for (ChunkHeader* c = chunkHead_; c != NULL; c = c->next)
        capacity += c->capacity;
    return capacity;
}
template<typename BaseAllocator> size_t rapidjson::MemoryPoolAllocator<BaseAllocator>::Size() const {
    size_t size = 0;
    for (ChunkHeader* c = chunkHead_; c != NULL; c = c->next)
        size += c->size;
    return size;
}
template<typename BaseAllocator> void* rapidjson::MemoryPoolAllocator<BaseAllocator>::Malloc(size_t size) {
    if (size <= 0) return NULL;
    size = RAPIDJSON_ALIGN(size);
    if (chunkHead_ == NULL || (chunkHead_->size + size) > chunkHead_->capacity)
        if (!AddChunk(chunk_capacity_ > size ? chunk_capacity_ : size)) return NULL;
    void *buffer = reinterpret_cast<char*>(chunkHead_) + RAPIDJSON_ALIGN(sizeof(ChunkHeader)) + chunkHead_->size;
    chunkHead_->size += size;
    return buffer;
}
template<typename BaseAllocator> void* rapidjson::MemoryPoolAllocator<BaseAllocator>::Realloc(void* originalPtr, size_t originalSize, size_t newSize) {
    if (newSize <= 0) return NULL;
    if (originalPtr == NULL) return Malloc(newSize);
    originalSize = RAPIDJSON_ALIGN(originalSize);
    newSize = RAPIDJSON_ALIGN(newSize);
    if (originalSize >= newSize) return originalPtr;
    if (originalPtr == reinterpret_cast<char *>(chunkHead_) + RAPIDJSON_ALIGN(sizeof(ChunkHeader)) + chunkHead_->size - originalSize) {
        size_t increment = static_cast<size_t>(newSize - originalSize);
        if (chunkHead_->size + increment <= chunkHead_->capacity) {
            chunkHead_->size += increment;
            return originalPtr;
        }
    }
    if (void* newBuffer = Malloc(newSize)) {
        if (originalSize > 0) std::memcpy(newBuffer, originalPtr, originalSize);
        return newBuffer;
    } else return NULL;
}
template<typename BaseAllocator> bool rapidjson::MemoryPoolAllocator<BaseAllocator>::AddChunk(size_t capacity) {
    if (baseAllocator_ == NULL) ownBaseAllocator_ = baseAllocator_ = RAPIDJSON_NEW(BaseAllocator)();
    if (ChunkHeader* chunk = reinterpret_cast<ChunkHeader*>(baseAllocator_->Malloc(RAPIDJSON_ALIGN(sizeof(ChunkHeader)) + capacity))) {
        chunk->capacity = capacity;
        chunk->size = 0;
        chunk->next = chunkHead_;
        chunkHead_ =  chunk;
        return true;
    } else return false;
}