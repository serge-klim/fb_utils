#pragma once
#include <charconv>
#include <algorithm>
#include <string>
#include <string_view>
#include <span>
#include <stdexcept>
#include <iterator>
#include <type_traits>
#include <concepts>
#include <utility>
#include <cstddef>
#include <cassert>

namespace utils { inline namespace v1 { namespace flags {

template <class T>
concept flags_strings = requires(T const& t) {
   { std::span{t}.front() } -> std::convertible_to<std::string_view>;
};

template <std::integral T, flags_strings Strings>
std::string to_string(T value, Strings&& strings, char delim = '|') {
   auto const sstrings = std::span{strings};
   auto res = std::string{};
   if (auto unsigned_value = static_cast<std::make_unsigned_t<T>>(value)) {
      auto i = std::size_t{0};
      do {
         if ((unsigned_value & 1) == 1) {
            res += i < sstrings.size() ? sstrings[i] : std::to_string(std::make_unsigned_t<T>{1} << i);
            res += delim;
         }
         unsigned_value >>= 1;
         ++i;
      } while (unsigned_value != 0);
      res.pop_back();
   }
   return res;
}

template <std::integral T>
std::pair<bool, T> parse_numeric(std::string_view token) {
   assert(!token.empty());
   auto begin = token.data();
   auto end = begin + token.size();
   auto base = 10;
   if (*begin == '0') {
      if (++begin == end)
         return {true, 0};
      if (*begin == 'x' || *begin == 'X') {
         base = 16;
         if (++begin == end)
            return {false, 0};
      }
   }
   auto val = T{0};
   auto [ptr, ec] = std::from_chars(begin, end, val, base);
   return {ec == std::errc{} && ptr == end, val};
}

template <std::integral T, std::contiguous_iterator I, flags_strings Strings>
T parse_string(I& begin, I end, Strings const& strings, char delim = '|') {
   auto const sstrings = std::span{strings};
   assert(sizeof(T) / sizeof(std::byte) * 8 >= sstrings.size());
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
            auto [ok, val] = parse_numeric<T>(token);
            if (!ok)
               break;
            res |= val;
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
