#pragma once
#include "immintrin.h"
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
   auto bits = sizeof(T) / sizeof(std::uint8_t) * 8 * size;
   if (bits <= 128)
      return 128;
#ifdef __AVX__
   /*else*/ if (bits <= 256)
      return 256;
#ifdef __AVX512F__
   else if (bits <= 512)
      return 512;
   else if (bits <= 1024)
      return 1024;
#endif // __AVX512F__
#endif // __AVX__
   else
      return 10240;
}

template<typename T>
constexpr bool can_vectorize(std::size_t size) noexcept {
   return detail::vector_size<T>(size) <= 512;
}

template <std::size_t Size>
struct vector;

template <>
struct vector<128> {
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
struct vector<256> {
   template<typename T, std::size_t Size>
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
struct vector<512> {
   __m512i data;
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

} // namespace detail

template <std::integral T, std::size_t Size>
class vector : detail::vector<detail::vector_size<T>(Size)> {
   using base_t = detail::vector<detail::vector_size<T>(Size)>;
 public:
   using size_type = std::uint8_t;
   template <std::size_t N>
   requires (N <= Size)
   vector(std::span<T const, N> v) noexcept
       : base_t{base_t::load(v)}
       , size_{static_cast<size_type>(std::span{v}.size())}{
   }
   template <std::size_t N>
   requires (N <= Size)
   vector(std::array<T, N> const& v) noexcept : vector(std::span<T const, N>{v.data(), N}) {}

   constexpr size_type size() const noexcept { return size_; }
   static constexpr size_type capacity() noexcept { return static_cast<size_type>(Size); }
   auto find_first(T value) noexcept { return detail::find_first(base_t::data, value); }
 private:
   size_type size_ = 0;
};

template <class T, std::size_t N>
vector(std::span<T, N>) -> vector<T, detail::vector_size<T>(N) / (sizeof(T) * 8)>;

template <class T, std::size_t N>
vector(std::array<T, N>) -> vector<T, detail::vector_size<T>(N)  / (sizeof(T) * 8)>;

template <class T, std::size_t N>
vector(T (&)[N]) -> vector<T, detail::vector_size<T>(N) / (sizeof(T) * 8)>;

}}} // namespace utils::v1::simd
