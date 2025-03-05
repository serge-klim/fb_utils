#pragma once
#include "windows.h"
#include <boost/stl_interfaces/iterator_interface.hpp>
// #include <boost/stl_interfaces/view_interface.hpp>
#include <boost/system/system_error.hpp>
#include <iterator>
#include <vector>
#include <ranges>

namespace utils::experemental {
inline namespace v1 {

namespace detail {

struct sys_set_info_iterator : boost::stl_interfaces::iterator_interface<
#if !BOOST_STL_INTERFACES_USE_DEDUCED_THIS
                                   sys_set_info_iterator,
#endif
                                   std::forward_iterator_tag,
                                   SYSTEM_CPU_SET_INFORMATION> {
   constexpr sys_set_info_iterator(char const* ptr = nullptr) noexcept : ptr_(ptr) {}

   SYSTEM_CPU_SET_INFORMATION const& operator*() const noexcept { return *reinterpret_cast<SYSTEM_CPU_SET_INFORMATION const*>(ptr_); }
   sys_set_info_iterator& operator++() {
      ptr_ += this->operator*().Size;
      return *this;
   }

   friend constexpr bool operator==(
       sys_set_info_iterator const& lhs, sys_set_info_iterator const& rhs) noexcept {
      return lhs.ptr_ == rhs.ptr_;
   }

   using boost::stl_interfaces::iterator_interface<
#if !BOOST_STL_INTERFACES_USE_DEDUCED_THIS
       sys_set_info_iterator,
#endif
       std::forward_iterator_tag,
       SYSTEM_CPU_SET_INFORMATION>::operator++;

 private:
   char const* ptr_;
};

class cpu_set_info
//: public //std::ranges::view_interface<cpu_set_info>
//           boost::stl_interfaces::view_interface<cpu_set_info>
{
 public:
   cpu_set_info() : data_(sizeof(SYSTEM_CPU_SET_INFORMATION) * 32) {
      unsigned long size = 0;
      if (!GetSystemCpuSetInformation(reinterpret_cast<PSYSTEM_CPU_SET_INFORMATION>(data_.data()), static_cast<ULONG>(data_.size()), &size, 0, 0)) {
         auto error = boost::winapi::GetLastError();
         if (error != ERROR_INSUFFICIENT_BUFFER)
            throw boost::system::system_error(error, boost::system::system_category(), "GetSystemCpuSetInformation filed");
         data_.resize(size);
         if (!GetSystemCpuSetInformation(reinterpret_cast<PSYSTEM_CPU_SET_INFORMATION>(data_.data()), static_cast<ULONG>(data_.size()), &size, 0, 0))
            throw boost::system::system_error(boost::winapi::GetLastError(), boost::system::system_category(), "GetSystemCpuSetInformation filed");
      }
      data_.resize(size);
   }
   constexpr auto cbegin() const noexcept { return sys_set_info_iterator{data_.data()}; }
   constexpr auto cend() const noexcept { return sys_set_info_iterator{data_.data() + data_.size()}; }

 private:
   std::vector<char> data_;
};

constexpr auto begin(cpu_set_info const& info) noexcept { return info.cbegin(); }
constexpr auto end(cpu_set_info const& info) noexcept { return info.cend(); }
}

}} // namespace utils::experemental::v1::detail
#include <boost/stl_interfaces/view_interface.hpp>
#include <ranges>

namespace utils::experemental { inline namespace v1 {

auto cpu_set() {
   static const auto info = detail::cpu_set_info{};
   //   static auto tmp = std::vector<typename decltype(info.cbegin())::value_type>{info.cbegin(), info.cend()};
   //   static auto it = boost::iterator_range<typename decltype(info.cbegin())>{begin(info), end(info)};
   //   static_assert(std::input_iterator<decltype(info.cbegin())>, "Oops! invalid iterator");
   //   static_assert(std::sentinel_for<decltype(info.cend()), decltype(info.cbegin())>); // error
   //   //return info
   ////   return std::ranges::subrange{cbegin(tmp), cend(tmp)}
   return std::ranges::subrange{begin(info), end(info)} | std::views::filter([](auto const& cpu_info) { return cpu_info.Type == CPU_SET_INFORMATION_TYPE::CpuSetInformation; }) | std::views::transform([](auto const& cpu_info) -> decltype(cpu_info.CpuSet) const& { return cpu_info.CpuSet; });
}

}} // namespace utils::experemental::v1
