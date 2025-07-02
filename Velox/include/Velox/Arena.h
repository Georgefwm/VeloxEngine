#pragma once

#include "Velox.h"

namespace Velox {

struct VELOX_API Arena {
    size_t size;
    size_t offset;
    char*  buffer;

    explicit Arena(size_t bytes);
    ~Arena();

    void* allocBytes(size_t bytes, size_t alignment);

    template<typename T>
    T* alloc(size_t count)
    {
        void* memory = allocBytes(sizeof(T) * count, alignof(T));
        return static_cast<T*>(memory);
    }

    void reset();
    void printUsage();
};

}
