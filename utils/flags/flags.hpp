#pragma once
#include <charconv>
#include <algorithm>
#include <string>
#include <string_view>
#include <span>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <concepts>

namespace utils { inline namespace v1 { namespace flags {

template <class T>
concept flags_strings = requires(T const& t) {
   { std::span{t}.front() } -> std::convertible_to<std::string>;
};

template <std::integral T, flags_strings Strings>
std::string to_string(T value, Strings&& strings, char delim = '|') {
   auto const sstrings = std::span{strings};
   auto res = std::string{};
   if (value != 0) {
      auto i = std::size_t{0};
      do {
         if ((value & 1) == 1) {
            res += i < sstrings.size() ? sstrings[i] : std::to_string(1 << i);
            res += delim;
         }
         value >>= 1;
         ++i;
      } while (value != 0);
      res.pop_back();
   }
   return res;
}

template <std::integral T, std::contiguous_iterator I, flags_strings Strings>
T parse_string(I& begin, I end, Strings const& strings, char delim = '|') {
   auto const sstrings = std::span{strings};
   auto res = T{0};
   auto begin_strings = cbegin(sstrings);
   auto end_strings = cend(sstrings);
   for (;;) {
      auto last = std::find(begin, end, delim);
      if (last != begin) {
         auto token = std::string_view{begin, last};
         auto i = std::find(begin_strings, end_strings, token);
         if (i != end_strings)
            res |= (T{1} << std::distance(begin_strings, i));
         else {
            auto begin = token.data();
            auto end = begin + token.size();
            if (begin == end)
               break;
            if (*begin != '\0' && ++begin != end) {
               auto base = 10;
               if (*begin == 'x' || *begin == 'X') {
                  base = 16;
                  ++begin;
               }
               if (begin == end)
                  break;
               auto val = T{0};
               auto [ptr, ec] = std::from_chars(begin, end, val, base);
               if (ec != std::errc{} || ptr != end)
                  break;
               res |= val;
            }
         }
      }
      begin = last;
      if (begin == end || ++begin == end)
         break;
   }
   return res;
}

template <std::integral T, flags_strings Strings>
T from_string(std::string_view text, Strings const& strings, char delim = '|') {
   auto begin = cbegin(text);
   auto end = cend(text);
   auto res = parse_string<T>(begin, end, strings, delim);
   if (begin != end)
      throw std::runtime_error{"unknown flag : " + std::string{begin, end}};
   return res;
}

}}} // namespace utils::v1::flags
