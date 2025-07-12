#include "../thread.hpp"
#include <pthread.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <boost/system/system_error.hpp>
#include <cassert>

int utils::getcpu_id() noexcept {
   return sched_getcpu();
}

std::thread::native_handle_type utils::this_thread() noexcept {
   return pthread_self();
}

boost::system::error_code utils::get_thread_afinity(unsigned int /*core_id*/, std::thread::native_handle_type handle /*= this_thread()*/) noexcept {
   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   auto error = pthread_getaffinity_np(handle, sizeof(cpuset), &cpuset);
   return error == 0 ? boost::system::error_code{} : boost::system::error_code{error, boost::system::system_category()};
 }

boost::system::error_code utils::pin_thread_to_core(unsigned int core_id, std::thread::native_handle_type handle /*= this_thread()*/) noexcept {
   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);
   auto error = pthread_setaffinity_np(handle, sizeof(cpuset), &cpuset) == 0;
   return error == 0 ? boost::system::error_code{} : boost::system::error_code{error, boost::system::system_category()};
}

// bool utils::set_thread_afinity(std::vector<int> const& cpus, std::thread::native_handle_type handle /*= this_thread()*/) noexcept
//{
//    cpu_set_t* cpu_set = CPU_ALLOC(cpus.size());
//    if (cpu_set!=0)
//       return false;
//    CPU_ZERO_S(cpus.size(), cpu_set);
//    for (auto const& cpu:cpus)
//        CPU_SET(cpu, cpu_set);
//    auto res = pthread_setaffinity_np(handle, cpus.size(), cpu_set) != 0;
//    CPU_FREE(cpu_set);
//    return res;
// }
//
////bool utils::set_thread_afinity(utils::cpu_set const& afinity_mask, std::thread::native_handle_type handle /*= this_thread()*/) noexcept
////{
////   auto size = afinity_mask.count();
////   cpu_set_t* cpu_set = CPU_ALLOC(size);
////   CPU_ZERO_S(size, cpu_set);
////   while (size--!=0)
////      if (afinity_mask.test(size))
////        CPU_SET(size, cpu_set);
////   auto res = pthread_setaffinity_np(handle, size, cpu_set) != 0 ;
////   CPU_FREE(cpu_set);
////   return res;
////}

boost::system::error_code utils::set_thread_priority(int priority, std::thread::native_handle_type handle /*= this_thread()*/) noexcept {
   // https://man7.org/linux/man-pages/man3/pthread_setschedparam.3.html
   //  auto params = sched_param{0};
   //  params.sched_priority = priority;
   //  return pthread_setschedparam(thread, priority) != 0;
   auto error = pthread_setschedprio(handle, priority) != 0;
   return error == 0 ? boost::system::error_code{} : boost::system::error_code{error, boost::system::system_category()};
}

boost::system::error_code utils::v1x::set_thread_cpu_set(cpu_set::value_type const* cpu_ids, cpu_set::size_type size, std::thread::native_handle_type handle /*= this_thread()*/) noexcept {
   auto nprocs = get_nprocs_conf();
   assert(nprocs != 0);
   auto /*cpu_set_t**/ cpu_set = CPU_ALLOC(nprocs);
   if (cpu_set == nullptr)
      return boost::system::error_code(errno != 0 ? errno : boost::system::errc::not_enough_memory, boost::system::system_category());
   struct sentry_t {
      ~sentry_t() {
         CPU_FREE(cpu_set);
      }
      cpu_set_t* cpu_set;
   } sentry{cpu_set};
   auto cpu_set_sz = CPU_ALLOC_SIZE(nprocs);
   CPU_ZERO_S(cpu_set_sz, cpu_set);
   while (size-- != 0) {
      CPU_SET_S(cpu_ids[size], nprocs, cpu_set);
   }
   // auto const n = size;
   // size = 0;
   // for (auto cpu = decltype(size){0}; cpu != n; ++cpu) {
   //    if (CPU_ISSET_S(cpu, cpu_set))
   //       cpu_ids[size++] = cpu;
   // }
   if (auto error = pthread_setaffinity_np(handle, cpu_set_sz, cpu_set))
      return boost::system::error_code(error, boost::system::system_category());
   return {};
}

utils::v1x::cpu_set utils::v1x::get_thread_cpu_set(std::thread::native_handle_type handle /*= this_thread()*/) {
   auto nprocs = get_nprocs_conf();
   assert(nprocs != 0);
   auto /*cpu_set_t**/ cpu_set = CPU_ALLOC(nprocs);
   if (cpu_set == nullptr)
      throw boost::system::system_error(errno != 0 ? errno : boost::system::errc::not_enough_memory, boost::system::system_category());

   struct sentry_t {
      ~sentry_t() {
         CPU_FREE(cpu_set);
      }
      cpu_set_t* cpu_set;
   } sentry{cpu_set};

   auto cpu_set_sz = CPU_ALLOC_SIZE(nprocs);
   CPU_ZERO_S(cpu_set_sz, cpu_set);
   if (auto error = pthread_getaffinity_np(handle, cpu_set_sz, cpu_set))
      throw boost::system::system_error(error, boost::system::system_category());

   auto res = utils::v1x::cpu_set{};
   res.reserve(nprocs);
   for (auto cpu = decltype(nprocs){0}; cpu != nprocs; ++cpu) {
      if (CPU_ISSET_S(cpu, nprocs, cpu_set))
         res.push_back(cpu);
   }
   return res;
}

//    auto nprocs = get_nprocs_conf();
//    assert(nprocs != 0);
//    auto /*cpu_set_t**/ cpu_set = CPU_ALLOC(nprocs);
//    if (cpu_set == nullptr)
//       return boost::system::error_code(errno != 0 ? errno : boost::system::errc::not_enough_memory, boost::system::system_category());
//
//    auto nprocs = get_nprocs_config();
//    auto res = utils::v1x::cpu_set(nproc);
//    get_thread_cpu_set(res.data(), );
//
//    assert(nocs != 0);
//    cpu_set_t* cpu_set = CPU_ALLOC(nprocs);
//    if (cpu_set == nullptr)
//       throw boost::system::error_code(errno != 0 ? errno, boost::system::errc::not_enough_memory, boost::system::system_category());
//
//    CPU_ZERO_S(nprocs, cpu_set);
//    pthread_getaffinity_np(handle, nprocs, cpu_set)
//
//    auto res = utils::v1x::cpu_set{};
//    res.reserve(nproc);
//
//    for (auto cpu = decltype(nprocs){0}; cpu != nprocs; ++cpu)
//       if (CPU_ISSET_S(cpu, cpu_set))
//          res.push_back(res);
//    auto res = pthread_getaffinity_np(handle, nprocs, cpu_set) != 0;
//    CPU_FREE(cpu_set);
//    return res;
//
//   ULONG n;
//   if (GetThreadSelectedCpuSets(handle, nullptr, 0, &n)) {
//      assert(n == 0);
//      return {};
//   }
//
//   if (auto error = boost::winapi::GetLastError(); error != ERROR_INSUFFICIENT_BUFFER)
//      throw boost::system::system_error(error, boost::system::system_category(), "GetSystemCpuSetInformation filed");
//   auto res = cpu_set(n);
//   auto error = get_thread_cpu_set(res.data(), res.size(), handle);
//   if (error)
//      throw boost::system::system_error{error};
//   return res;
//}
