#include "utils/cpu_info.hpp"
#include "utils/cpu_clock.hpp"
#include <boost/test/unit_test.hpp>
#include <thread>

BOOST_AUTO_TEST_SUITE(utils_test_cpu_cycles_clock_suite)

BOOST_AUTO_TEST_CASE(cpu_id_string_test) {
   auto id_string = utils::cpu_id_string();
   BOOST_CHECK(!id_string.empty());
}

BOOST_AUTO_TEST_CASE(estimate_test)
{
   auto clock = utils::cpu_clock{};
   auto freq = utils::system_tsc_freq();
   auto estimated = utils::estimate_cpu_frequency();
   //freq = get_tsc_freq(freq);
   //if (!freq)
   //   freq = estimate_tsc_freq();
   //EAL_LOG(DEBUG, "TSC frequency is ~%" PRIu64 " KHz", freq / 1000);
   //eal_tsc_resolution_hz = freq;


   auto first = clock.now();
   //std::atomic_thread_fence::
   std::this_thread::yield();
   auto second = clock.now();
   BOOST_CHECK_NE(first, second);
}

BOOST_AUTO_TEST_SUITE_END()
