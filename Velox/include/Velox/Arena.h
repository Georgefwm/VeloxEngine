#pragma once

namespace Velox {

struct Arena {
    size_t size;
    size_t offset;
    char*  buffer;

    explicit Arena(size_t bytes);
    ~Arena();

    void* AllocBytes(size_t bytes, size_t alignment);

    template<typename T>
    T* Alloc(size_t count);

    void Reset();
};

}
