#include "../memory.hpp"
#include "Windows.h"
#include <boost/winapi/get_last_error.hpp>
#include <boost/system/system_error.hpp>
#include <vector>
#include <cstddef>
#include <cassert>

//namespace {
//
// std::size_t /*utils::*/ memory_info(void* mem)
//{
//    assert(!mem == false);
//    MEMORY_BASIC_INFORMATION info = {0};
//    if (VirtualQuery(mem, &info, sizeof(info)) == 0)
//      throw boost::system::system_error(boost::winapi::GetLastError(), boost::system::system_category(), "GetAdaptersAddresses filed");
//
//    return info.RegionSize;
// }
//
//} // namespace

std::size_t utils::v1::page_size() noexcept
{
   static auto size = []() {
      SYSTEM_INFO info{};
      GetSystemInfo(&info);
      return static_cast<std::size_t>(info.dwPageSize);
   }();
   return size;
}

void utils::v1::detail::free_memory::operator()(void* mem) const noexcept
{
   assert(mem != nullptr);
   VirtualFree(mem, 0, MEM_RELEASE);
}

utils::v1::memory_block utils::v1::alloc_mem(size_t size /*, int socket_id = 0*/) noexcept {
   assert(size % page_size() == 0);
   auto mem = VirtualAllocEx(GetCurrentProcess(), nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
   // auto mem =  VirtualAllocExNuma(GetCurrentProcess(), NULL, size, flags | huge_pages_flags, PAGE_READWRITE, /*eal_socket_numa_node(socket_id)*/ socket_id);
   return {mem, detail::free_memory{}};
}


