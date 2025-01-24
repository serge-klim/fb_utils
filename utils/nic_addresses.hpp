#pragma once
#include <vector>
#include <string>
#ifdef WIN32
#include "WinSock2.h"
#include "ws2ipdef.h"
#else
#include <netinet/in.h>
#endif

namespace utils { namespace net {

template <typename AddressFamily>
std::vector<AddressFamily> get_nic_addresses(std::string const& name);

struct nic_info
{
   std::string name;
   std::vector<unsigned char> mac;
   std::vector<in_addr> ipv4;
   std::vector<in6_addr> ipv6;
};

std::vector<nic_info> get_nic_info();

}

} // namespace utils::net
