#pragma once
#include "../net.hpp"
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/any.hpp>
#include <boost/asio.hpp>
#include <boost/url/url.hpp>
#include <boost/url/grammar/error.hpp>
#include <boost/url/parse.hpp>
#include <stdexcept>

namespace boost { 
    
namespace program_options { inline void validate(boost::any& v, std::vector<std::string> const& values, boost::asio::ip::address*, int);}
namespace urls { inline void validate(boost::any& v, std::vector<std::string> const& values, boost::urls::url*, int);}

} // namespace boost::program_options

#include <boost/program_options.hpp>

namespace boost { 
namespace program_options { // has to be in std::chrono namcpese:( to make it reacheble for boost::program_options

inline void validate(boost::any& v, std::vector<std::string> const& values, boost::asio::ip::address*, int)
{
   /*boost::program_options::*/ validators::check_first_occurrence(v);
   auto const& value = boost::program_options::validators::get_single_string(values);
   auto error = boost::system::error_code{};
   auto address = boost::asio::ip::address::from_string(value, error);
   if (!!error)
       throw std::runtime_error{"can't parse ip address \"" + value + "\" : " + error.message()};
   v = boost::any(std::move(address));
}

inline void validate(boost::any& v, std::vector<std::string> const& values, boost::asio::ip::address_v4*, int)
{
   /*boost::program_options::*/ validators::check_first_occurrence(v);
   auto const& value = boost::program_options::validators::get_single_string(values);
   auto error = boost::system::error_code{};
   auto address = boost::asio::ip::address_v4::from_string(value, error);
   if (!!error)
       throw std::runtime_error{"can't parse ip v4 address \"" + value + "\" : " + error.message()};
   v = boost::any(std::move(address));
}

inline void validate(boost::any& v, std::vector<std::string> const& values, boost::asio::ip::address_v6*, int)
{
   /*boost::program_options::*/ validators::check_first_occurrence(v);
   auto const& value = boost::program_options::validators::get_single_string(values);
   auto error = boost::system::error_code{};
   auto address = boost::asio::ip::address_v6::from_string(value, error);
   if (!!error)
       throw std::runtime_error{"can't parse ip v6 address \"" + value + "\" : " + error.message()};
   v = boost::any(std::move(address));
}

} // namespace program_options

namespace urls {

inline void validate(boost::any& v, std::vector<std::string> const& values, boost::urls::url*, int)
{
   boost::program_options::validators::check_first_occurrence(v);
   auto const& value = boost::program_options::validators::get_single_string(values);
   auto res = boost::urls::parse_uri(value);
   if (res.has_error()) 
   {
       auto const& error = res.error();
       if (error != boost::system::error_code{boost::urls::grammar::error::need_more})
            throw std::runtime_error{"can't parse url \"" + value + "\" : " + error.message()};
       auto url = boost::urls::url{};
       url.set_host(value);
       v = boost::any(std::move(url));
       return;
   }
   v = boost::any(boost::urls::url{*res});
}


}// namespace urls

} // namespace boost


namespace net {

inline void validate(boost::any& v, std::vector<std::string> const& values, net::endpoint*, int)
{
   boost::program_options::validators::check_first_occurrence(v);
   auto const& value = boost::program_options::validators::get_single_string(values);
   auto begin = std::begin(value);
   auto end = std::end(value);
   auto res = endpoint{};
   if (!boost::spirit::x3::parse(begin, end, parser::endpoint(), res) || begin != end)
      throw boost::program_options::validation_error{boost::program_options::validation_error::invalid_option_value, "invalid endpoint"};

   v = boost::any(std::move(res));
}

inline void validate(boost::any& v, std::vector<std::string> const& values, net::credentials*, int)
{
   boost::program_options::validators::check_first_occurrence(v);
   auto const& value = boost::program_options::validators::get_single_string(values);
   auto begin = std::begin(value);
   auto end = std::end(value);
   auto res = credentials{};
   if (!boost::spirit::x3::parse(begin, end, parser::credentials(), res) || begin != end)
      throw boost::program_options::validation_error{boost::program_options::validation_error::invalid_option_value, "invalid credentials"};

   v = boost::any(std::move(res));
}

inline void validate(boost::any& v, std::vector<std::string> const& values, net::scheme_endpoint*, int)
{
   boost::program_options::validators::check_first_occurrence(v);
   auto const& value = boost::program_options::validators::get_single_string(values);
   auto begin = std::begin(value);
   auto end = std::end(value);
   auto res = scheme_endpoint{};
   if (!boost::spirit::x3::parse(begin, end, parser::scheme_endpoint(), res) || begin != end) {
      res.scheme = std::string{};
      begin = std::begin(value);
      if (!boost::spirit::x3::parse(begin, end, parser::endpoint(), res.ep) || begin != end)
            throw boost::program_options::validation_error{boost::program_options::validation_error::invalid_option_value, "schema-endpoint"};
   }

   v = boost::any(std::move(res));
}
} //namespace net
