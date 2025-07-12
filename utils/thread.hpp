#pragma once
#include <thread>
#include <bitset>
#include <boost/system/error_code.hpp>

namespace utils{

std::thread::native_handle_type this_thread() noexcept;
boost::system::error_code set_thread_priority(int priority, std::thread::native_handle_type handle = this_thread()) noexcept;
boost::system::error_code pin_thread_to_core(unsigned int core_id, std::thread::native_handle_type handle = this_thread()) noexcept;
boost::system::error_code get_thread_afinity(unsigned int core_id, std::thread::native_handle_type handle = this_thread()) noexcept;
[[nodiscard]] int getcpu_id() noexcept;

using cpu_set = std::bitset<64>;
boost::system::error_code set_thread_afinity(utils::cpu_set const& afinity_mask, std::thread::native_handle_type handle = this_thread()) noexcept;


boost::system::error_code set_thread_cpu_set(unsigned long* cpu_ids, std::size_t size, std::thread::native_handle_type handle /*= this_thread()*/) noexcept;

namespace v1x {
using cpu_set = std::vector<unsigned long>;
boost::system::error_code set_thread_cpu_set(cpu_set::value_type const* cpu_ids, cpu_set::size_type size, std::thread::native_handle_type handle = this_thread()) noexcept;
boost::system::error_code get_thread_cpu_set(cpu_set::value_type* cpu_ids, cpu_set::size_type& size, std::thread::native_handle_type handle = this_thread()) noexcept;
cpu_set get_thread_cpu_set(std::thread::native_handle_type handle = this_thread());
}


} // namespace utils
