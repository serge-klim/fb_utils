#define BOOST_TEST_MODULE utils_tests
#include "utils/thread.hpp"
#include "utils/sysinfo.hpp"
#include "utils/huge_pages.hpp"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(utils_test_suite)

//BOOST_AUTO_TEST_CASE(get_set_thread_cpu_set_test)
//{
//   auto cpu_set_before = utils::v1x::get_thread_cpu_set();
//   auto cpu = decltype(cpu_set_before)::value_type{256};
//   if (!cpu_set_before.empty())
//      cpu = cpu_set_before.back();
//   // BOOST_CHECK(!cpu_set_before.empty());
//   BOOST_CHECK(!utils::v1x::set_thread_cpu_set(&cpu, 1));
//   auto cpu_set_after = utils::v1x::get_thread_cpu_set();
//   BOOST_REQUIRE_EQUAL(cpu_set_after.size(), 1);
//   BOOST_CHECK_EQUAL(cpu_set_after.front(), cpu);
//}

BOOST_AUTO_TEST_CASE(get_thread_afinity_test)
{
   BOOST_CHECK_GT(utils::cache_line_size(), 16);
}


BOOST_AUTO_TEST_CASE(page_size_test)
{
   BOOST_CHECK_NE(utils::page_size(), 0);
   auto huge_page_size = utils::huge_page_size();
   BOOST_WARN_MESSAGE(huge_page_size == 0, "No huge/large pages");
   if (huge_page_size != 0) {
      auto mem_block = utils::alloc_huge_region(huge_page_size * 2);
      BOOST_CHECK_EQUAL(!mem_block, false);
   }
}

BOOST_AUTO_TEST_CASE(cache_line_size_test)
{
   BOOST_CHECK_GT(utils::cache_line_size(), 16);
}

//BOOST_AUTO_TEST_CASE(numa_node_test)
//{
//   auto workset = utils::numa_node_workset(0);
//   BOOST_CHECK(!workset.empty());
//}

BOOST_AUTO_TEST_SUITE_END()
