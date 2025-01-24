#pragma once

#include "utils/flags/flags.hpp"
#include <boost/spirit/home/x3.hpp>
#include <span>
#include <string_view>
#include <concepts>

namespace parser {
// template <typename T/*, typename Encoding*/>
template <typename T, typename String, std::size_t Extent = std::dynamic_extent, char Delimiter = '|'>
   requires(std::convertible_to<String, std::string_view>)
struct flags_parser : boost::spirit::x3::parser<flags_parser<T, String, Extent /*, Encoding*/>> {
   // using encoding = Encoding;
   using attribute_type = T;
   static constexpr bool has_attribute = true;

   /*constexpr*/ flags_parser(std::span<String, Extent>&& strings) noexcept : strings_{std::move(strings)} {}

   template <typename Iterator, typename Context>
   bool parse(Iterator& first, Iterator const& last, Context const& context, boost::spirit::x3::unused_type, T& attr) const {
      boost::spirit::x3::skip_over(first, last, context);
      auto res = first == last;
      if (!res) {
         auto tmp = first;
         auto val = utils::flags::parse_string<T>(first, last, strings_, Delimiter);
         if (tmp != first) {
            boost::spirit::x3::traits::move_to(val, attr);
            return true;
         }
      } else
         boost::spirit::x3::traits::move_to(0, attr);
      return res;
   }

   template <typename Iterator, typename Context, typename Attribute>
   bool parse(Iterator& first, Iterator const& last, Context const& context, boost::spirit::x3::unused_type unused, Attribute& attr_param) const {
      // this case is called when Attribute is not T
      T attr;
      auto const res = parse(first, last, context, unused, attr);
      if (res)
         boost::spirit::x3::traits::move_to(attr, attr_param);
      return res;
   }

 private:
   std::span<String, Extent> strings_;
};

template <typename T, utils::flags::flags_strings Strings>
auto make_flags_parser(Strings&& strings) noexcept {
   auto span = std::span{std::forward<Strings>(strings)};
   return flags_parser<T, typename decltype(span)::element_type, decltype(span)::extent>{std::move(span)};
}

} // namespace parser

namespace boost { namespace spirit { namespace x3 {

template <typename T, typename String, std::size_t Extent, char Delimiter>
struct get_info<::parser::flags_parser<T, String, Extent, Delimiter>, void> {
   using result_type = const char*;
   result_type operator()(::parser::flags_parser<T, String, Extent, Delimiter> const&) const { return "flags"; }
};

}}} // namespace boost::spirit::x3