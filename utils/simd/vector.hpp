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

template <std::size_t SizeOf, std::size_t N = 1>
constexpr std::size_t vector_size() noexcept {
   constexpr auto bits =  SizeOf/sizeof(std::uint8_t) * 8 * N;
   if constexpr (bits <= 128)
      return 128;
#ifdef __AVX__
   else if constexpr (bits <= 256)
      return 256;
#ifdef __AVX512F__
   else if constexpr (bits <= 512)
      return 512;
   else if constexpr (bits <= 1024)
      return 1024;
#endif // __AVX512F__
#endif // __AVX__
   else
      return 10240;
}

template <std::size_t Size>
struct vector;

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
class vector : detail::vector<detail::vector_size<sizeof(T), Size>()> {
   using base_t = detail::vector<detail::vector_size<sizeof(T), Size>()>;
 public:
   using size_type = std::uint8_t;
   template <typename V>
   requires requires(V const& v) { std::span{v}; }
   vector(V const& v) noexcept
       : base_t{base_t::load(std::span{v})}
       , size_{static_cast<size_type>(std::span{v}.size())}{
   }
   constexpr size_type size() const noexcept { return size_; }
   static constexpr size_type capacity() noexcept { return static_cast<size_type>(Size); }
   auto find_first(T value) noexcept { return detail::find_first(base_t::data, value); }
 private:
   size_type size_ = 0;
};

template <class T, std::size_t Size>
vector(std::array<T, Size>) -> vector<T, detail::vector_size<sizeof(T), Size>() / (sizeof(T) * 8)>;


    //vector<T, detail::vector_size<Size * (sizeof(T) * 8 / sizeof(std::uint8_t))>() / (sizeof(T) * 8)>;

// template <typename T, std::size_t Size>
//    requires(Size * sizeof(T) < sizeof(__m256i))
// class searcher {
//  public:
//    searcher(std::span<T, Size> s)
//        : values_{load(s)} {
//    }
//    static __m256i load(std::span<T, Size> const& s) {
//       if constexpr (Size == sizeof(__m256i))
//          return _mm256_loadu_si256(static_cast<__m256i const*>(s.data()));
//       else {
//          auto tmp = std::array<std::byte, 256 / 8>{};
//          std::memcpy(tmp.data(), s.data(), Size * sizeof(T) / sizeof(std::byte));
//          return _mm256_loadu_si256(reinterpret_cast<__m256i const*>(tmp.data()));
//       }
//    }
//    auto raw(T value) noexcept {
//       auto comparand = _mm256_set1_epi16(static_cast<short>(value));
//       auto const res = static_cast<unsigned int>(_mm256_movemask_epi8(_mm256_cmpeq_epi16(values_, comparand)));
//       return _tzcnt_u32(res) * sizeof(std::byte) / sizeof(T);
//    }
//
//  private:
//    __m256i values_;
// };
//
// template <class T, std::size_t N>
// searcher(std::array<T, N>) -> searcher<T, N>;

// template<typename T, std::size_t Size>
// requires(Size * sizeof(T) / sizeof(std::uint8_t) * 8 < 512)
// void find(std::span<T, Size> s, T&& value) {
//    return searcher<T, Size>{s}(value);
// }

}}} // namespace utils::v1::simd
