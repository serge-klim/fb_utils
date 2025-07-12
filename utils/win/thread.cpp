#include "../thread.hpp"
#include "experemental/sysinfo.hpp"
#include <boost/winapi/thread.hpp>
#include <boost/winapi/get_last_error.hpp>
#include <boost/system/system_error.hpp>
#include "windows.h"
#include "processthreadsapi.h"

int utils::getcpu_id() noexcept {
   return static_cast<int>(GetCurrentProcessorNumber());
}

std::thread::native_handle_type utils::this_thread() noexcept {
   return boost::winapi::GetCurrentThread();
}

boost::system::error_code utils::pin_thread_to_core(unsigned int core_id, std::thread::native_handle_type handle /*= this_thread()*/) noexcept {
   return SetThreadIdealProcessor(handle, core_id) == (DWORD)-1
              ? boost::system::error_code(boost::winapi::GetLastError(), boost::system::system_category())
              : boost::system::error_code{};
}

boost::system::error_code utils::set_thread_afinity(utils::cpu_set const& afinity_mask, std::thread::native_handle_type handle /*= this_thread()*/) noexcept {
   return SetThreadAffinityMask(handle, afinity_mask.to_ullong()) == 0
              ? boost::system::error_code(boost::winapi::GetLastError(), boost::system::system_category())
              : boost::system::error_code{};
}

boost::system::error_code utils::set_thread_cpu_set(unsigned long* cpu_ids, std::size_t size, std::thread::native_handle_type handle /*= this_thread()*/) noexcept {
   return !SetThreadSelectedCpuSets(handle, cpu_ids, static_cast<ULONG>(size))
              ? boost::system::error_code(boost::winapi::GetLastError(), boost::system::system_category())
              : boost::system::error_code{};
}

boost::system::error_code utils::set_thread_priority(int priority, std::thread::native_handle_type handle /*= this_thread()*/) noexcept {
   return SetThreadPriority(handle, priority) != FALSE 
       ? boost::system::error_code{} 
       : boost::system::error_code(boost::winapi::GetLastError(), boost::system::system_category());

}

///////////////v2

boost::system::error_code utils::v1x::set_thread_cpu_set(cpu_set::value_type const* cpu_ids, cpu_set::size_type size, std::thread::native_handle_type handle /*= this_thread()*/) noexcept {
   return !SetThreadSelectedCpuSets(handle, cpu_ids, static_cast<ULONG>(size))
              ? boost::system::error_code(boost::winapi::GetLastError(), boost::system::system_category())
              : boost::system::error_code{};
}

boost::system::error_code utils::v1x::get_thread_cpu_set(cpu_set::value_type* cpu_ids, cpu_set::size_type& size, std::thread::native_handle_type handle /*= this_thread()*/) noexcept {
   ULONG n = 0;
   auto res = !GetThreadSelectedCpuSets(handle, cpu_ids, static_cast<ULONG>(size), &n)
                  ? boost::system::error_code(boost::winapi::GetLastError(), boost::system::system_category())
                  : boost::system::error_code{};
   size = static_cast<cpu_set::size_type>(n);
   return res;
}

utils::v1x::cpu_set utils::v1x::get_thread_cpu_set(std::thread::native_handle_type handle /*= this_thread()*/) {

   cpu_set::size_type n = 0;
   auto error = get_thread_cpu_set(nullptr, n, handle);
   switch (error.value()) {
      case ERROR_SUCCESS:
         assert(n == 0);
         return {};
      case ERROR_INSUFFICIENT_BUFFER:
         break;
      default:
         throw boost::system::system_error(error, "GetThreadSelectedCpuSets filed");
   }

   auto res = cpu_set(n);
   assert(res.size() == n);
   error = get_thread_cpu_set(res.data(), n, handle);
   if (error)
      throw boost::system::system_error{error};
   return res;
}