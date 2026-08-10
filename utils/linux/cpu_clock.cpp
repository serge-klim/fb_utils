#include "../cpu_clock.hpp"
#include "../detail/cpu_clock.hpp"
#include "../math.hpp"
#include <fstream>
#include <iterator>
#include <algorithm>
#include <functional>
#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

namespace utils { inline namespace v1 { namespace detail {

std::int32_t read_dmsr(int msr, std::uint64_t& val) noexcept {
   auto fd = open("/dev/cpu/0/msr", O_RDONLY);
   if (fd < 0)
      return fd;

   auto res = pread(fd, &val, sizeof(uint64_t), msr);

   close(fd);

   return res;
}

bool is_tsc_known_freq() {
   auto in = std::ifstream{"/proc/cpuinfo"};
   if (!in)
      return false;

   auto const tsc_known_freq = std::string_view{"tsc_known_freq"};
   auto end = std::istream_iterator<char>{};
   auto i = std::search(std::istream_iterator<char>(in),
                         end,
                         std::default_searcher{cbegin(tsc_known_freq), cend(tsc_known_freq)});
   return i != end;
}

}}} // namespace utils::v1::detail            



std::uint64_t utils::v1::estimate_cpu_frequency() {
#ifdef CLOCK_MONOTONIC_RAW /* Defined in glibc bits/time.h */
   struct timespec t_start, t_end;
   if (clock_gettime(CLOCK_MONOTONIC_RAW, &t_start) == 0) {       
      auto start = utils::cpu_clock::now() /*rte_rdtsc()*/;
      struct timespec sleeptime = {.tv_nsec = static_cast<long int>(NS_PER_SEC / 10)}; /* 1/10 second */
      auto res1 = nanosleep(&sleeptime, nullptr);
      auto res2 = clock_gettime(CLOCK_MONOTONIC_RAW, &t_end);
      auto end = utils::cpu_clock::now() /*rte_rdtsc()*/;
      if( (res1|res2) !=0 )
          return 0;
      
      auto ns = ((t_end.tv_sec - t_start.tv_sec) * NS_PER_SEC);
      ns += (t_end.tv_nsec - t_start.tv_nsec);

      auto secs = static_cast<double>(ns) / NS_PER_SEC;
      auto tsc_hz = (uint64_t)((end - start) / secs);

      //if (arch_hz) {
      //  /* Make sure we're within 1% for sanity check */
      //    if (std::max(arch_hz, tsc_hz) - std::min(arch_hz, tsc_hz) > arch_hz / 100)
      //     return arch_hz;
      //}

      /* Round up to 100Khz. 1E5 ~ 100Khz */
      return align_mul_near(tsc_hz, CYC_PER_100KHZ);
   }
#endif
   return 0;
}

// double GetCPUCyclesPerSecond(CPUInfo::Scaling scaling) {
//    long freq = 0;

//    // If the kernel is exporting the tsc frequency use that. There are issues
//    // where cpuinfo_max_freq cannot be relied on because the BIOS may be
//    // exporintg an invalid p-state (on x86) or p-states may be used to put the
//    // processor in a new mode (turbo mode). Essentially, those frequencies
//    // cannot always be relied upon. The same reasons apply to /proc/cpuinfo as
//    // well.
//    if (ReadFromFile("/sys/devices/system/cpu/cpu0/tsc_freq_khz", &freq)
//        // If CPU scaling is disabled, use the *current* frequency.
//        // Note that we specifically don't want to read cpuinfo_cur_freq,
//        // because it is only readable by root.
//        || (scaling == CPUInfo::Scaling::DISABLED &&
//            ReadFromFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq",
//                         &freq))
//        // Otherwise, if CPU scaling may be in effect, we want to use
//        // the *maximum* frequency, not whatever CPU speed some random processor
//        // happens to be using now.
//        || ReadFromFile("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq",
//                        &freq)) {
//       // The value is in kHz (as the file name suggests).  For example, on a
//       // 2GHz warpstation, the file contains the value "2000000".
//       return static_cast<double>(freq) * 1000.0;
//    }

//    const double error_value = -1;
//    double bogo_clock = error_value;

//    std::ifstream f("/proc/cpuinfo");
//    if (!f.is_open()) {
//       std::cerr << "failed to open /proc/cpuinfo\n";
//       return error_value;
//    }
//    auto StartsWithKey = [](std::string const& Value, std::string const& Key) {
//       if (Key.size() > Value.size()) {
//          return false;
//       }
//       auto Cmp = [&](char X, char Y) {
//          return std::tolower(X) == std::tolower(Y);
//       };
//       return std::equal(Key.begin(), Key.end(), Value.begin(), Cmp);
//    };

//    std::string ln;
//    while (std::getline(f, ln)) {
//       if (ln.empty()) {
//          continue;
//       }
//       std::size_t split_idx = ln.find(':');
//       std::string value;
//       if (split_idx != std::string::npos) {
//          value = ln.substr(split_idx + 1);
//       }
//       // When parsing the "cpu MHz" and "bogomips" (fallback) entries, we only
//       // accept positive values. Some environments (virtual machines) report zero,
//       // which would cause infinite looping in WallTime_Init.
//       if (StartsWithKey(ln, "cpu MHz")) {
//          if (!value.empty()) {
//             double cycles_per_second = benchmark::stod(value) * 1000000.0;
//             if (cycles_per_second > 0) {
//                return cycles_per_second;
//             }
//          }
//       } else if (StartsWithKey(ln, "bogomips")) {
//          if (!value.empty()) {
//             bogo_clock = benchmark::stod(value) * 1000000.0;
//             if (bogo_clock < 0.0) {
//                bogo_clock = error_value;
//             }
//          }
//       }
//    }
//    if (f.bad()) {
//       std::cerr << "Failure reading /proc/cpuinfo\n";
//       return error_value;
//    }
//    if (!f.eof()) {
//       std::cerr << "Failed to read to end of /proc/cpuinfo\n";
//       return error_value;
//    }
//    f.close();
//    // If we found the bogomips clock, but nothing better, we'll use it (but
//    // we're not happy about it); otherwise, fallback to the rough estimation
//    // below.
//    if (bogo_clock >= 0.0) {
//       return bogo_clock;
//    }
// }

// ////////////////
// //#elif defined BENCHMARK_OS_WINDOWS_WIN32
// //// In NT, read MHz from the registry. If we fail to do so or we're in win9x
// //// then make a crude estimate.
// //DWORD data, data_size = sizeof(data);
// //if (IsWindowsXPOrGreater() &&
// //    SUCCEEDED(
// //        SHGetValueA(HKEY_LOCAL_MACHINE,
// //                    "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
// //                    "~MHz", nullptr, &data, &data_size))) {
// //   return static_cast<double>(static_cast<int64_t>(data) *
// //                              static_cast<int64_t>(1000 * 1000)); // was mhz
// //}
// //#elif defined(BENCHMARK_OS_SOLARIS)
