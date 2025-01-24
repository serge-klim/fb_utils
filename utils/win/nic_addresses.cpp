#include "nic_addresses.hpp"
#include "../nic_addresses.hpp"

template <typename T>
auto make_buffer(std::size_t size_in_bytes) { return std::vector<T>((size_in_bytes - 1 + sizeof(T)) / sizeof(T)); }

std::vector<IP_ADAPTER_ADDRESSES> utils::net::get_adapters_addresses(unsigned long family, unsigned long flags, std::size_t retries /*= 3*/) {
   constexpr auto min_size = static_cast<unsigned long>(1500); // 15KB  as recommended in remark https://learn.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersaddresses
   for (auto size = min_size;;) {
      auto addresses = make_buffer<IP_ADAPTER_ADDRESSES>(size);
      switch (auto result = GetAdaptersAddresses(family, flags, nullptr, addresses.data(), &size)) {
         case ERROR_SUCCESS:
            return addresses;
         case ERROR_BUFFER_OVERFLOW:
            if (retries-- > 1)
               break;
         default:
            throw boost::system::system_error(boost::winapi::GetLastError(), boost::system::system_category(), "GetAdaptersAddresses filed");
      }
   }
}

template <typename AddressFamily>
std::vector<AddressFamily> utils::net::get_nic_addresses(std::string const& ifname) {
   return utils::net::get_nic_addresses<AddressFamily>(ifname, utils::net::address_type::Unicast);
}

template std::vector<in_addr> utils::net::get_nic_addresses<in_addr>(std::string const&);
template std::vector<in6_addr> utils::net::get_nic_addresses<in6_addr>(std::string const&);

std::vector<utils::net::nic_info> utils::net::get_nic_info() {
   auto res = std::vector<utils::net::nic_info>{};
   constexpr auto flags = GAA_FLAG_INCLUDE_PREFIX |
                          /*GAA_FLAG_SKIP_UNICAST |*/ GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                          GAA_FLAG_SKIP_DNS_SERVER;

   static auto adapters = get_adapters_addresses(AF_UNSPEC, flags);
   for (auto const* adapter = adapters.data(); adapter != nullptr; adapter = adapter->Next) {
      auto& info = res.emplace_back(utils::net::nic_info{});
      auto name = std::wstring{adapter->FriendlyName};
      info.name = std::string{cbegin(name), cend(name)};
      info.mac.assign(adapter->PhysicalAddress, adapter->PhysicalAddress + adapter->PhysicalAddressLength);
      for (auto address = adapter->FirstUnicastAddress; address != nullptr; address = address->Next) {
         switch (address->Address.lpSockaddr->sa_family) {
            case AF_INET: {
               auto& addr = info.ipv4.emplace_back(in_addr{});
               // std::memcpy(&res.back(), address->Address.lpSockaddr, sizeof(AddressFamily));
               detail::copy(addr, address->Address.lpSockaddr);
               break;
            }
            case AF_INET6: {
               auto& addr = info.ipv6.emplace_back(in6_addr{});
               // std::memcpy(&res.back(), address->Address.lpSockaddr, sizeof(AddressFamily));
               detail::copy(addr, address->Address.lpSockaddr);
               break;
            }
         }
      }
   }
   return res;
}