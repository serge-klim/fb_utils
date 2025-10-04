#include "test.hpp"
#include "utils/simd/vector.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>
#include <array>
#include <ranges>
#include <type_traits>


BOOST_AUTO_TEST_SUITE(utils_simd_test_suite)

#if(!defined(__GNUC__) || defined(__BMI__))
// gcc requires  -mbmi https://gcc.gnu.org/bugzilla/show_bug.cgi?id=110921

using test_types = boost::mp11::mp_list<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t> ;
using bit_sizes = boost::mp11::mp_list<
#if (defined(__SSE__) || defined(__AVX__))
                                       std::integral_constant<std::size_t, 128>,
#endif // __SSE__                                       
#ifdef __AVX2__
                                       std::integral_constant<std::size_t, 256>,
#endif // __AVX2__
#ifdef __AVX512BW_                                       
                                       std::integral_constant<std::size_t, 512>,
#endif // __AVX512BW__
                                       std::integral_constant<std::size_t, 0>
                                       >;

using test_pairs = boost::mp11::mp_product<boost::mp11::mp_list, test_types, bit_sizes>;

BOOST_AUTO_TEST_CASE_TEMPLATE(smid_128_find_test, Pair, test_pairs) {
    using type = boost::mp11::mp_first<Pair>;
    constexpr auto bit_size = boost::mp11::mp_second<Pair>::value;
    if constexpr (bit_size != 0) {
      constexpr auto vector_bytes_size = bit_size / 8;
      auto values = std::array<type, vector_bytes_size * sizeof(std::uint8_t) / sizeof(type) - 1>{};
      std::ranges::copy(std::views::transform(std::views::iota(0) | std::views::take(values.size()), [](auto v) {
                           return v * 3;
                        }),
                        begin(values));
      BOOST_CHECK_LE(utils::simd::detail::vector_size<type>(values.size()), bit_size);
      auto vector = utils::simd::vector{values};
      BOOST_CHECK_EQUAL(vector.size(), values.size());
      BOOST_CHECK_EQUAL(vector.capacity(), values.size() + 1);
      for (auto i : std::views::iota(0) | std::views::take(values.size()))
         BOOST_CHECK_EQUAL(vector.find_first(values[i]), i);

      BOOST_CHECK_GT(vector.find_first(values.back() + 33), values.size());
   }
}

#endif // !defined(__GNUC__) || defined(__BMI__)_

BOOST_AUTO_TEST_SUITE_END()
