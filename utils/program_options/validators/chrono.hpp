#pragma once
#include "../chrono.hpp"
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/any.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <list>
#include <chrono>

namespace boost { namespace program_options { 
//namespace std { namespace chrono {// has to be in std::chrono namcpese:( to make it reacheble for boost::program_options

template <typename Rep, typename Period>
void validate(boost::any& v, std::vector<std::string> const& values, std::chrono::duration<Rep, Period>*, int);
template <typename Rep, typename Period>
void validate(boost::any& v, std::vector<std::string> const& values, std::list<std::chrono::duration<Rep, Period>>*, int);
template <typename Duration>
void validate(boost::any& v, std::vector<std::string> const& values, std::chrono::time_point<std::chrono::system_clock, Duration>*, int);

}} // namespace boost::program_options

#include <boost/program_options.hpp>

namespace boost { namespace program_options { // has to be in std::chrono namcpese:( to make it reacheble for boost::program_options

template <typename Rep, typename Period>
void validate(boost::any& v, std::vector<std::string> const& values,
              std::chrono::duration<Rep, Period>*, int)
{
   /*boost::program_options::*/validators::check_first_occurrence(v);
   auto const& value = boost::program_options::validators::get_single_string(values);
   auto begin = std::begin(value);
   auto end = std::end(value);
   auto res = std::chrono::duration<Rep, Period>{};
   if (!boost::spirit::x3::parse(begin, end, parser::duration<std::chrono::duration<Rep, Period>>(), res) || begin != end)
      throw boost::program_options::validation_error{boost::program_options::validation_error::invalid_option_value, "invalid duration"};

   v = boost::any(std::move(res));
}

template <typename Rep, typename Period>
void validate(boost::any& v, std::vector<std::string> const& values,
              std::list<std::chrono::duration<Rep, Period>>*, int)
{
   /*boost::program_options::*/ validators::check_first_occurrence(v);
   auto const& value = boost::program_options::validators::get_single_string(values);
   auto begin = std::begin(value);
   auto end = std::end(value);
   auto res = std::vector<std::chrono::duration<Rep, Period>>{};
   if (!boost::spirit::x3::parse(begin, end, parser::duration<std::chrono::duration<Rep, Period>>() % ',', res) || begin != end)
      throw boost::program_options::validation_error{boost::program_options::validation_error::invalid_option_value, "invalid durations list"};

   v = boost::any(std::list<std::chrono::duration<Rep, Period>>(cbegin(res), cend(res)));
}

template <typename Duration>
void validate(boost::any& v, std::vector<std::string> const& values,
              std::chrono::time_point<std::chrono::system_clock, Duration>*, int)
{
   /*boost::program_options::*/ validators::check_first_occurrence(v);
   auto const& value = boost::program_options::validators::get_single_string(values);
   try {
      auto ptime = boost::posix_time::time_from_string(value);
      auto time_point = std::chrono::time_point_cast<Duration>(std::chrono::system_clock::from_time_t(to_time_t(ptime)));
      v = boost::any(std::move(time_point));
   } catch (std::exception& e) {
      std::throw_with_nested( std::runtime_error{"can't parse date-time \"" + value + "\" : " + e.what()} );
   }      
}

}} // namespace boost::program_options


