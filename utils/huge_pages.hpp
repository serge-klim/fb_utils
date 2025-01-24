#pragma once
#include "memory.hpp"
#include <optional>

namespace utils { inline namespace v1 {

// enum class AllocatonFlags
//{
//    Default,
//    Strict,
//    Fallback
// };

using huge_region = memory_block;
// huge pages
[[nodiscard]] std::size_t huge_page_size() noexcept;

struct allocation_parameters
{
   std::optional<unsigned long> numa_node;
   std::size_t size;
   enum class allocation_type
   {
      huge_pages,
      strict = huge_pages,
      relaxed,
      memory = relaxed
   } type;
   std::size_t page_size = 0;
};

[[nodiscard]] huge_region alloc_huge_pages(std::size_t size, std::optional<unsigned long> numa_node = {}) noexcept;
[[nodiscard]] huge_region alloc_huge_region(std::size_t size, std::optional<unsigned long> numa_node = {} /*, AllocatonFlags flags*/) noexcept;
[[nodiscard]] huge_region alloc_huge_region(allocation_parameters& params) noexcept;

}} // namespace utils::v1
