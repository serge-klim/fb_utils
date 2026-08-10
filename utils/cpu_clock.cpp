#include "cpu_clock.hpp"
#include "detail/x64/cpu_info.hpp"
#include <cstdint>
#include <array>


namespace {

constexpr bool x86_vendor_amd(std::uint32_t t1, std::uint32_t t2, std::uint32_t t3) noexcept {
   return ((t1 == 0x68747541) && /* htuA */
           (t2 == 0x444d4163) && /* DMAc */
           (t3 == 0x69746e65))   /* itne */
       ;
}

constexpr std::uint32_t check_model_wsm_nhm(int/*std::uint8_t*/ model) {
   switch (model) {
      /* Westmere */
      case 0x25:
      case 0x2C:
      case 0x2F:
      /* Nehalem */
      case 0x1E:
      case 0x1F:
      case 0x1A:
      case 0x2E:
         return 1;
   }

   return 0;
}

constexpr std::uint32_t check_model_gdm_dnv(int /*std::uint8_t*/ model) {
   switch (model) {
      /* Goldmont */
      case 0x5C:
      /* Denverton */
      case 0x5F:
         return 1;
   }
   return 0;
}

constexpr auto avx_bit() noexcept { return std::uint32_t(1) << 28; }

} //namespace


namespace utils { inline namespace v1 { namespace detail {

std::int32_t read_dmsr(int /*msr*/, std::uint64_t& /*val*/) noexcept;

}}} // namespace utils::v1::detail

std::uint64_t utils::v1::system_tsc_freq() noexcept {
    constexpr static auto CPUID_LEAF_MWAIT = 0x05;
    constexpr static auto CPUID_LEAF_DCA = 0x09;
    constexpr static auto CPUID_LEAF_XSTATE = 0x0d;
    constexpr static auto CPUID_LEAF_TSC = 0x15;
    constexpr static auto CPUID_LEAF_FREQ = 0x16;
    constexpr static auto CPUID_LEAF_TILE = 0x1d;

   auto cpu_info = cpuid();
   if (x86_vendor_amd(cpu_info[1], cpu_info[2], cpu_info[3]))
      return 0;
   auto maxleaf = max_leaf();
   if (maxleaf >= CPUID_LEAF_TSC) {
      auto cpu_info_tsc = cpuid(CPUID_LEAF_TSC);
      if (cpu_info_tsc[0] != 0 && cpu_info_tsc[1] != 0 && cpu_info_tsc[2] != 0)
         return static_cast<std::uint64_t>(cpu_info_tsc[2]) * cpu_info_tsc[1] / cpu_info_tsc[0];
   }
   auto cpu_info1 = cpuid(0x1);
   // auto model = cpu_model(cpu_info1[0]);
   auto cpu_family = (cpu_info1[0] >> 8) & 0xf;
   auto cpu_model = (cpu_info1[0] >> 4) & 0xf;
   if (cpu_family == 6 || cpu_family == 15) {
      auto ext_model = (cpu_info1[0] >> 16) & 0xf;
      cpu_model += (ext_model << 4);
   }

   auto mult = std::uint8_t{0};
   if (check_model_wsm_nhm(cpu_model))
      mult = 133;
   else if ((cpu_info1[2] & avx_bit()) || check_model_gdm_dnv(cpu_model))
      mult = 100;
   else
      return 0;

   std::uint64_t tsc_hz = 0;
   if (/*utils::*/detail::read_dmsr(0xCE, tsc_hz) < 0)
       return 0;
   return static_cast<std::uint64_t>(((tsc_hz >> 8) & 0xff) * mult * 1E6);
}

std::uint64_t utils::v1::tsc_frequency() {
   static auto const frequency = [] {
      auto res = system_tsc_freq();
      return res != 0 ? res : estimate_cpu_frequency();
    }();
   return frequency;
}

