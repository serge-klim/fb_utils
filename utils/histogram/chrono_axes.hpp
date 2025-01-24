#pragma once

#include <boost/histogram.hpp>
#include <numeric>
#include <chrono>
#include <tuple>
#include <ranges>
#include <ostream>
#include <iomanip>

namespace histogram::axis { inline namespace v1 {

template <typename T>
struct duration_adapter;

template <typename Rep, typename Period>
struct duration_adapter<std::chrono::duration<Rep, Period>> {
   template <typename Rep1, typename Period1>
   duration_adapter(std::chrono::duration<Rep1, Period1> const& duration) : value{std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(duration)} {}
   std::chrono::duration<Rep, Period> value;
};

template <typename T, typename Base /*, template <class...> class Base, typename... Options*/>
struct duration;

template <typename Rep, typename Period, template <class...> class Base, typename... Options>
struct duration<std::chrono::duration<Rep, Period>, Base<Rep, Options...>> : Base<Rep, Options...> {
   using base = Base<Rep, Options...>;
   using Base<Rep, Options...>::Base;

   auto index(duration_adapter<std::chrono::duration<Rep, Period>> const& duration) {
      return base::index(duration.value.count());
   }

   auto update(duration_adapter<std::chrono::duration<Rep, Period>> const& duration) {
      return base::update(duration.value.count());
   }
};

template <typename Rep, typename Period>
std::string readable_time(std::chrono::duration<Rep, Period> dur) noexcept {
   auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(dur);
   if (ns.count() == 0)
      return {'0'};
   if (ns < std::chrono::minutes{1}) {
      if (ns.count() % std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds{1}).count() == 0)
         return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(ns).count()) + " s";
      if (ns.count() % std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds{1}).count() == 0)
         return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(ns).count()) + " ms";
      if (ns.count() % std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::microseconds{1}).count() == 0)
         return std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(ns).count()) + " us";

      return std::to_string(ns.count()) + " ns";
   }
   return ns < std::chrono::minutes{9}
              ? std::to_string(static_cast<double>(ns.count()) / std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds{1}).count()) + " s"
              : std::to_string(static_cast<double>(ns.count()) / std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::minutes{1}).count()) + " min";
}

// template <typename T, template <class...> class Base, typename... Options>
// template <class Axes, class Storage>
// std::ostream& dump(std::ostream& out, boost::histogram::histogram<Axes, Storage> /*duration<T, Base, Options...>*/ const& histogram)
template <typename Rep, typename Period, typename Base, typename Storage>
std::ostream& dump(std::ostream& out, boost::histogram::histogram<std::tuple<duration<std::chrono::duration<Rep, Period>, Base>>, Storage> const& histogram) {
   using row = std::tuple<int, std::string, long long>;
   auto rows = std::vector<row>{};
   auto ixs = indexed(histogram, boost::histogram::coverage::all);
   auto max_time_width = 0;
   for (auto&& ix : ixs) {
      if (auto value = ix.get()) {
         auto time = readable_time(std::chrono::duration<Rep, Period>{static_cast<Rep>(ix.bin().lower())});
         time += '-';
         time += readable_time(std::chrono::duration<Rep, Period>{static_cast<Rep>(ix.bin().upper())});
         if (auto const width = time.size(); width > max_time_width)
            max_time_width = width;
         rows.emplace_back(
             row{
                 ix.index(),
                 std::move(time),
                 static_cast<long long>(value)});
      }
   }
   if (!rows.empty()) {
      auto const ix_width = std::get<0>(rows.back()) < 999 ? 3 : 5;
      for (auto const& row : rows) {
         out << std::setw(ix_width) << std::get<0>(row)
             << " : [ "
             << std::setw(max_time_width) << std::get<1>(row)
             << ") : " << std::get<2>(row) << '\n';
      }
      out << "    ---------------\n\t" << std::fixed << std::accumulate(cbegin(histogram), cend(histogram), .0) << '\n';
   }
   return out;
}

}} // namespace histogram::axis::v1
