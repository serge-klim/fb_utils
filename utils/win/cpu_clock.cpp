#include "../cpu_clock.hpp"
#include "../detail/cpu_clock.hpp"
#include "../math.hpp"
#include <cstdint>
#include <windows.h>
//#include <ntstatus.h>

namespace utils { inline namespace v1 { namespace detail {

std::int32_t read_dmsr(int /*msr*/, std::uint64_t& /*val*/) noexcept {
   return -1;
}

using NtDelayExecutionFn = /*NTSYSCALLAPI*/ LONG /*NTSTATUS*/ (NTAPI*)(_In_ BOOLEAN /*Alertable*/, _In_ PLARGE_INTEGER /*DelayInterval*/);
[[nodiscard]] NtDelayExecutionFn nt_delay_execution_fn() noexcept {
   static auto f = reinterpret_cast<NtDelayExecutionFn>(GetProcAddress(GetModuleHandleA("ntdll"), "NtDelayExecution"));
   return f;
}

bool sleep(long long delay_ns) {
   LARGE_INTEGER delay;
   delay.QuadPart = -delay_ns / 100;
   auto f = nt_delay_execution_fn();
   return f != nullptr && f(FALSE, &delay) == /*STATUS_SUCCESS*/ 0;
}

}}} // namespace utils::v1::detail

//rte_delay_us_sleep(unsigned int us) {
//   HANDLE timer;
//   LARGE_INTEGER due_time;
//
//   /* create waitable timer */
//   timer = CreateWaitableTimer(NULL, TRUE, NULL);
//   if (!timer) {
//      RTE_LOG_WIN32_ERR("CreateWaitableTimer()");
//      rte_errno = ENOMEM;
//      return;
//   }
//
//   /*
//    * due_time's uom is 100 ns, multiply by 10 to convert to microseconds
//    * set us microseconds time for timer
//    */
//   due_time.QuadPart = -((int64_t)us * 10);
//   if (!SetWaitableTimer(timer, &due_time, 0, NULL, NULL, FALSE)) {
//      RTE_LOG_WIN32_ERR("SetWaitableTimer()");
//      rte_errno = EINVAL;
//      goto end;
//   }
//   /* start wait for timer for us microseconds */
//   if (WaitForSingleObject(timer, INFINITE) == WAIT_FAILED) {
//      RTE_LOG_WIN32_ERR("WaitForSingleObject()");
//      rte_errno = EINVAL;
//   }
//
//end:
//   CloseHandle(timer);
//}

std::uint64_t utils::v1::estimate_cpu_frequency() {
   if (detail::nt_delay_execution_fn() == nullptr)
      return 0;
   LARGE_INTEGER frequency;  
   if (!QueryPerformanceFrequency(&frequency) || frequency.QuadPart == 0)
      return 0;
   LARGE_INTEGER t_start;
   auto r1 = QueryPerformanceCounter(&t_start);
   auto start = cpu_clock::raw_now /*rte_get_tsc_cycles*/ ();
   auto r2 = detail::sleep(NS_PER_SEC / 10); /* 1/10 second */
   LARGE_INTEGER t_end;
   auto r3 = QueryPerformanceCounter(&t_end);
   auto end = cpu_clock::raw_now /*rte_get_tsc_cycles*/ ();
   if (!r1 || !r2 || !r3)
      return 0;
   auto elapsed_us = t_end.QuadPart - t_start.QuadPart;
   /*
    * To guard against loss-of-precision, convert to microseconds
    * *before* dividing by ticks-per-second.
    */
   elapsed_us *= US_PER_SEC;
   elapsed_us /= frequency.QuadPart;

   auto secs = static_cast<double>(elapsed_us) / US_PER_SEC;
   auto tsc_hz = static_cast<std::uint64_t>((end - start) / secs);
   /* Round up to 100Khz. 1E5 ~ 100Khz */
   return align_mul_near(tsc_hz, CYC_PER_100KHZ);
}



