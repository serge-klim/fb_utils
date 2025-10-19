#pragma once
#if defined(__i386__) || defined(__x86_64__)
#include "x64/vector.hpp"
#else
#include "not_supported/vector.hpp"
#endif 
#include "detail.hpp"
#include <span>
#include <array>
#include <cstring>
#include <concepts>
#include <algorithm>
#include <type_traits>
#include <cstdint>

namespace utils { inline namespace v1 { namespace simd {
         


template <std::integral T, std::size_t Size>
class vector : detail::vector<detail::vector_size<T>(Size)> {
   using base_t = detail::vector<detail::vector_size<T>(Size)>;
 public:
   using size_type = std::uint8_t;

   vector() = default;

   template <std::size_t N>
   requires (N <= Size)
   vector(std::span<T const, N> v) noexcept
       : base_t{base_t::load(v)}
       , size_{static_cast<size_type>(std::span{v}.size())}{
   }
   template <std::size_t N>
   requires (N <= Size)
   vector(std::array<T, N> const& v) noexcept : vector(std::span<T const, N>{v.data(), N}) {}

   template <std::size_t N>
   vector(T(&v)[N]) noexcept : vector(std::span<T const, N>{v,}) {}

   template <std::size_t N>
   requires(N <= Size)
   vector& operator = (std::span<T const, N> v) noexcept {
      base_t::data = base_t::load(v);
      size_ = static_cast<size_type>(std::span{v}.size());
      return *this;
   }

   constexpr size_type size() const noexcept { return size_; }
   static constexpr size_type capacity() noexcept { return base_t::template capacity<T>(); }
   auto find_first(T value) noexcept { return detail::find_first(base_t::data, value); }
 private:
   size_type size_ = 0;
};

template <class T, std::size_t N>
vector(std::span<T, N>) -> vector<T, (detail::vector_size<T>(N) * sizeof(std::uint8_t))/ (sizeof(T) * 8)>;

template <class T, std::size_t N>
vector(std::array<T, N>) -> vector<T, (detail::vector_size<T>(N) * sizeof(std::uint8_t))/ (sizeof(T) * 8)>;

template <class T, std::size_t N>
vector(T (&)[N]) -> vector<T, (detail::vector_size<T>(N) * sizeof(std::uint8_t))/ (sizeof(T) * 8)>;

template <typename T>
constexpr bool can_vectorize(std::size_t size) noexcept {
   return detail::vector_size<T>(size) <= 512;
}
template <typename... T>
   requires(std::is_integral_v<std::common_type_t<T...>> && can_vectorize<std::common_type_t<T...>>(sizeof...(T)))
auto make_vector(T&&... v) {
   using value_type = std::common_type_t<T...>;
   constexpr auto byte_size = detail::vector_size<value_type>(sizeof...(T)) / 8;
   auto ids = std::array<std::common_type_t<T...>, byte_size * sizeof(std::uint8_t) / sizeof(value_type)>{std::forward<T>(v)...};
   return ::utils::simd::vector{ids};
}

template <std::size_t BitSize, typename... T>
   requires(std::is_integral_v<std::common_type_t<T...>>)
auto make_vector(T&&... v) {
   using value_type = std::common_type_t<T...>;
   constexpr auto byte_size = (std::max)(detail::vector_size<value_type>(sizeof...(T)), BitSize) / 8;
   static_assert(can_vectorize<std::uint8_t>(byte_size),
                 "too big to fit in avalible simd vector");
   auto ids = std::array<std::common_type_t<T...>, byte_size * sizeof(std::uint8_t) / sizeof(value_type)>{std::forward<T>(v)...};
   return ::utils::simd::vector{ids};
}


            
}}} // namespace utils::v1::simd
