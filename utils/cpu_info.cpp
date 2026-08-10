#include "cpu_info.hpp"
#include <cstdint>

#include <utility>
#include <span>
#include <algorithm>

#if defined(_M_AMD64) || defined(__i386__) || defined(__x86_64__)
#include "detail/x64/cpu_info.hpp"
#else
// #include "not_supported/vector.hpp"
#endif

std::string utils::v1::cpu_id_string() {
   auto cpuinfo = cpuid(0x80000000);
   if (cpuinfo[0] < 0x80000004)
      return {};
   constexpr auto chunk_size = std::tuple_size_v<cpuid_t> * sizeof(typename cpuid_t::value_type);
   auto buffer = std::string(chunk_size * 3, ' ');
   // x64 only so:
   cpuid(0x80000002, std::span<typename cpuid_t::value_type, std::tuple_size_v<cpuid_t>>{reinterpret_cast<cpuid_t::value_type*>(buffer.data()), std::tuple_size_v<cpuid_t>});
   cpuid(0x80000003, std::span<typename cpuid_t::value_type, std::tuple_size_v<cpuid_t>>{reinterpret_cast<cpuid_t::value_type*>(buffer.data()) + std::tuple_size_v<cpuid_t>, std::tuple_size_v<cpuid_t>});
   cpuid(0x80000004, std::span<typename cpuid_t::value_type, std::tuple_size_v<cpuid_t>>{reinterpret_cast<cpuid_t::value_type*>(buffer.data()) + std::tuple_size_v<cpuid_t> * 2, std::tuple_size_v<cpuid_t>});
   auto pos = std::string::npos;
   do {
      pos = buffer.find_last_not_of(' ', pos);
      if (pos == std::string::npos)
         return buffer;
   } while (buffer[pos] == '\0' && pos-- != 0);
   buffer.resize(pos + 1);
   return buffer;
}
