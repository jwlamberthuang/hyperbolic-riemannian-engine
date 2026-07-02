#include <engine/trajectory_store.hpp>

#include <array>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <typename Callable>
void require_throws_invalid_argument(Callable&& callable, const char* message) {
    try {
        callable();
    } catch (const std::invalid_argument&) {
        return;
    }

    throw std::runtime_error(message);
}

void require_span_equals(std::span<const float> actual,
                         std::span<const float> expected,
                         const std::string& label) {
    require(actual.size() == expected.size(), (label + " size mismatch").c_str());

    for (std::size_t index = 0; index < actual.size(); ++index) {
        require(actual[index] == expected[index], (label + " value mismatch").c_str());
    }
}

} // namespace

void run_trajectory_store_tests() {
    engine::TrajectoryStore single_line_store(/*raw_dimensions=*/15, /*initial_capacity=*/2);

    require(single_line_store.get_math_dimensions() == 15, "raw dimension should be retained");
    require(single_line_store.get_storage_dimensions() == 16, "Lorentz storage should be d + 1");
    require(single_line_store.get_stride() == 16, "d=15 should exactly fill one 64-byte cache line");
    require(single_line_store.capacity() == 2, "initial capacity should be expressed in vectors");
    require(single_line_store.num_vectors() == 0, "new store should start empty");
    require(single_line_store.is_aligned(), "base buffer should be 64-byte aligned");

    const std::array<float, 16> first_vector = {
        1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F,
        9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
    const std::array<float, 16> second_vector = {
        17.0F, 18.0F, 19.0F, 20.0F, 21.0F, 22.0F, 23.0F, 24.0F,
        25.0F, 26.0F, 27.0F, 28.0F, 29.0F, 30.0F, 31.0F, 32.0F};

    single_line_store.insert_vector(first_vector);
    single_line_store.insert_vector(second_vector);

    require(single_line_store.num_vectors() == 2, "insert should increase vector count");
    require_span_equals(single_line_store.get_vector(0), first_vector, "first vector");
    require_span_equals(single_line_store.get_vector(1), second_vector, "second vector");

    const auto* first_address = single_line_store.get_vector(0).data();
    const auto* second_address = single_line_store.get_vector(1).data();
    require(second_address - first_address == static_cast<std::ptrdiff_t>(single_line_store.get_stride()),
            "vectors should be separated by exactly one stride");
    require(reinterpret_cast<std::uintptr_t>(first_address) % engine::TrajectoryStore::kAlignmentBytes == 0,
            "first vector should start on a cache-line boundary");
    require(reinterpret_cast<std::uintptr_t>(second_address) % engine::TrajectoryStore::kAlignmentBytes == 0,
            "second vector should start on a cache-line boundary");

    engine::TrajectoryStore padded_store(/*raw_dimensions=*/3, /*initial_capacity=*/1);
    require(padded_store.get_storage_dimensions() == 4, "storage dimension should include x0");
    require(padded_store.get_stride() == 16, "small vectors should be padded to one cache line");

    const std::array<float, 4> short_vector = {1.0F, 2.0F, 3.0F, 4.0F};
    const std::array<float, 4> grown_vector = {5.0F, 6.0F, 7.0F, 8.0F};
    padded_store.insert_vector(short_vector);
    padded_store.insert_vector(grown_vector);

    require(padded_store.capacity() >= 2, "store should grow when capacity is exhausted");
    require(padded_store.is_aligned(), "grown base buffer should remain aligned");
    require_span_equals(padded_store.get_vector(0), short_vector, "short vector");
    require_span_equals(padded_store.get_vector(1), grown_vector, "grown vector");

    const auto padded_view = padded_store.get_padded_vector(0);
    require(padded_view.size() == 16, "padded view should expose full stride");
    for (std::size_t index = padded_store.get_storage_dimensions(); index < padded_view.size(); ++index) {
        require(padded_view[index] == 0.0F, "padding should be zero-filled");
    }

    require_throws_invalid_argument(
        [&padded_store] {
            const std::array<float, 3> wrong_size = {1.0F, 2.0F, 3.0F};
            padded_store.insert_vector(wrong_size);
        },
        "inserting a non d+1 vector should fail");
}
