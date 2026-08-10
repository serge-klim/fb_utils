#include "../memory.hpp"
#include <unistd.h>
// #include <hugetlbfs.h>
#include <cassert>

std::size_t utils::v1::page_size() noexcept {
   return static_cast<std::size_t>(sysconf(_SC_PAGESIZE));
}

void utils::v1::detail::free_memory::operator()(void* mem) const noexcept {
   assert(mem != nullptr);
   // free_hugepage_region(mem);
}

utils::v1::memory_block utils::v1::alloc_mem(size_t /*size*/ /*, int socket_id = 0*/) noexcept {
   return {nullptr, utils::detail::free_memory{}};
}
