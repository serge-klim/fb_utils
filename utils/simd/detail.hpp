#pragma once
#include <cstdint>
#include <cstddef>

namespace utils { inline namespace v1 { namespace simd {

template <typename T>
constexpr std::size_t bit_size(std::size_t size = 1) noexcept {
   return sizeof(T) / sizeof(std::uint8_t) * 8 * size;
}

namespace detail {

template <std::size_t Size>
struct vector;

template <typename T, typename U>
int find_first(T data, U value);

} // namespace detail
            
}}} // namespace utils::v1::simd
