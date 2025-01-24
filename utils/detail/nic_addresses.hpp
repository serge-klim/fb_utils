#pragma once
#include <type_traits>

namespace utils { namespace net { namespace detail {

template <typename T> struct address_family;
template <> struct address_family<in_addr> : std::integral_constant<ADDRESS_FAMILY, AF_INET> {};
template <> struct address_family<in6_addr> : std::integral_constant<ADDRESS_FAMILY, AF_INET6>{};

} // namespace detail

} // namespace net

} // namespace utils