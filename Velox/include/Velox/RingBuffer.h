#pragma once

#include <Velox.h>

namespace Velox {

template<typename T>
struct VELOX_API RingBuffer {
    std::vector<T> buffer;
    size_t head = 0;

    explicit RingBuffer(size_t capacity)
        : buffer(capacity) {}

    explicit RingBuffer(size_t capacity, const T& item)
        : buffer(capacity, item) {}

    T& operator[](size_t index)
    {
        assert(index >= 0 && index < buffer.size());

        return buffer[index];
    }

    void assign(const T& item, bool resetHead = false)
    {
        buffer = std::vector<T>(buffer.size(), item);

        if (resetHead)
            head = 0;
    }

    void push(const T& item)
    {
        buffer[head] = item;
        head = head + 1 % buffer.size() - 1;
    }

    void pop()
    {
        head = head - 1 < 0 ? buffer.size() : head - 1;
    }

    size_t Size() { return buffer.size(); }
};

}
