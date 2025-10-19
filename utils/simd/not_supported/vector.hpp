#pragma once
#include <cstddef>

namespace utils { inline namespace v1 { namespace simd {

namespace detail {

template <typename T>
constexpr std::size_t vector_size(std::size_t size = 1) noexcept {
   return 1024 * 10;
}

template <typename T, typename U>
int find_first(T data, U value);

} // namespace detail
            
}}} // namespace utils::v1::simd
