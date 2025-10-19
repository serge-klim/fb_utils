#pragma once
#include "../detail.hpp"
#include <immintrin.h>
#include <span>
#include <array>
#include <cstring>
#include <concepts>
#include <limits>
#include <cstddef>
#include <cstdint>

namespace utils { inline namespace v1 { namespace simd {
          
namespace detail {

template <typename T>
constexpr std::size_t vector_size(std::size_t size = 1) noexcept {
   auto bits = bit_size<T>(size);
#if (defined(__SSE__) || defined(__AVX__)) && (!defined(__GNUC__) || defined(__BMI__))
// gcc requires  -mbmi https://gcc.gnu.org/bugzilla/show_bug.cgi?id=110921
   if (bits <= 128)
      return 128;
#endif // __SSE__
#ifdef __AVX2__
   if (bits <= 256)
      return 256;
#endif // __AVX2__
#ifdef __AVX512BW__
   if (bits <= 512)
      return 512;
   if (bits <= 1024)
      return 1024;
#endif // __AVX512BW__
     return 1024 * 10;
}

template <std::size_t Size>
struct vector;

template <typename T>
   requires(std::is_integral_v<T> && sizeof(T) == sizeof(std::uint8_t))
std::uint8_t find_first(__m128i& data, T value) noexcept {
   auto comparand = _mm_set1_epi8(static_cast<char>(value));
   auto const res = static_cast<unsigned int>(_mm_movemask_epi8(_mm_cmpeq_epi8(data, comparand)));
   return res != 0 ? static_cast<std::uint8_t>(_tzcnt_u32(res) * sizeof(std::byte) / sizeof(T)) : (std::numeric_limits<std::uint8_t>::max)();
}

template <typename T>
   requires(std::is_integral_v<T> && sizeof(T) == sizeof(std::uint16_t))
std::uint8_t find_first(__m128i& data, T value) noexcept {
   auto comparand = _mm_set1_epi16(static_cast<short>(value));
   auto const res = static_cast<unsigned int>(_mm_movemask_epi8(_mm_cmpeq_epi16(data, comparand)));
   return res != 0 ? static_cast<std::uint8_t>(_tzcnt_u32(res) * sizeof(std::byte) / sizeof(T)) : (std::numeric_limits<std::uint8_t>::max)();
}

template <typename T>
   requires(std::is_integral_v<T> && sizeof(T) == sizeof(std::uint32_t))
std::uint8_t find_first(__m128i& data, T value) noexcept {
   auto comparand = _mm_set1_epi32(static_cast<int>(value));
   auto const res = static_cast<unsigned int>(_mm_movemask_epi8(_mm_cmpeq_epi32(data, comparand)));
   return res != 0 ? static_cast<std::uint8_t>(_tzcnt_u32(res) * sizeof(std::byte) / sizeof(T)) : (std::numeric_limits<std::uint8_t>::max)();
}

template <typename T>
   requires(std::is_integral_v<T> && sizeof(T) == sizeof(std::uint64_t))
std::uint8_t find_first(__m128i& data, T value) noexcept {
   auto comparand = _mm_set1_epi64x(static_cast<long long>(value));
   auto const res = static_cast<unsigned int>(_mm_movemask_epi8(_mm_cmpeq_epi64(data, comparand)));
   return res != 0 ? static_cast<std::uint8_t>(_tzcnt_u32(res) * sizeof(std::byte) / sizeof(T)) : (std::numeric_limits<std::uint8_t>::max)();
}

template <>
struct vector<128> {
   template <typename T>
   static constexpr std::size_t capacity() noexcept { return (128 / 8) / (sizeof(T) / sizeof(std::uint8_t)); }
   template <typename T, std::size_t Size>
   static auto load(std::span<T, Size> const& s) {
      if constexpr (Size * sizeof(T) == sizeof(data))
         return _mm_loadu_si128(reinterpret_cast<__m128i const*>(s.data()));
      else {
         auto tmp = std::array<std::byte, 128 / 8>{};
         std::memcpy(tmp.data(), s.data(), Size * sizeof(T) / sizeof(std::byte));
         return _mm_loadu_si128(reinterpret_cast<__m128i const*>(tmp.data()));
      }
   }
   __m128i data;
};

template <typename T>
   requires(std::is_integral_v<T> && sizeof(T) == sizeof(std::uint8_t))
std::uint8_t find_first(__m256i& data, T value) noexcept {
   auto comparand = _mm256_set1_epi8(static_cast<char>(value));
   auto const res = static_cast<unsigned int>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(data, comparand)));
   return res != 0 ? static_cast<std::uint8_t>(_tzcnt_u32(res) * sizeof(std::byte) / sizeof(T)) : (std::numeric_limits<std::uint8_t>::max)();
}

template <typename T>
requires(std::is_integral_v<T> && sizeof(T) == sizeof(std::uint16_t))
std::uint8_t find_first(__m256i& data, T value) noexcept {
   auto comparand = _mm256_set1_epi16(static_cast<short>(value));
   auto const res = static_cast<unsigned int>(_mm256_movemask_epi8(_mm256_cmpeq_epi16(data, comparand)));
   return res != 0 ? static_cast<std::uint8_t>(_tzcnt_u32(res) * sizeof(std::byte) / sizeof(T)) : (std::numeric_limits<std::uint8_t>::max)();
}

template <typename T>
   requires(std::is_integral_v<T> && sizeof(T) == sizeof(std::uint32_t))
std::uint8_t find_first(__m256i& data, T value) noexcept {
   auto comparand = _mm256_set1_epi32(static_cast<int>(value));
   auto const res = static_cast<unsigned int>(_mm256_movemask_epi8(_mm256_cmpeq_epi32(data, comparand)));
   return res != 0 ? static_cast<std::uint8_t>(_tzcnt_u32(res) * sizeof(std::byte) / sizeof(T)) : (std::numeric_limits<std::uint8_t>::max)();
}

template <typename T>
   requires(std::is_integral_v<T> && sizeof(T) == sizeof(std::uint64_t))
std::uint8_t find_first(__m256i& data, T value) noexcept {
   auto comparand = _mm256_set1_epi64x(static_cast<long long>(value));
   auto const res = static_cast<unsigned int>(_mm256_movemask_epi8(_mm256_cmpeq_epi64(data, comparand)));
   return res != 0 ? static_cast<std::uint8_t>(_tzcnt_u32(res) * sizeof(std::byte) / sizeof(T)) : (std::numeric_limits<std::uint8_t>::max)();
}

template <>
struct vector<256> {
   template <typename T>
   static constexpr std::size_t capacity() noexcept { return (256 / 8) / (sizeof(T) / sizeof(std::uint8_t)); }
   template <typename T, std::size_t Size>
   static auto load(std::span<T, Size> const& s) {
      if constexpr (Size * sizeof(T) == sizeof(data))
         return _mm256_loadu_si256(reinterpret_cast<__m256i const*>(s.data()));
      else {
         auto tmp = std::array<std::byte, 256 / 8>{};
         std::memcpy(tmp.data(), s.data(), Size * sizeof(T) / sizeof(std::byte));
         return _mm256_loadu_si256(reinterpret_cast<__m256i const*>(tmp.data()));
      }
   }
   __m256i data;
};

template <typename T>
   requires(std::is_integral_v<T> && sizeof(T) == sizeof(std::uint8_t))
std::uint8_t find_first(__m512i& data, T value) noexcept {
   auto comparand = _mm512_set1_epi8(static_cast<char>(value));
   auto res = _mm512_cmpeq_epi8_mask(data, comparand);
   return res != 0 ? static_cast<std::uint8_t>(_tzcnt_u64(res)) : (std::numeric_limits<std::uint8_t>::max)();
}

template <typename T>
   requires(std::is_integral_v<T> && sizeof(T) == sizeof(std::uint16_t))
std::uint8_t find_first(__m512i& data, T value) noexcept {
   auto comparand = _mm512_set1_epi16(static_cast<short>(value));
   auto res = _mm512_cmpeq_epi16_mask(data, comparand);
   return res != 0 ? static_cast<std::uint8_t>(_tzcnt_u64(res)) : (std::numeric_limits<std::uint8_t>::max)();
}

template <typename T>
   requires(std::is_integral_v<T> && sizeof(T) == sizeof(std::uint32_t))
std::uint8_t find_first(__m512i& data, T value) noexcept {
   auto comparand = _mm512_set1_epi32(static_cast<short>(value));
   auto res = _mm512_cmpeq_epi32_mask(data, comparand);
   return res != 0 ? static_cast<std::uint8_t>(_tzcnt_u64(res)) : (std::numeric_limits<std::uint8_t>::max)();
}

template <typename T>
   requires(std::is_integral_v<T> && sizeof(T) == sizeof(std::uint64_t))
std::uint8_t find_first(__m512i& data, T value) noexcept {
   auto comparand = _mm512_set1_epi64(static_cast<short>(value));
   auto res = _mm512_cmpeq_epi64_mask(data, comparand);
   return res != 0 ? static_cast<std::uint8_t>(_tzcnt_u64(res)) : (std::numeric_limits<std::uint8_t>::max)();
}

template <>
struct vector<512> {
   template <typename T>
   static constexpr std::size_t capacity() noexcept { return (512 / 8) / (sizeof(T) / sizeof(std::uint8_t)); }
   template <typename T, std::size_t Size>
   static auto load(std::span<T, Size> const& s) {
      if constexpr (Size * sizeof(T) == sizeof(data))
         return _mm512_loadu_si512(reinterpret_cast<__m512i const*>(s.data()));
      else {
         auto tmp = std::array<std::byte, 512 / 8>{};
         std::memcpy(tmp.data(), s.data(), Size * sizeof(T) / sizeof(std::byte));
         return _mm512_loadu_si512(reinterpret_cast<__m512i const*>(tmp.data()));
      }
   }
   __m512i data;
};

} // namespace detail

            
}}} // namespace utils::v1::simd
