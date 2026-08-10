#pragma once
#include <array>
#include <span>
#if !defined(_MSC_VER) || defined(__clang__)

#include <cpuid.h>

namespace utils {inline namespace v1 {

using cpuid_t = std::array<std::uint32_t, 4>;

inline void cpuid(int function_id, std::span<typename cpuid_t::value_type, std::tuple_size_v<cpuid_t>> res) noexcept {
   __cpuid(function_id, res[0], res[1], res[2], res[3]);
}

inline auto max_leaf() noexcept {
   return __get_cpuid_max(0, NULL);
}

inline cpuid_t cpuid(int function_id = 0) noexcept;        

#else

namespace utils {
inline namespace v1 {

using cpuid_t = std::array<int, 4>;
inline void cpuid(int function_id, std::span<typename cpuid_t::value_type, std::tuple_size_v<cpuid_t>> res) noexcept {
   __cpuid(res.data(), function_id);
}

inline cpuid_t cpuid(int function_id = 0) noexcept;
           
inline auto max_leaf() noexcept {
   auto cpuinfo = cpuid(0);
   return cpuinfo[0];
}

#endif

inline cpuid_t cpuid(int function_id /* = 0*/) noexcept {
   auto res = cpuid_t{};
   cpuid(function_id, res);
   return res;
}

}} // namespace utils::v1
