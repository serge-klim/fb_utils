#pragma once
#include <iterator>
//#include <concept>


namespace utils { inline namespace v1 { namespace workarounds {
template <typename T, typename R>
//  requires std::constructible_from<T, decltype(std::ranges::begin(std::declval<R&&>())), decltype(std::ranges::end(std::declval<R&&>()))>
auto to(R&& from) -> decltype(T{std::ranges::begin(from), std::ranges::end(from)})
{
   return {std::ranges::begin(from), std::ranges::end(from)};
}

}}}

