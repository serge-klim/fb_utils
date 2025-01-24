#include "../nic_addresses.hpp"
#include <linux/if_packet.h>
#include <bit>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>

using ADDRESS_FAMILY = int;
#include "../detail/nic_addresses.hpp"

// https://man7.org/linux/man-pages/man3/getifaddrs.3.html

namespace detail {

inline void copy(in_addr& to, sockaddr const* from) {
   std::memcpy(&to, &reinterpret_cast<sockaddr_in const*>(from)->sin_addr, sizeof(in_addr));
}

inline void copy(in6_addr& to, sockaddr const* from) {
   std::memcpy(&to, &reinterpret_cast<sockaddr_in6 const*>(from)->sin6_addr, sizeof(in6_addr));
}

} // namespace detail

template <typename AddressFamily>
std::vector<AddressFamily> utils::net::get_nic_addresses(std::string const& ifname) {
   auto res = std::vector<AddressFamily>{};
   struct ifaddrs* interface_array = nullptr;
   if (auto rc = getifaddrs(&interface_array) != -1) {
      for (auto addr = interface_array; addr != nullptr; addr = addr->ifa_next) {
         if (ifname == addr->ifa_name && addr->ifa_addr->sa_family == detail::address_family<AddressFamily>::value) {
            res.push_back(AddressFamily{});
            // std::memcpy(&res.back(), addr->ifa_addr, sizeof(AddressFamily));
            ::detail::copy(res.back(), addr->ifa_addr);
         }
      }
   }
   return res;
}

template std::vector<in_addr> utils::net::get_nic_addresses<in_addr>(std::string const&);
template std::vector<in6_addr> utils::net::get_nic_addresses<in6_addr>(std::string const&);

std::vector<utils::net::nic_info> utils::net::get_nic_info() {
   auto res = std::vector<utils::net::nic_info>{};
   struct ifaddrs* interface_array = nullptr;
   if (auto rc = getifaddrs(&interface_array) != -1) {
      for (auto addr = interface_array; addr != nullptr; addr = addr->ifa_next) {
         auto end = std::end(res);
         auto i = std::find_if(begin(res), end, [&addr](auto const& info) {
            return info.name == addr->ifa_name;
         });
         auto& info = i == end ? res.emplace_back(utils::net::nic_info{}) : *i;
         info.name = addr->ifa_name;
         switch (addr->ifa_addr->sa_family) {
            case AF_INET: {
               auto& address = info.ipv4.emplace_back(in_addr{});
               // std::memcpy(&res.back(), address->Address.lpSockaddr, sizeof(AddressFamily));
               ::detail::copy(address, addr->ifa_addr);
               break;
            }
            case AF_INET6: {
               auto& address = info.ipv6.emplace_back(in6_addr{});
               // std::memcpy(&res.back(), address->Address.lpSockaddr, sizeof(AddressFamily));
               ::detail::copy(address, addr->ifa_addr);
               break;
            }
            case AF_PACKET: {
               // constexpr auto ll = std::bit_cast<sockaddr_ll>(addr->ifa_addr);
               auto ll = sockaddr_ll{};
               std::memcpy(&ll, addr->ifa_addr, sizeof(ll));
               info.mac.assign(ll.sll_addr, ll.sll_addr + ll.sll_halen);
               break;
            }
         }
      }
   }
   return res;
}
