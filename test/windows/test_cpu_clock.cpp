#include "windows.h"
#include "utils/cpu_info.hpp"
#include "utils/cpu_clock.hpp"
#include <boost/test/unit_test.hpp>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(utils_test_cpu_cycles_clock_suite)


BOOST_AUTO_TEST_CASE(estimate_win_test)
{
   auto estimated = utils::estimate_cpu_frequency();
   HKEY hkey;
   BOOST_REQUIRE_EQUAL(RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hkey), ERROR_SUCCESS);
   auto key = std::unique_ptr<std::remove_pointer_t<HKEY>, decltype(&RegCloseKey)>{hkey, &RegCloseKey};
   DWORD type;
   DWORD value;
   DWORD size = sizeof(value);
   RegQueryValueExW(key.get(), L"~MHz", nullptr, &type, reinterpret_cast<LPBYTE>(&value), &size);
   BOOST_REQUIRE_EQUAL(type, REG_DWORD);
   BOOST_REQUIRE_EQUAL(type, sizeof(value));
   BOOST_CHECK(estimated / 1000000, value);
 
}

BOOST_AUTO_TEST_SUITE_END()
