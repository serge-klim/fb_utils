#pragma once
#include <utility>
#include <vector>
#include <cstdint>

namespace utils { inline namespace v1 {

[[nodiscard]] std::size_t page_size() noexcept;
[[nodiscard]] std::size_t cache_line_size() noexcept;
[[nodiscard]] std::size_t huge_page_size() noexcept;

struct group_afinity {
   unsigned short group;
   std::uint64_t mask;
};

[[nodiscard]] std::vector<group_afinity> numa_node_workset(unsigned short node);

struct core_info {
   unsigned short efficency_class;
   std::vector<group_afinity> groups;
};

[[nodiscard]] std::vector<core_info> cores_info();

struct cpu_info {
   unsigned short id;              // DWORD Id;
   unsigned short group;           // WORD Group;
   unsigned char cpu_ix;           // BYTE LogicalProcessorIndex;
   unsigned char core_ix;          // BYTE CoreIndex;
                                   // unsigned char core_ix;     // BYTE LastLevelCacheIndex;
   unsigned char numa_node;        // BYTE NumaNodeIndex;
   unsigned char efficiency_class; // BYTE EfficiencyClass;
   struct
   {
      unsigned char parked : 1;
      unsigned char allocated : 1;
      unsigned char allocated_to_target_process : 1;
      unsigned char real_time : 1;
   } flags;
   // union
   //{
   //    DWORD Reserved;
   //    BYTE SchedulingClass;
   // };

   // DWORD64 AllocationTag;
};

std::vector<cpu_info> cpus_info();

// struct core_info_
//{
//    unsigned char core_ix;   // BYTE CoreIndex;
//    unsigned char numa_node; // BYTE NumaNodeIndex;
//    unsigned short group;    // WORD Group;
//    unsigned long long mask;
//    unsigned char efficiency_class; // BYTE EfficiencyClass;
// };

}} // namespace utils::v1
