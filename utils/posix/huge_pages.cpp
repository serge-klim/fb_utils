#include "../huge_pages.hpp"
// #include <hugetlbfs.h>
// #include <cassert>

std::size_t utils::v1::huge_page_size() noexcept {
   return 0 /*gethugepagesize()*/;
}

// utils::v1::huge_region utils::v1::alloc_huge_pages(std::size_t size /*, unsigned long flags*/ /* = GHR_DEFAULT*/) noexcept
utils::v1::huge_region utils::v1::alloc_huge_pages(std::size_t /*size*/, std::optional<unsigned long> /*numa_node*/ /*= {}*/) noexcept {
   return {nullptr, utils::detail::free_memory{}}; /*get_hugepage_region(size, flags)*/
                                                   /*free_hugepage_region*/
}

// utils::v1::huge_region utils::v1::alloc_huge_region(std::size_t size /*, unsigned long flags*/ /* = GHR_DEFAULT*/) noexcept
utils::v1::huge_region utils::v1::alloc_huge_region(std::size_t /*size*/, std::optional<unsigned long> /*numa_node*/ /*= {}*/ /*, AllocatonFlags flags*/) noexcept {
   return {nullptr, utils::detail::free_memory{}}; /*get_hugepage_region(size, flags)*/
   ;                                               /*free_hugepage_region*/
}

utils::v1::huge_region utils::v1::alloc_huge_region(allocation_parameters& /*params*/) noexcept {
   return {nullptr, utils::detail::free_memory{}}; /*get_hugepage_region(size, flags)*/
   ;                                               /*free_hugepage_region*/
}
