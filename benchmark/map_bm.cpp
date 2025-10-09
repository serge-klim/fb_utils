#include "benchmark/benchmark.h"
#include <chrono>
#include <numeric>
#include <algorithm>
#include <array>
#include <vector>
#include <unordered_set>
#include <set>
#include <ranges>
#include <numeric>
#include <random>
#include <type_traits>


template<typename T, std::size_t Size>
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
void vector_bm(benchmark::State& state) {
   using value_type= typename T::value_type;
   auto values = T{};
   auto const& lookup = test_set<value_type, Size>();
   values.reserve(lookup.size());
   std::ranges::copy(lookup, std::back_inserter(values));
   std::ranges::shuffle(values, std::mt19937{std::random_device{}()});
   auto i = lookup.size() - lookup.size();
   auto sum = value_type{0};
   for (auto _ : state) {
     benchmark::DoNotOptimize(sum += *std::ranges::find(values, lookup[i++ % lookup.size()]));//
   }
}

template <typename T, std::size_t Size>
void map_bm(benchmark::State& state) {
   using value_type = typename T::key_type;
   auto const& lookup = test_set<value_type, Size>();
   auto values = T{};
   //std::ranges::transform(lookup, std::inserter(values, end(values)));
   std::ranges::copy(std::views::transform(lookup, [](auto v) { return std::make_pair(v, static_cast<typename T::mapped_type>(v + v)); }), std::inserter(values, end(values)));
   auto i = lookup.size() - lookup.size();
   auto sum = typename T::mapped_type{0};
   for (auto _ : state) {
      //benchmark::DoNotOptimize(sum += *std::ranges::find(set, lookup[i++ % lookup.size()]));
      benchmark::DoNotOptimize(sum += values.find(lookup[i++ % lookup.size()])->second);
   }
}

BENCHMARK(vector_bm<std::vector<unsigned short>, 10>);
BENCHMARK(map_bm<std::unordered_map<unsigned short, long>, 10>);
BENCHMARK(map_bm<std::map<unsigned short, long>, 10>);
BENCHMARK(vector_bm<std::vector<unsigned short>, 64>);
BENCHMARK(map_bm<std::unordered_map<unsigned short, long>, 64>);
BENCHMARK(map_bm<std::map<unsigned short, long>, 64>);
BENCHMARK(vector_bm<std::vector<unsigned short>, 1024>);
BENCHMARK(map_bm<std::unordered_map<unsigned short, long>, 1024>);
BENCHMARK(map_bm<std::map<unsigned short, long>, 1024>);
BENCHMARK(vector_bm<std::vector<unsigned short>, 1024 * 100>);
BENCHMARK(map_bm<std::unordered_map<unsigned short, long>, 1024 * 100>);
BENCHMARK(map_bm<std::map<unsigned short, long>, 1024 * 100>);



BENCHMARK(vector_bm<std::vector<int>, 10>);
BENCHMARK(map_bm<std::unordered_map<int, long>, 10>);
BENCHMARK(map_bm<std::map<int, long>, 10>);
BENCHMARK(vector_bm<std::vector<int>, 64>);
BENCHMARK(map_bm<std::unordered_map<int, long>, 64>);
BENCHMARK(map_bm<std::map<int, long>, 64>);
BENCHMARK(vector_bm<std::vector<int>, 1024>);
BENCHMARK(map_bm<std::unordered_map<int, long>, 1024>);
BENCHMARK(map_bm<std::map<int, long>, 1024>);
BENCHMARK(vector_bm<std::vector<int>, 1024 * 100>);
BENCHMARK(map_bm<std::unordered_map<int, long>, 1024 * 100>);
BENCHMARK(map_bm<std::map<int, long>, 1024 * 100>);


BENCHMARK_MAIN();