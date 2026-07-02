#ifndef HYPERBOLIC_ENGINE_TRAJECTORY_STORE_HPP
#define HYPERBOLIC_ENGINE_TRAJECTORY_STORE_HPP

#include <cstddef>
#include <limits>
#include <new>
#include <span>
#include <vector>

namespace engine {

template <typename T, std::size_t Alignment>
class AlignedAllocator {
public:
    using value_type = T;

    static_assert(Alignment >= alignof(T));
    static_assert((Alignment & (Alignment - 1)) == 0);

    AlignedAllocator() noexcept = default;

    template <typename U>
    constexpr AlignedAllocator(const AlignedAllocator<U, Alignment>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t count) {
        if (count > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
            throw std::bad_array_new_length{};
        }

        return static_cast<T*>(
            ::operator new(count * sizeof(T), std::align_val_t{Alignment}));
    }

    void deallocate(T* pointer, std::size_t) noexcept {
        ::operator delete(pointer, std::align_val_t{Alignment});
    }

    template <typename U>
    struct rebind {
        using other = AlignedAllocator<U, Alignment>;
    };
};

template <typename T, typename U, std::size_t Alignment>
constexpr bool operator==(const AlignedAllocator<T, Alignment>&,
                          const AlignedAllocator<U, Alignment>&) noexcept {
    return true;
}

template <typename T, typename U, std::size_t Alignment>
constexpr bool operator!=(const AlignedAllocator<T, Alignment>&,
                          const AlignedAllocator<U, Alignment>&) noexcept {
    return false;
}

class TrajectoryStore {
public:
    static constexpr std::size_t kCacheLineBytes = 64;
    static constexpr std::size_t kAlignmentBytes = kCacheLineBytes;
    static constexpr std::size_t kFloatsPerCacheLine = kCacheLineBytes / sizeof(float);

    using Buffer = std::vector<float, AlignedAllocator<float, kAlignmentBytes>>;

    // raw_dimensions is the hyperbolic dimension d. Stored Lorentz points contain d + 1 floats.
    TrajectoryStore(std::size_t raw_dimensions, std::size_t initial_capacity = 1000);

    ~TrajectoryStore() = default;

    // Copies one Lorentz point containing exactly d + 1 coordinates. Padding is zero-filled.
    void insert_vector(std::span<const float> vector_data);

    // Returns a zero-copy read-only span over the mathematical coordinates, excluding padding.
    [[nodiscard]] std::span<const float> get_vector(std::size_t index) const;
    [[nodiscard]] std::span<const float> get_padded_vector(std::size_t index) const;

    // Raw sizing metrics for profiling
    [[nodiscard]] std::size_t num_vectors() const noexcept { return num_vectors_; }
    [[nodiscard]] std::size_t get_stride() const noexcept { return stride_; }
    [[nodiscard]] std::size_t get_math_dimensions() const noexcept { return math_dimensions_; }
    [[nodiscard]] std::size_t get_storage_dimensions() const noexcept { return storage_dimensions_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return buffer_.size() / stride_; }
    [[nodiscard]] const float* data() const noexcept { return buffer_.data(); }
    [[nodiscard]] float* data() noexcept { return buffer_.data(); }
    [[nodiscard]] bool is_aligned() const noexcept;

private:
    std::size_t math_dimensions_; // The raw 'd'
    std::size_t storage_dimensions_; // The 'd + 1' required by Lorentz
    std::size_t stride_;             // The final padded element count, in floats
    std::size_t num_vectors_ = 0;    // Number of elements currently active

    // The single contiguous storage block
    Buffer buffer_;

    // Helper to calculate the cache-aligned stride during initialization
    static std::size_t calculate_aligned_stride(std::size_t storage_dims);
};

} // namespace engine

#endif // HYPERBOLIC_ENGINE_TRAJECTORY_STORE_HPP
