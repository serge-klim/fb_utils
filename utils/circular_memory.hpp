#pragma once
#include <atomic>
#include <iterator>
#include <utility>
#include <new>

namespace utils { inline namespace v1 {


namespace detail {

template <typename T>
struct free_memory_counter
{
   using size_type = T;
   T counter;
};

template <typename T>
struct free_memory_counter<std::atomic<T>>
{
   using size_type = T;
   alignas(std::hardware_destructive_interference_size) std::atomic<T> counter;
};

} // namespace detail

template <typename Counter = std::atomic<std::size_t>>
class circular_memory
{
 public:
   using size_type = typename detail::free_memory_counter<Counter>::size_type;
   constexpr circular_memory(char* buffer, size_type size /*, std::uint_fast8_t alignment = 1*/) noexcept
       : begin_{buffer}, size_{size}, free_{size} /*, alignment_{alignment}*/ {}

   bool empty() const noexcept { return free_.counter == size_; }
   void* allocate(size_type size, std::size_t alignment = 1) noexcept;
   void release(void* mem) noexcept;

   // debug only
   size_type free() const noexcept { return free_.counter; }
   constexpr auto size() const noexcept { return size_; }
   size_type used_memory(void const* pointer) const noexcept;

 private:
   char* begin_;
   size_type tail_ = 0;
   size_type /*const*/ size_;
   detail::free_memory_counter<Counter> free_;
};

}} // namespace utils::v1

template <typename Counter>
void* utils::v1::circular_memory<Counter>::allocate(size_type size, std::size_t alignment /*= 1*/) noexcept {   
   assert(tail_ <= size_);
   auto const end = tail_ + free_.counter;
   auto const split_space = end > size_
       ? std::make_pair(size_ - tail_, end - size_)
       : std::make_pair(end - tail_, size_type{0});
   assert(split_space.first + split_space.second <= free_.counter);

   auto pad_n_align = [](std::size_t alignment, size_type size, char* pointer, std::size_t space) noexcept -> void* {
      if (sizeof(size) + size > space)
         return nullptr;
      space -= sizeof(size);
      std::advance(pointer, sizeof(size));
      auto ptr = static_cast<void*>(pointer);
      return std::align(alignment, size, ptr/*static_cast<void*>(pointer)*/, space);
   };

   auto pointer = std::next(begin_, tail_);
   auto res = pad_n_align(alignment, size, pointer, split_space.first);
   if (!res) {
      pointer = begin_;
      res = pad_n_align(alignment, size, pointer, split_space.second);
      if (!res)
         return nullptr;
      size += split_space.first;
   }
   size += static_cast<size_type>(std::distance(pointer, static_cast<char*>(res)));
   std::memcpy(static_cast<char*>(res) - sizeof(size), &size, sizeof(size));
   free_.counter -= size;
   tail_ = (tail_ + size)%size_;
   assert(tail_ <= size_);
   return res;
}

template <typename Counter>
void utils::v1::circular_memory<Counter>::release(void* mem) noexcept {
   assert(mem != nullptr);
   free_.counter += used_memory(mem);
}

template <typename Counter>
utils::v1::circular_memory<Counter>::size_type utils::v1::circular_memory<Counter>::used_memory(void const* pointer) const noexcept {
   size_type res;
   std::memcpy(&res, std::prev(static_cast<size_type const*>(pointer)), sizeof(res));
   return res;
}

