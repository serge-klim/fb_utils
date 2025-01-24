#pragma once
#include <boost/test/unit_test.hpp>
#include <chrono>
#include <utility>
#include <ostream>
#include <string>
#include <vector>

namespace test{


} // namespace test

namespace std {
std::ostream& operator<<(std::ostream& out, std::vector<std::string> const values);

template <typename First, typename Second>
std::ostream& operator<<(std::ostream& out, std::pair<First, Second> const name_value)
{
   out << name_value.first << " - "
       << "{...}" /*nameValue.second*/;
   return out;
}

template<typename First, typename Rep, typename Period>
std::ostream& operator << (std::ostream& out, std::pair<First, std::chrono::duration<Rep, Period>> const name_value)
{
    out << name_value.first << " - " << std::chrono::duration_cast<std::chrono::nanoseconds>(name_value.second).count();
    return out;
}

} // namespace std