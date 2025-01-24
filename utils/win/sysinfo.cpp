#include "../memory.hpp"
#include "../sysinfo.hpp"
#include "Windows.h"
#include <boost/winapi/get_last_error.hpp>
#include <boost/system/system_error.hpp>
#include <vector>
#include <cstddef>
#include <cassert>
#include "sysinfoapi.h"

namespace detail {

 std::vector<char> logical_processor_info(LOGICAL_PROCESSOR_RELATIONSHIP type, boost::system::error_code& error)
 {
    if (auto f = reinterpret_cast<decltype(&GetLogicalProcessorInformationEx)>(GetProcAddress(GetModuleHandleA("kernel32"), "GetLogicalProcessorInformationEx")))
    {
       auto buffer = std::vector<char>(5 * sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX));
       for (;;) {
          auto size = static_cast<boost::winapi::DWORD_>(buffer.size());
          if (f(type, reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(buffer.data()), &size)) {
            buffer.resize(size);
            return buffer;
         }
         switch (auto err = boost::winapi::GetLastError()) {
            case ERROR_INSUFFICIENT_BUFFER:
               buffer.resize(size);
               break;
            case ERROR_SUCCESS:
               assert(!"Oops that's strange");
               return {};
            default:
               error = boost::system::error_code(err, boost::system::system_category());
               return {};
         }
       }
    } else
       error = boost::system::error_code(static_cast<int>(boost::winapi::GetLastError()), boost::system::system_category());
    return {};
 }

 } //detail

std::size_t utils::v1::cache_line_size() noexcept
 {
    constexpr auto default_cache_line_size = std::size_t{64};
    auto error = boost::system::error_code{};
    auto logical_processor_info = ::detail::logical_processor_info(RelationCache, error);
    if (!error) {
       auto const end = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX const*>(logical_processor_info.data()  + logical_processor_info.size());
       for (
           auto info = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX const*>(logical_processor_info.data());
           info < end;
           info = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX const*>(reinterpret_cast<char const*>(info) + info->Size))
         if (info->Relationship == RelationCache && info->Cache.Level == 1)
            return info->Cache.LineSize;
    }
    return default_cache_line_size;
}


std::vector<utils::group_afinity> utils::v1::numa_node_workset(unsigned short node)
{
    auto error = boost::system::error_code{};
    auto logical_processor_info = ::detail::logical_processor_info(RelationNumaNodeEx, error);
    if (error) 
        logical_processor_info = ::detail::logical_processor_info(RelationNumaNode, error);

    static_assert(sizeof(KAFFINITY) == sizeof(group_afinity::mask), "mask has to be adjusted");
    auto res = std::vector<group_afinity>{};
    res.reserve(logical_processor_info.size() / sizeof(NUMA_NODE_RELATIONSHIP));
    auto const end = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX const*>(logical_processor_info.data()  + logical_processor_info.size());
    for (
        auto info = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX const*>(logical_processor_info.data());
        info < end;
        info = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX const*>(reinterpret_cast<char const*>(info) + info->Size)) {
        assert(info->Relationship == RelationNumaNode || info->Relationship == RelationNumaNodeEx);
        if (node == info->NumaNode.NodeNumber) {
          auto const n = info->NumaNode.GroupCount == 0 ? 1 : info->NumaNode.GroupCount;
          for (auto i = decltype(info->NumaNode.GroupCount){0}; i != n; ++i) {
             res.emplace_back(static_cast<unsigned short>(info->NumaNode.GroupMasks[i].Group), static_cast<std::uint64_t>(info->NumaNode.GroupMasks[i].Mask));
          }
        }
    }
    return res;
}

std::vector<utils::core_info> utils::v1::cores_info()
{
    auto error = boost::system::error_code{};
    auto logical_processor_info = ::detail::logical_processor_info(RelationProcessorCore, error);

    auto res = std::vector<core_info>{};
    res.reserve(logical_processor_info.size() / sizeof(PROCESSOR_RELATIONSHIP));
    auto const end = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX const*>(logical_processor_info.data() + logical_processor_info.size());
    for (
        auto info = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX const*>(logical_processor_info.data());
        info < end;
        info = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX const*>(reinterpret_cast<char const*>(info) + info->Size)) {
        assert(info->Relationship == RelationProcessorCore);
        //info->Processor.Flags == LTP_PC_SMT
        auto groups = std::vector<group_afinity>(info->Processor.GroupCount);
        for (auto i = decltype(info->Processor.GroupCount){0}; i != info->Processor.GroupCount; ++i) {
          groups[i].group = static_cast<unsigned short>(info->NumaNode.GroupMasks[i].Group);
          groups[i].mask = static_cast<std::uint64_t>(info->NumaNode.GroupMasks[i].Mask);
        }
        res.emplace_back(static_cast<unsigned short>(info->Processor.EfficiencyClass), std::move(groups));
    }
    return res;
}

//struct cpu_info
//{
//
//};
//
//std::vector<cpu_info> cpu_set() {
//    auto cpu_set_info = std::vector<char>(sizeof(SYSTEM_CPU_SET_INFORMATION) * 16);
//    unsigned long size = 0;
//    if (!GetSystemCpuSetInformation(reinterpret_cast<PSYSTEM_CPU_SET_INFORMATION>(cpu_set_info.data()), cpu_set_info.size(), &size, 0, 0)) {
//        auto error = boost::winapi::GetLastError();
//        if (error != ERROR_INSUFFICIENT_BUFFER)
//          throw boost::system::system_error(error, boost::system::system_category(), "GetSystemCpuSetInformation filed");
//        cpu_set_info.resize(size);
//        if (!GetSystemCpuSetInformation(reinterpret_cast<PSYSTEM_CPU_SET_INFORMATION>(cpu_set_info.data()), cpu_set_info.size(), &size, 0, 0))
//          throw boost::system::system_error(boost::winapi::GetLastError(), boost::system::system_category(), "GetSystemCpuSetInformation filed");
//    }
//
//    struct group
//    {
//        unsigned long long mask;
//        unsigned short numa;
//        unsigned short max_efficiency_class;
//        std::vector<unsigned long long> cores;
//    };
//
//    auto groups = std::vector<group>(1);
//    auto const end = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION const*>(cpu_set_info.data() + size);
//    for (
//        auto cpu_info = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION const*>(cpu_set_info.data());
//        cpu_info < end;
//        cpu_info = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION const*>(reinterpret_cast<char const*>(cpu_info) + cpu_info->Size)) {
//        if (cpu_info->Type == CPU_SET_INFORMATION_TYPE::CpuSetInformation) {
//          std::clog << "\nid " << cpu_info->CpuSet.Id
//                    << "\n\tgroup " << cpu_info->CpuSet.Group
//                    << "\n\tnuma node " << int(cpu_info->CpuSet.NumaNodeIndex)
//                    << "\n\tlogicalProcessorIndex " << int(cpu_info->CpuSet.LogicalProcessorIndex)
//                    << "\n\tcoreIndex " << int(cpu_info->CpuSet.CoreIndex)
//                    << "\n\trealTime " << bool(cpu_info->CpuSet.RealTime)
//                    << "\n\tparked " << bool(cpu_info->CpuSet.Parked)
//                    << "\n\tallocated " << bool(cpu_info->CpuSet.Allocated)
//                    << "\n\tallocatedToTargetProcess " << bool(cpu_info->CpuSet.AllocatedToTargetProcess)
//                    << "\n\tallocationTag " << cpu_info->CpuSet.AllocationTag
//                    << "\n\tefficiencyClass " << int(cpu_info->CpuSet.EfficiencyClass)
//                    << "\n\tschedulingClass " << int(cpu_info->CpuSet.SchedulingClass);
//                    << "\n\LastLevelCacheIndex " << int(cpu_info->CpuSet.LastLevelCacheIndex);
//          if (groups.size() < cpu_info->CpuSet.Group + 1)
//             groups.resize(cpu_info->CpuSet.Group);
//          if (groups[cpu_info->CpuSet.Group].cores.size() < cpu_info->CpuSet.CoreIndex + 1)
//             groups[cpu_info->CpuSet.Group].cores.resize(cpu_info->CpuSet.CoreIndex + 1);
//          groups[cpu_info->CpuSet.Group].cores[cpu_info->CpuSet.CoreIndex] |= (1ull << cpu_info->CpuSet.LogicalProcessorIndex);
//          groups[cpu_info->CpuSet.Group].mask |= (1ull << cpu_info->CpuSet.LogicalProcessorIndex);
//          if (groups[cpu_info->CpuSet.Group].max_efficiency_class < cpu_info->CpuSet.EfficiencyClass)
//             groups[cpu_info->CpuSet.Group].max_efficiency_class = cpu_info->CpuSet.EfficiencyClass;
//        }
//    }
//}


std::vector<utils::cpu_info> utils::v1::cpus_info()
{
    auto cpu_set_info = std::vector<char>(sizeof(SYSTEM_CPU_SET_INFORMATION) * 16);
    unsigned long size = 0;
    if (!GetSystemCpuSetInformation(reinterpret_cast<PSYSTEM_CPU_SET_INFORMATION>(cpu_set_info.data()), static_cast<unsigned long>(cpu_set_info.size()), &size, 0, 0)) {
        auto error = boost::winapi::GetLastError();
        if (error != ERROR_INSUFFICIENT_BUFFER)
          throw boost::system::system_error(error, boost::system::system_category(), "GetSystemCpuSetInformation filed");
        cpu_set_info.resize(size);
        if (!GetSystemCpuSetInformation(reinterpret_cast<PSYSTEM_CPU_SET_INFORMATION>(cpu_set_info.data()), static_cast<unsigned long>(cpu_set_info.size()), &size, 0, 0))
          throw boost::system::system_error(boost::winapi::GetLastError(), boost::system::system_category(), "GetSystemCpuSetInformation filed");
    }

    auto res = std::vector<cpu_info>{};
    auto const end = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION const*>(cpu_set_info.data() + size);
    for (
        auto info = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION const*>(cpu_set_info.data());
        info < end;
        info = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION const*>(reinterpret_cast<char const*>(info) + info->Size)) {
        if (info->Type == CPU_SET_INFORMATION_TYPE::CpuSetInformation) {
          auto entry = cpu_info{0};
          entry.id = static_cast<decltype(entry.id)>(info->CpuSet.Id);
          entry.group = info->CpuSet.Group;
          entry.numa_node = info->CpuSet.NumaNodeIndex;
          entry.cpu_ix = info->CpuSet.LogicalProcessorIndex;
          entry.core_ix = info->CpuSet.CoreIndex;
          entry.flags.real_time = info->CpuSet.RealTime;
          entry.flags.parked = info->CpuSet.Parked;
          entry.flags.allocated = info->CpuSet.Allocated;
          entry.flags.allocated_to_target_process = info->CpuSet.AllocatedToTargetProcess;
          entry.efficiency_class = info->CpuSet.EfficiencyClass;
          // entry.scheduling_class = info->CpuSet.SchedulingClass;
          // entry.allocation_tag = info->CpuSet.AllocationTag;
          res.push_back(std::move(entry));
        }
    }
    return res;
}
