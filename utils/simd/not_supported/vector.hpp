#pragma once
#include <limits>
#include <cstddef>

namespace utils { inline namespace v1 { namespace simd {

namespace detail {

template <typename T>
constexpr std::size_t vector_size(std::size_t size = 1) noexcept {
   return (std::numeric_limits<std::size_t>::max)();
}

template <typename T, typename U>
int find_first(T data, U value);

} // namespace detail
            
}}} // namespace utils::v1::simd
