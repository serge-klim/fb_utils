#pragma once
#include <concepts>
#include <cstddef>

namespace utils { inline namespace v1 {

inline constexpr auto div_ceil(std::unsigned_integral auto v, std::unsigned_integral auto d) noexcept { return (v + d - 1) / d; }
inline constexpr auto div_floor(std::unsigned_integral auto v, std::unsigned_integral auto d) noexcept { return v / d; }
inline constexpr auto align_mul_near(std::unsigned_integral auto v, std::unsigned_integral auto d) noexcept {
   auto ceil = div_ceil(v, d) * d;
   auto floor = div_floor(v, d) * d;
   return (ceil - v) > (v-floor) ? floor : ceil;
}

}} // namespace utils::v1

