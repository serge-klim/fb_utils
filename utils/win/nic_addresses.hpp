#pragma once
#include "winsock2.h"
#include "ws2ipdef.h"
#include "iphlpapi.h"

#include "../detail/nic_addresses.hpp"

#pragma comment(lib, "IPHLPAPI.lib")

#include <boost/winapi/get_last_error.hpp>
#include <boost/system/system_error.hpp>
#include <vector>
#include <tuple>
#include <cstring>
#include <cassert>

namespace utils { namespace net {

enum class address_type {
   Unicast,
   Anycast,
   Multicast
};

namespace detail {

inline void copy(in_addr& to, LPSOCKADDR from) {
   std::memcpy(&to, &reinterpret_cast<sockaddr_in const*>(from)->sin_addr, sizeof(in_addr));
}

inline void copy(in6_addr& to, LPSOCKADDR from) {
   std::memcpy(&to, &reinterpret_cast<sockaddr_in6 const*>(from)->sin6_addr, sizeof(in6_addr));
}

} // namespace detail

template <typename AddressType, typename AddressFamily>
std::vector<AddressFamily> find_address(IP_ADAPTER_ADDRESSES const* adapter_addresses) {
   auto res = std::vector<AddressFamily>{};
   static constexpr auto accessors = std::make_tuple(&IP_ADAPTER_ADDRESSES::FirstUnicastAddress, &IP_ADAPTER_ADDRESSES::FirstMulticastAddress, &IP_ADAPTER_ADDRESSES::FirstAnycastAddress);
   for (auto address = (adapter_addresses->*std::get<AddressType>(accessors)); address != nullptr; address = address->Next) {
      if (address->Address.lpSockaddr->sa_family == detail::address_family<AddressFamily>::value && address->Address.iSockaddrLength >= sizeof(AddressFamily)) {
         res.push_back(AddressFamily{});
         // std::memcpy(&res.back(), address->Address.lpSockaddr, sizeof(AddressFamily));
         detail::copy(res.back(), address->Address.lpSockaddr);
      }
   }
   return res;
}

std::vector<IP_ADAPTER_ADDRESSES> get_adapters_addresses(unsigned long family, unsigned long flags, std::size_t retries = 3);

template <typename AddressFamily>
std::vector<AddressFamily> get_nic_addresses(std::wstring const& name, address_type type, std::size_t retries = 3) {
   auto flags = GAA_FLAG_INCLUDE_PREFIX |
                /*GAA_FLAG_SKIP_UNICAST | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |*/
                GAA_FLAG_SKIP_DNS_SERVER;

   std::vector<AddressFamily> (*find)(IP_ADAPTER_ADDRESSES const*) = nullptr;
   switch (type) {
      case address_type::Unicast:
         flags |= GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;
         find = &find_address<decltype(&IP_ADAPTER_ADDRESSES::FirstUnicastAddress), AddressFamily>;
         break;
      case address_type::Anycast:
         flags |= GAA_FLAG_SKIP_UNICAST | GAA_FLAG_SKIP_MULTICAST;
         find = &find_address<decltype(&IP_ADAPTER_ADDRESSES::FirstAnycastAddress), AddressFamily>;
         break;
      case address_type::Multicast:
         flags |= GAA_FLAG_SKIP_UNICAST | GAA_FLAG_SKIP_ANYCAST;
         find = &find_address<decltype(&IP_ADAPTER_ADDRESSES::FirstMulticastAddress), AddressFamily>;
         break;
   }
   assert(find != nullptr);
   static auto adapters = get_adapters_addresses(detail::address_family<AddressFamily>::value, flags);
   for (auto const* adapter = adapters.data(); adapter != nullptr; adapter = adapter->Next) {
      if (name == adapter->FriendlyName)
         return (*find)(adapter);
   }
   return {};
}

template <typename AddressFamily>
std::vector<AddressFamily> get_nic_addresses(std::string const& name, address_type type, std::size_t retries = 3) {
   return get_nic_addresses<AddressFamily>(std::wstring{cbegin(name), cend(name)}, type, retries);
}

}

} // namespace utils::net