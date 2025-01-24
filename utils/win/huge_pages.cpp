#include "../huge_pages.hpp"
#include "Windows.h"
#include <boost/winapi/get_last_error.hpp>
#include <boost/system/system_error.hpp>
#include <boost/winapi/basic_types.hpp>
#include <cstddef>
#include <cassert>

namespace {

bool enable_hugepage_privilege(HANDLE process = GetCurrentProcess()) noexcept {
   HANDLE token;
   auto res = OpenProcessToken(process, TOKEN_ADJUST_PRIVILEGES, &token) != FALSE;
   if (res) {
      auto tp = TOKEN_PRIVILEGES{};
      tp.PrivilegeCount = 1;
      static_assert(sizeof(tp.Privileges) >= sizeof(tp.Privileges[0]), "dynamic allocation required");
      tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
      res = LookupPrivilegeValueW(nullptr, L"SeLockMemoryPrivilege", &tp.Privileges[0].Luid) && AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), nullptr, nullptr) != FALSE;
      if (res) {
         res = boost::winapi::GetLastError() == 0;
         // auto ps = PRIVILEGE_SET{0};
         // ps.PrivilegeCount = 1;
         // static_assert(sizeof(ps.Privilege) >= sizeof(ps.Privilege[0]), "dynamic allocation required");
         // ps.Control = PRIVILEGE_SET_ALL_NECESSARY;
         // std::memcpy(&ps.Privilege[0].Luid, &tp.Privileges[0].Luid, sizeof(ps.Privilege[0].Luid));
         // boost::winapi::BOOL_ enabled;
         // res = PrivilegeCheck(token, &ps, &enabled) != FALSE && enabled;
      }
      CloseHandle(token);
   }
   return res;
}

} // namespace

std::size_t utils::v1::huge_page_size() noexcept {
   static auto res = enable_hugepage_privilege() ? GetLargePageMinimum() : std::size_t{0};
   return res;
}

utils::v1::huge_region utils::v1::alloc_huge_pages(std::size_t size, std::optional<unsigned long> numa_node /*= {}*/) noexcept {
   static const auto huge_page_sz = huge_page_size();
   assert(huge_page_sz == 0 || size % huge_page_sz == 0);
   void* mem = nullptr;
   if (huge_page_sz != 0) {
      auto process = GetCurrentProcess();
      mem = !numa_node
                ? VirtualAllocEx(process, nullptr, size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE)
                : VirtualAllocExNuma(process, nullptr, size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE, *numa_node);
   }
   return {mem, detail::free_memory{}};
}

[[nodiscard]] utils::v1::huge_region utils::v1::alloc_huge_region(allocation_parameters& params) noexcept {
   auto flags = boost::winapi::DWORD_{MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES};
   auto allocation_type = allocation_parameters::allocation_type::huge_pages;
   static const auto huge_page_sz = huge_page_size();
   if ((params.page_size = huge_page_sz) == 0) {
      static_assert(allocation_parameters::allocation_type::strict == allocation_parameters::allocation_type::huge_pages);
      if (params.type == allocation_parameters::allocation_type::strict)
         return {};
      static_assert(allocation_parameters::allocation_type::relaxed == allocation_parameters::allocation_type::memory);
      allocation_type = allocation_parameters::allocation_type::memory;
      flags &= ~boost::winapi::DWORD_{MEM_LARGE_PAGES};
      params.page_size = page_size();
   }

   auto process = GetCurrentProcess();
   auto size = utils::size_in_blocks(params.size, params.page_size) * params.page_size;
   auto mem = !params.numa_node
                  ? VirtualAllocEx(process, nullptr, size, flags, PAGE_READWRITE)
                  : VirtualAllocExNuma(process, nullptr, size, flags, PAGE_READWRITE, *params.numa_node);
   if (mem == nullptr && allocation_type != allocation_parameters::allocation_type::memory && params.type != allocation_parameters::allocation_type::strict) {
      allocation_type = allocation_parameters::allocation_type::memory;
      flags &= ~boost::winapi::DWORD_{MEM_LARGE_PAGES};
      params.page_size = page_size();
      size = utils::size_in_blocks(params.size, params.page_size) * params.page_size;
      mem = !params.numa_node
                ? VirtualAllocEx(process, nullptr, size, flags, PAGE_READWRITE)
                : VirtualAllocExNuma(process, nullptr, size, flags, PAGE_READWRITE, *params.numa_node);
   }
   if (mem != nullptr) {
      params.size = size;
      params.type = allocation_type;
   }
   return {mem, detail::free_memory{}};
}

utils::v1::huge_region utils::v1::alloc_huge_region(std::size_t size, std::optional<unsigned long> numa_node /*= {}*/) noexcept {
   auto params = allocation_parameters{std::move(numa_node), size, allocation_parameters::allocation_type::relaxed, 0};
   return alloc_huge_region(params);
}
