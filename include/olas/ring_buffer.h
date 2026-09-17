#pragma once

#include <atomic>
#include <optional>
#include <stdlib.h>

// Single-producer, single-consumer ring buffer.
template <typename T, size_t N>
class RingBuffer {
    static_assert(N > 0);
    static_assert((N & (N - 1)) == 0);

    static constexpr size_t mask = N - 1;

public:
    bool push(const T& value)
    {
        const uint32_t write = write_index.load(std::memory_order_relaxed);
        const uint32_t read = read_index.load(std::memory_order_acquire);

        if (write - read == N) {
            return false;
        }

        data[write & mask] = value;

        write_index.store(
            write + 1,
            std::memory_order_release);

        return true;
    }

    std::optional<T> pop()
    {
        const uint32_t read = read_index.load(std::memory_order_relaxed);
        const uint32_t write = write_index.load(std::memory_order_acquire);

        if (read == write) {
            return std::nullopt;
        }

        const auto value = data[read & mask];
        read_index.store(
            read + 1,
            std::memory_order_release);

        return value;
    }

private:
    std::array<T, N> data { };

    std::atomic<uint32_t> write_index = 0;
    std::atomic<uint32_t> read_index = 0;
};
