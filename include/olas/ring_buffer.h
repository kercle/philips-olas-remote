#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <optional>
#include <type_traits>

namespace detail {

#ifdef ESP8266

class SpscIndex {
public:
    uint32_t IRAM_ATTR load_relaxed() const
    {
        return value;
    }

    uint32_t IRAM_ATTR load_acquire() const
    {
        const uint32_t result = value;

        // Do not move subsequent memory accesses before this load.
        std::atomic_signal_fence(std::memory_order_acquire);

        return result;
    }

    void IRAM_ATTR store_release(uint32_t new_value)
    {
        // Do not move preceding memory accesses after publication.
        std::atomic_signal_fence(std::memory_order_release);

        value = new_value;
    }

private:
    volatile uint32_t value = 0;
};

#else

static_assert(
    std::atomic<uint32_t>::is_always_lock_free,
    "RingBuffer requires lock-free 32-bit atomics");

class SpscIndex {
public:
    uint32_t IRAM_ATTR load_relaxed() const
    {
        return value.load(std::memory_order_relaxed);
    }

    uint32_t IRAM_ATTR load_acquire() const
    {
        return value.load(std::memory_order_acquire);
    }

    void IRAM_ATTR store_release(uint32_t new_value)
    {
        value.store(new_value, std::memory_order_release);
    }

private:
    std::atomic<uint32_t> value { 0 };
};

#endif

}

template <typename T, size_t N>
class RingBuffer {
    static_assert(N > 0);
    static_assert((N & (N - 1)) == 0);
    static_assert(std::is_trivially_copyable_v<T>);

    static constexpr size_t mask = N - 1;

public:
    bool IRAM_ATTR push(const T& value)
    {
        const uint32_t write = write_index.load_relaxed();
        const uint32_t read = read_index.load_acquire();

        if (write - read == N) {
            return false;
        }

        data[write & mask] = value;

        write_index.store_release(write + 1);

        return true;
    }

    std::optional<T> pop()
    {
        const uint32_t read = read_index.load_relaxed();
        const uint32_t write = write_index.load_acquire();

        if (read == write) {
            return std::nullopt;
        }

        const T value = data[read & mask];

        read_index.store_release(read + 1);

        return value;
    }

private:
    std::array<T, N> data { };

    detail::SpscIndex write_index;
    detail::SpscIndex read_index;
};
