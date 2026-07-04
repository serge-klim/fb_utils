#pragma once
#include <boost/align/aligned_allocator.hpp>
#include <vector>
#include <cstddef>

namespace utils { inline namespace v1 {

template <typename T>
using aligned_buffer = std::vector<char, boost::alignment::aligned_allocator<char, alignof(T)>>;

template <typename T>
aligned_buffer<T> make_bytesize_vector(std::size_t size_in_bytes) {
   return aligned_buffer<T>(size_in_bytes);
}

}} // namespace utils::v1

