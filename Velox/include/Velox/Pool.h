#pragma once

namespace Velox {

template<typename T>
struct Pool {
    struct FreeNode {
        FreeNode* next;
    };

    char*     buffer;
    FreeNode* freeList;
    size_t    size;

    Pool(size_t count)
        : size(count)
    {
        assert(sizeof(T) >= sizeof(void*) && "ERROR: T too small for for free list node");

        buffer = static_cast<char*>(malloc(size * sizeof(T)));
        assert(buffer && "ERROR: Allocation for buffer failed");

        freeList = reinterpret_cast<FreeNode*>(buffer);

        for (size_t index = 0; index < size - 1; index++)
        {
            FreeNode* current = reinterpret_cast<FreeNode*>(buffer +  index      * sizeof(T));
            FreeNode* next    = reinterpret_cast<FreeNode*>(buffer + (index + 1) * sizeof(T));

            current->next = next;
        }

        // Don't forget about the last node.
        reinterpret_cast<FreeNode*>(buffer + (size - 1) * sizeof(T))->next = nullptr;
    }

    ~Pool()
    {
        free(buffer);
    }

    T* alloc()
    {
        if (!freeList)
            return nullptr;

        void* result = freeList;
        freeList = freeList->next;
        return static_cast<T*>(result);
    }

    void free(T* object)
    {
        if (!object)
            return;

        FreeNode* node = reinterpret_cast<FreeNode*>(object);
        node->next = freeList;
        freeList = node;
    }
};

}
