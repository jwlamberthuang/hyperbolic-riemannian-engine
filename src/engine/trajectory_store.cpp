#include <engine/trajectory_store.hpp>

#include <algorithm>
#include <cstdint>
#include <stdexcept>

namespace engine {

TrajectoryStore::TrajectoryStore(std::size_t raw_dimensions, std::size_t initial_capacity)
    : math_dimensions_(raw_dimensions),
      storage_dimensions_(raw_dimensions + 1),
      stride_(calculate_aligned_stride(storage_dimensions_)),
      buffer_(initial_capacity * stride_, 0.0F) {
    if (raw_dimensions == 0) {
        throw std::invalid_argument("TrajectoryStore requires at least one hyperbolic dimension");
    }
}

void TrajectoryStore::insert_vector(std::span<const float> vector_data) {
    if (vector_data.size() != storage_dimensions_) {
        throw std::invalid_argument("insert_vector expects exactly d + 1 Lorentz coordinates");
    }

    if (num_vectors_ == capacity()) {
        const std::size_t next_capacity = capacity() == 0 ? 1 : capacity() * 2;
        buffer_.resize(next_capacity * stride_, 0.0F);
    }

    const std::size_t offset = num_vectors_ * stride_;
    std::fill(buffer_.begin() + static_cast<std::ptrdiff_t>(offset),
              buffer_.begin() + static_cast<std::ptrdiff_t>(offset + stride_),
              0.0F);
    std::copy(vector_data.begin(), vector_data.end(), buffer_.begin() + static_cast<std::ptrdiff_t>(offset));
    ++num_vectors_;
}

std::span<const float> TrajectoryStore::get_vector(std::size_t index) const {
    if (index >= num_vectors_) {
        throw std::out_of_range("TrajectoryStore vector index is out of range");
    }

    return {buffer_.data() + (index * stride_), storage_dimensions_};
}

std::span<const float> TrajectoryStore::get_padded_vector(std::size_t index) const {
    if (index >= num_vectors_) {
        throw std::out_of_range("TrajectoryStore vector index is out of range");
    }

    return {buffer_.data() + (index * stride_), stride_};
}

bool TrajectoryStore::is_aligned() const noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(buffer_.data());
    return address % kAlignmentBytes == 0;
}

std::size_t TrajectoryStore::calculate_aligned_stride(std::size_t storage_dims) {
    const std::size_t remainder = storage_dims % kFloatsPerCacheLine;
    if (remainder == 0) {
        return storage_dims;
    }
    return storage_dims + (kFloatsPerCacheLine - remainder);
}

} // namespace engine
