#ifndef TLSF_ALLOCATOR_HPP
#define TLSF_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <array>
#include <map>
#include <set>

class TLSFAllocator {
public:
    explicit TLSFAllocator(std::size_t memoryPoolSize);
    ~TLSFAllocator();

    void* allocate(std::size_t size);
    void deallocate(void* ptr);

    void* getMemoryPoolStart() const;
    std::size_t getMemoryPoolSize() const;
    std::size_t getMaxAvailableBlockSize() const;

    TLSFAllocator(const TLSFAllocator&) = delete;
    TLSFAllocator& operator=(const TLSFAllocator&) = delete;
    TLSFAllocator(TLSFAllocator&&) = delete;
    TLSFAllocator& operator=(TLSFAllocator&&) = delete;

private:
    struct Block {
        std::size_t start;
        std::size_t size;
        bool free;
    };

    void* memoryPool;
    std::size_t poolSize;

    std::map<std::size_t, Block> blocks; // key = start offset
    std::set<std::pair<std::size_t, std::size_t>> freeSet; // (size,start)

    void initializeMemoryPool(std::size_t size);
    void insertFree(std::size_t start, std::size_t size);
    void eraseFree(std::size_t start, std::size_t size);
    std::size_t ptrToOffset(void* ptr) const;
};

#endif // TLSF_ALLOCATOR_HPP
