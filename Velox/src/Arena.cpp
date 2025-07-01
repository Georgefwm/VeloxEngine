#include "Arena.h"
#include <PCH.h>


Velox::Arena::Arena(size_t bytes)
    : size(bytes), buffer(static_cast<char*>(malloc(bytes))), offset(0)
{
    assert(buffer && "Failed to initialise buffer :(");
}

Velox::Arena::~Arena()
{
    free(buffer);
}

void* Velox::Arena::allocBytes(size_t bytes, size_t alignment = alignof(max_align_t))
{
    size_t current = reinterpret_cast<size_t>(buffer + offset);
    size_t aligned = (current + alignment - 1) & ~(alignment - 1);
    size_t adjustment = aligned - current;

    if (offset + adjustment + bytes > size)
    {
        LOG_ERROR("Arena out of memory");
        return nullptr;
    }

    offset += adjustment;

    void* ptr = buffer + offset;
    offset += bytes;

    // Sometimes helpful.
    // printf("Arena: Added %zu bytes, now using %zu/%zu bytes (%c%.1f)\n",
    //         adjustment + bytes, offset, size, '%', ((float)offset / size) * 100);

    return ptr;
}

void Velox::Arena::reset()
{
    offset = 0;
}

void Velox::Arena::printUsage()
{
    printf("Arena using %zu / %zu bytes\n", offset, size);
}

