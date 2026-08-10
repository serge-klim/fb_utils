#pragma once
#include <immintrin.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif
#include <cstdint>

namespace utils { inline namespace v1 {

struct cpu_clock {
   static auto now() noexcept {
      _mm_mfence();
      return __rdtsc();
   }
   static auto raw_now() noexcept {
      return __rdtsc();
   }
};

std::uint64_t tsc_frequency();
std::uint64_t system_tsc_freq() noexcept;
std::uint64_t estimate_cpu_frequency();

}} // namespace utils::v1
