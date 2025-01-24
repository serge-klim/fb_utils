#pragma once
//#include <boost/asio.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <string>

namespace net { inline namespace v0 {

struct endpoint
{
   std::string host;
   unsigned short port;
};

struct scheme_endpoint
{
   std::string scheme;
   endpoint ep;
};


struct credentials
{
   std::string username;
   std::string password;
};
}} // namespace net::v0

BOOST_FUSION_ADAPT_STRUCT(net::endpoint, host, port)
BOOST_FUSION_ADAPT_STRUCT(net::credentials, username, password)
BOOST_FUSION_ADAPT_STRUCT(net::scheme_endpoint, scheme, ep)

namespace net::parser { inline namespace v0 {

namespace x3 = boost::spirit::x3;

inline auto endpoint(unsigned short default_port = 0) noexcept
{
   static auto const parser = x3::rule</*struct*/ net::endpoint, net::endpoint>{"net::endpoint"} = +(x3::char_ - ':') >> ((':' >> x3::ushort_) | x3::attr(default_port));
   return parser;
}

inline auto scheme_endpoint(unsigned short default_port = 0) noexcept
{
   static auto const parser = x3::rule</*struct*/ net::scheme_endpoint, net::scheme_endpoint>{"net::scheme_endpoint"} = 
																								  (+(x3::char_ - ':') >> (("://" >> endpoint()) |( ':' >> x3::attr(net::endpoint{}))))
																								  ;
   return parser;
}

//auto endpoint(unsigned short default_port = 0) noexcept
//{
//   auto make_address = [](auto& ctx) {
//      // auto x1 = std::get<0>(x3::_attr(ctx));
//      // auto x2 = std::get<1>(x3::_attr(ctx));
//      // test_type(x3::_attr(ctx));
//      std::string value = x3::_attr(ctx);
//      x3::_val(ctx).address = boost::asio::ip::make_address(value);
//   };
//   static auto const parser = x3::rule</*struct*/ net::endpoint, net::endpoint>{"net::endpoint"} = x3::omit[+(x3::char_ - ':')][make_address] >> ((':' >> x3::ushort_) | x3::attr(default_port));
//   return parser;
//}


inline auto credentials() noexcept
{
   static auto const parser = x3::rule</*struct*/ net::credentials, net::credentials>{"net::credentials"} = +(x3::char_ - ':') >> ':' >> +x3::char_;
   return parser;
}

}} // namespace net::parser::v0


