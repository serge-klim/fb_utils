#include "utils/simd/vector.hpp"
#include "benchmark/benchmark.h"
#include "immintrin.h"
#include <bit>
#include <array>
#include <random>
#include <cstdint>


#include <numeric>
#include <ranges>

template <typename T, std::size_t Size>
std::array<T, Size> const& test_set() {
   static auto data = [] {
      auto res = std::array<T, Size>{};
      std::iota(begin(res), end(res), static_cast<T>(101));
      std::shuffle(begin(res), end(res), std::mt19937{std::random_device{}()});
      return res;
   }();
   return data;
}

template <typename T, std::size_t Size>
requires(Size * sizeof(T)/sizeof(std::uint8_t) * 8 <= 512)
void avx_bm(benchmark::State& state) {
   using value_type = T;
   auto lookup = test_set<value_type, Size>();
   auto values = lookup;
   std::ranges::shuffle(values, std::mt19937{std::random_device{}()});
   auto vector = utils::simd::vector{values};
   auto i = lookup.size() - lookup.size();
   auto sum = value_type{0};
   for (auto _ : state) {
      benchmark::DoNotOptimize(sum += vector.find_first(lookup[i++ % lookup.size()])); //
   }
}

BENCHMARK(avx_bm<std::uint8_t, 15>);
BENCHMARK(avx_bm<unsigned short, 7>);
BENCHMARK(avx_bm<std::uint64_t, 2>);

BENCHMARK(avx_bm<std::uint8_t, 20>);
BENCHMARK(avx_bm<unsigned short, 10>);
BENCHMARK(avx_bm<std::uint64_t, 4>);

BENCHMARK(avx_bm<std::uint8_t, 61>);
BENCHMARK(avx_bm<unsigned short, 31>);
BENCHMARK(avx_bm<std::uint64_t, 7>);