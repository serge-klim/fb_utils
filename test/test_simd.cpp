#include "test.hpp"
#include "utils/simd/vector.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/mpl/list.hpp>
#include <array>
#include <ranges>


BOOST_AUTO_TEST_SUITE(utils_simd_test_suite)

using test_types = boost::mpl::list<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t> ;

BOOST_AUTO_TEST_CASE_TEMPLATE(smid_256_find_test, T, test_types) {
   constexpr auto vector_bytes_size = 256 / 8;
   auto values = std::array<T, vector_bytes_size * sizeof(std::uint8_t) / sizeof(T)  - 1>{};
   std::ranges::copy(std::views::transform(std::views::iota(0) | std::views::take(values.size()), [](auto v) {
                        return v * 3;
       }), begin(values));
   auto vector = utils::simd::vector{values};
   BOOST_CHECK_EQUAL(vector.size(), values.size());
   BOOST_CHECK_EQUAL(vector.capacity(), values.size() + 1);
   for (auto i : std::views::iota(0) | std::views::take(values.size()))
      BOOST_CHECK_EQUAL(vector.find_first(values[i]), i);

   BOOST_CHECK_GT(vector.find_first(values.back() + 33), values.size());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(smid_512_find_test, T, test_types) {
   constexpr auto vector_bytes_size = 512 / 8;
   auto values = std::array<T, vector_bytes_size * sizeof(std::uint8_t) / sizeof(T) - 1>{};
   std::ranges::copy(std::views::transform(std::views::iota(0) | std::views::take(values.size()), [](auto v) {
                        return v * 3;
                     }),
                     begin(values));
   auto vector = utils::simd::vector{values};
   BOOST_CHECK_EQUAL(vector.size(), values.size());
   BOOST_CHECK_EQUAL(vector.capacity(), values.size() + 1);
   for (auto i : std::views::iota(0) | std::views::take(values.size()))
      BOOST_CHECK_EQUAL(vector.find_first(values[i]), i);

   BOOST_CHECK_GT(vector.find_first(values.back() + 33), values.size());
}

BOOST_AUTO_TEST_SUITE_END()
