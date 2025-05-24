#include "Arena.h"

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>

Velox::Arena::Arena(size_t bytes)
    : size(bytes), buffer(static_cast<char*>(malloc(bytes))), offset(0)
{
    assert(buffer && "Failed to initialise buffer :(");
}

Velox::Arena::~Arena()
{
    free(buffer);
}

void* Velox::Arena::AllocBytes(size_t bytes, size_t alignment = alignof(max_align_t))
{
    size_t current = reinterpret_cast<size_t>(buffer + offset);
    size_t aligned = (current + alignment - 1) & ~(alignment - 1);
    size_t adjustment = aligned - current;

    if (offset + adjustment + bytes > size)
    {
        // No memory left in arena.
        printf("WARNING: Arena out of memory!\n");

        return nullptr;
    }

    offset += adjustment;

    void* ptr = buffer + offset;
    offset += bytes;

    return ptr;
}

template<typename T>
T* Alloc(size_t count = 1)
{
    void* memory = allocateRaw(sizeof(T) * count, alignof(T));

    return static_cast<T*>(memory);
}

void Velox::Arena::Reset()
{
    offset = 0;
}

