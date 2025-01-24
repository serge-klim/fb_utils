#pragma once
#include <memory>
#include <utility>

namespace utils { inline namespace v1 {

template <typename T, typename B>
inline constexpr auto size_in_blocks(T size, B block) noexcept { return (size + block - 1) / block; }

namespace detail {

struct free_memory
{
   void operator()(void* mem) const noexcept;
};

} // namespace detail

[[nodiscard]] std::size_t page_size() noexcept;
using memory_block = std::unique_ptr<void, utils::detail::free_memory>;
[[nodiscard]] memory_block alloc_mem(size_t size /*, int socket_id = 0*/) noexcept;


}} // namespace utils::v1

