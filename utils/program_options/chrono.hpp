#pragma once
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/spirit/home/x3.hpp>
#include <utility>
#include <chrono>
#include <type_traits>

namespace parser {

//namespace detail::pure_man_concept {
//template <typename T>
//struct is_duration : std::false_type{};
//
//template <typename Rep, typename Period>
//struct is_duration<std::chrono::duration<Rep, Period>> : std::true_type{};
//
//} //namespace detail::pure_man_concept


template <typename Duration>
using parsed_duration = std::pair<typename Duration::rep, Duration (*)(typename Duration::rep) noexcept>;

template <typename Duration>
auto duration()
{
   // static_assert(detail::pure_man_concept::is_duration<Duration>::value, "please specialize with chrono duration only!");
   using reps = typename Duration::rep;
   using duration_cast = typename parsed_duration<Duration>::second_type /*Duration (*)(reps) noexcept*/;
   struct duration_parser : boost::spirit::x3::symbols<duration_cast>
   {
      duration_parser()
      {
         // clang-format off
          boost::spirit::x3::symbols<duration_cast>::add
              ("ns", [](reps n) noexcept { return std::chrono::duration_cast<Duration>(std::chrono::nanoseconds{n});})
              ("us", [](reps n) noexcept { return std::chrono::duration_cast<Duration>(std::chrono::microseconds{n});})
              ("ms", [](reps n) noexcept { return std::chrono::duration_cast<Duration>(std::chrono::milliseconds{n});})               
              ("s" , [](reps n) noexcept { return std::chrono::duration_cast<Duration>(std::chrono::seconds{n});})
	    	  ("m" , [](reps n) noexcept { return std::chrono::duration_cast<Duration>(std::chrono::minutes{n});})
	    	  ("h" , [](reps n) noexcept { return std::chrono::duration_cast<Duration>(std::chrono::hours{n});})
	       ;
         // clang-format on
      }
   } static const dparser;

   auto defaut_duration_cast = [](reps n) noexcept { return Duration{n}; };
   constexpr auto rep_parser = boost::spirit::x3::uint_parser<reps>{};
   return boost::spirit::x3::rule<struct duration, parsed_duration<Duration>>{"duration"} = rep_parser >> (dparser | boost::spirit::x3::attr(static_cast<duration_cast>(defaut_duration_cast)));
}

} // namespace parser

namespace boost::spirit::x3::traits {

template <typename Rep, typename Period>
struct transform_attribute<std::chrono::duration<Rep, Period>, ::parser::parsed_duration<std::chrono::duration<Rep, Period>>, boost::spirit::x3::parser_id>
{
   using type = ::parser::parsed_duration<std::chrono::duration<Rep, Period>>;

   static type pre(std::chrono::duration<Rep, Period> const&) { return type{}; }

   static void post(std::chrono::duration<Rep, Period>& val, type&& attribute)
   {
      val = (*attribute.second)(attribute.first);
   }
};

} // namespace boost::spirit::x3::traits



