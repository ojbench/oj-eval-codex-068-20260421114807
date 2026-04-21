#include "allocator.hpp"
#include <new>
#include <algorithm>

TLSFAllocator::TLSFAllocator(std::size_t memoryPoolSize)
    : memoryPool(nullptr), poolSize(memoryPoolSize) {
    initializeMemoryPool(memoryPoolSize);
}

TLSFAllocator::~TLSFAllocator() {
    delete[] reinterpret_cast<unsigned char*>(memoryPool);
}

void TLSFAllocator::initializeMemoryPool(std::size_t size) {
    memoryPool = reinterpret_cast<void*>(new (std::nothrow) unsigned char[size]);
    blocks.clear();
    freeSet.clear();
    if (memoryPool && size > 0) {
        Block b{0u, size, true};
        blocks.emplace(0u, b);
        freeSet.emplace(size, 0u);
    }
}

void* TLSFAllocator::getMemoryPoolStart() const { return memoryPool; }
std::size_t TLSFAllocator::getMemoryPoolSize() const { return poolSize; }

std::size_t TLSFAllocator::getMaxAvailableBlockSize() const {
    if (freeSet.empty()) return 0u;
    auto it = freeSet.rbegin();
    return it->first;
}

void TLSFAllocator::insertFree(std::size_t start, std::size_t size) {
    freeSet.emplace(size, start);
}

void TLSFAllocator::eraseFree(std::size_t start, std::size_t size) {
    auto it = freeSet.find({size, start});
    if (it != freeSet.end()) freeSet.erase(it);
}

void* TLSFAllocator::allocate(std::size_t size) {
    if (size == 0) return nullptr;
    auto it = freeSet.lower_bound({size, 0u});
    if (it == freeSet.end()) return nullptr;
    std::size_t bSize = it->first;
    std::size_t bStart = it->second;
    freeSet.erase(it);
    auto bit = blocks.find(bStart);
    if (bit == blocks.end() || !bit->second.free || bit->second.size != bSize) {
        return nullptr;
    }
    if (bSize == size) {
        bit->second.free = false;
        unsigned char* base = reinterpret_cast<unsigned char*>(memoryPool);
        return base + bStart;
    } else {
        Block alloc{bStart, size, false};
        Block rem{bStart + size, bSize - size, true};
        bit->second = alloc;
        blocks.emplace(rem.start, rem);
        insertFree(rem.start, rem.size);
        unsigned char* base = reinterpret_cast<unsigned char*>(memoryPool);
        return base + bStart;
    }
}

std::size_t TLSFAllocator::ptrToOffset(void* ptr) const {
    if (!ptr || !memoryPool) return static_cast<std::size_t>(-1);
    auto base = reinterpret_cast<const unsigned char*>(memoryPool);
    auto p = reinterpret_cast<const unsigned char*>(ptr);
    if (p < base) return static_cast<std::size_t>(-1);
    return static_cast<std::size_t>(p - base);
}

void TLSFAllocator::deallocate(void* ptr) {
    if (!ptr) return;
    std::size_t start = ptrToOffset(ptr);
    if (start == static_cast<std::size_t>(-1)) return;
    auto it = blocks.find(start);
    if (it == blocks.end() || it->second.free) return;

    it->second.free = true;

    auto next = std::next(it);
    if (next != blocks.end() && next->second.free &&
        it->second.start + it->second.size == next->second.start) {
        eraseFree(next->second.start, next->second.size);
        it->second.size += next->second.size;
        blocks.erase(next);
    }

    if (it != blocks.begin()) {
        auto prev = std::prev(it);
        if (prev->second.free && prev->second.start + prev->second.size == it->second.start) {
            eraseFree(prev->second.start, prev->second.size);
            prev->second.size += it->second.size;
            blocks.erase(it);
            it = prev;
        }
    }

    insertFree(it->second.start, it->second.size);
}
