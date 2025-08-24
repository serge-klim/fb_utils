#include "ini_parser.hpp"
#include <locale>
#include <string_view>
#include <iostream>
#include <fstream>
#include <optional>
#include <format>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <cassert>

namespace {

constexpr char space() noexcept { return ' '; }

std::string_view trim(std::string_view value) noexcept {
   auto first = value.find_first_not_of(space());
   if (first == std::string_view::npos)
      return {};
   auto last = value.find_last_not_of(space());
   return value.substr(first, last - first + 1);
}

void parse_ini(std::istream& in, std::filesystem::path const& search_path, std::vector<std::pair<std::string, std::string>>& values);
void process_include(std::filesystem::path const& search_path, std::vector<std::pair<std::string, std::string>>& values, std::vector<std::pair<char, std::string_view>> const& tokens)
{
   switch (tokens.size()) {
      case 2: {
         if (tokens[1].first != '"' && tokens[1].first != 't')
            throw std::runtime_error{std::format("filename expected after \"#include\", \"{}\" - found", tokens[1].second)};
         auto filename = trim(tokens[1].second);
         if (!filename.empty()) {
            auto f = std::fstream{std::string{filename}};
            if (!f && (f.open(search_path / filename), !f)) {
               throw std::runtime_error{std::format("can't open #include file \"{}\"", filename)};
            }
            parse_ini(f, search_path, values);
            break;
         }
         [[fallthrough]];
      }
      case 1:
         throw std::runtime_error{"filename expected after \"#include\""};
      default:
         assert(tokens.size() >= 3);
         throw std::runtime_error{std::format("\"{}\" - unexpected after \"{}\"", tokens[2].second, tokens[1].second)};
   }
}


std::pair<std::string, std::string> process_value(std::vector<std::pair<char, std::string_view>> const& tokens) {
   switch (auto n = tokens.size()) {
      default: {
         assert(n != 0);
         auto begin = tokens[2].second.data();
         auto const& last_token = tokens.back();
         auto end = last_token.second.data() + last_token.second.size();
         return {std::string{tokens[0].second}, std::string{begin, end}};
      }
      case 2: {
         if (tokens[1].first != '=')
            throw std::runtime_error{std::format("\"{}\" - unexpected", tokens[1].second)};
         // empty values are fine
         return {std::string{tokens[0].second}, {}};
      }
      case 1:
         throw std::runtime_error{"missing value"};
   }
}

std::pair<std::string, std::string> process_quoted_value(std::vector<std::pair<char, std::string_view>> const& tokens)
{
   if (tokens[2].first != '"' && tokens[2].first != 't')
      throw std::runtime_error{std::format("\"{}\" - unexpected after '='", tokens[2].second)};
   auto value = std::string{};
   if (auto n = tokens[2].second.size()) {
      value.reserve(n);
      value.push_back(tokens[2].second[0]);
      for (auto i = decltype(n){1}; i != n; ++i) {
         if ((tokens[2].second[i] == '\\' || tokens[2].second[i] == '"') && tokens[2].second[i - 1] == '\\')
            value.back() = tokens[2].second[i];
         else
            value.push_back(tokens[2].second[i]);
      }
   }
   return {std::string{tokens[0].second}, std::move(value)};
}


void parse_ini(std::istream& in, std::filesystem::path const& search_path, std::vector<std::pair<std::string, std::string>>& values) {
auto prefix = std::string{};
   auto line = std::string{};
   for (auto line_n = 1; getline(in, line); ++line_n) {
      try {
         auto tokens = utils::v1::ini::detail::tokenize(line);
         tokens.erase(std::ranges::find_if(tokens, [](auto const& token) constexpr {
            return token.first == '#' || token.first == ';'; })
             , end(tokens));
         if (auto size = tokens.size()) {
            using std::operator""sv;
            static constexpr auto incl = "include"sv;
            switch (tokens[0].first) {
               case 'i':
                  process_include(search_path, values, tokens);
                  break;
               case '[': {
                  auto name = trim(tokens[0].second);
                  if (name.empty())
                     throw std::runtime_error{"empty section"};
                  if (size > 1) 
                      throw std::runtime_error{std::format("unexpected token found after section [{}]", name)};
                  prefix = name;
                  prefix += '.';
                  break;
               }
               case '=':
                  throw std::runtime_error{"unexpected '=' at the begining of line"};
               case 't': 
                 [[fallthrough]];
               case '"':
                  switch (size) {
                    default: {
                        assert(size >= 3);
                        if (tokens[1].first != '=')
                           throw std::runtime_error{std::format("\"{}\" - unexpected", tokens[1].second)};
                        switch (tokens[2].first) {
                           case 't': {
                              auto value = process_value(tokens);
                              values.emplace_back(prefix + std::string{value.first}, std::move(value.second));
                              break;
                           }
                           case '"': {
                              if (size != 3)
                                 throw std::runtime_error{std::format("\"{}\" - unexpected, after quoted string, please use \" around value and escape \" with \\", tokens[3].second)};

                              auto value = process_quoted_value(tokens);
                              values.emplace_back(prefix + std::string{value.first}, std::move(value.second));
                              break;
                           }
                        }
                        break;
                     }
                     case 2: {
                        if (tokens[1].first != '=')
                           throw std::runtime_error{std::format("\"{}\" - unexpected", tokens[1].second)};
                        // empty values are fine
                        values.emplace_back(prefix + std::string{tokens[0].second}, std::string{});
                        break;
                     }
                     case 1:
                        throw std::runtime_error{"missing value"};
                  }

            }
         }
      }
      catch (std::runtime_error const& e) {
         //throw std::runtime_error{std::format("unable parse line {} : {}", line_n, e.what())};
         throw std::runtime_error{std::format("({}): {}", line_n, e.what())};
      }
   }
}

} // namespace

utils::v1::ini::pairs utils::v1::ini::parse(std::istream& in, std::filesystem::path const& search_path /*= std::filesystem::path{}*/) {
   auto res = pairs{};
   parse_ini(in, search_path, res);
   return res;
}

utils::v1::ini::pairs utils::v1::ini::parse(std::filesystem::path const& filename) try {
   auto f = std::fstream{filename};
   if (!f)
      throw std::runtime_error{std::format("unable open included file \"{}\"", filename.string())};
   return parse(f, filename.parent_path());
} catch (std::runtime_error& e) {
   throw std::runtime_error{std::format("{}:{}", filename.string(), e.what())};
}


std::vector<std::pair<char, std::string_view>> utils::v1::ini::detail::tokenize(std::string_view input) {
   auto res = std::vector<std::pair<char, std::string_view>>{};
   auto const n = input.size();
   auto last = n - n;
   for (auto first = input.find_first_not_of(' '); first < n; first = input.find_first_not_of(' ', last + 1)) {
      switch (auto c = input[first]) {
         case '"': {
            last = first;
            do {
               if (last + 1 == n || (last = input.find('"', last + 1)) == std::string_view::npos)
                  throw std::runtime_error{"unmatched quote"};
            } while (input[last - 1] == '\\');
            res.emplace_back('"', input.substr(first + 1, last - first - 1));
            break;
         }
         case '[':
            last = input.find(']', first + 1);
            if (last == std::string_view::npos)
               throw std::runtime_error{"unmatched '['"};
            res.emplace_back('[', input.substr(first + 1, last - first - 1));
            break;
         case '#': {
            using std::operator""sv;
            static constexpr auto incl = "include"sv;
            auto comment = input.substr(first + 1);
            if (comment.starts_with(incl)) {
               last = first + incl.size();
               res.emplace_back('i', input.substr(first + 1, incl.size()));
               break;
            }
            [[fallthrough]];
         }           
         case ';':
            res.emplace_back(c, input.substr(first + 1));
            return res;
         case '=':
            res.emplace_back(c, input.substr(first, 1));
            last = first;
            break;
         default: {
            if (last + 1 == n || (last = input.find_first_of(" =#;[]" /* " quate - inside text condidered as part of text*/, first + 1)) == std::string_view::npos) {
               res.emplace_back('t', input.substr(first));
               return res;
             }
             res.emplace_back('t', input.substr(first, last - first));
             if (input[last] != ' ')
                --last;
         }
      }
   }

   return res;
}
