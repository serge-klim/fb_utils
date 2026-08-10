#include "../sysinfo.hpp"
#include <unistd.h>
// #include <hugetlbfs.h>
// #include <cassert>

std::vector<utils::group_afinity> utils::v1::numa_node_workset(unsigned short /*node*/) {
   return {};
}

std::size_t utils::cache_line_size() noexcept {
   return static_cast<std::size_t>(sysconf(_SC_LEVEL1_DCACHE_LINESIZE));
}
