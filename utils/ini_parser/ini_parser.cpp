#include "ini_parser.hpp"
#include <string_view>
#include <fstream>
#include <format>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <cassert>

namespace {

constexpr auto escapee() noexcept { return std::string_view{"\\\""}; }
constexpr auto token_delimiters() noexcept { return std::string_view{" \t\r#;="}; }
constexpr auto space() noexcept { return token_delimiters().substr(0, 3) /*std::string_view{" \t\r"}*/; }
constexpr auto comment() noexcept { return token_delimiters().substr(3, 2) /*std::string_view{"#;"}*/; }   
auto is_space(char c) noexcept { return space().find(c) != std::string_view::npos; }

using token = std::pair<char, std::string_view>;

class ini_parser {
 public:
   auto const& values() const { return values_; }
   //void operator()(std::filesystem::path const& filename);
   void operator()(std::istream& in, std::filesystem::path const& filename);
 private:
   void process_include(std::vector<token> const& tokens);
 private:
   utils::v1::ini::pairs values_;
   std::vector<std::filesystem::path> includes_;
};

//void ini_parser::operator()(std::filesystem::path const& filename){
//  auto f = std::ifstream{filename};
//  if (!f) 
//      throw std::runtime_error{std::format("can't open file \"{}\"", filename.string())};
//  (*this)(f, filename);
//}

std::string_view trim(std::string_view value) noexcept {
   auto first = value.find_first_not_of(space());
   if (first == std::string_view::npos)
      return {};
   auto last = value.find_last_not_of(space());
   return value.substr(first, last - first + 1);
}

std::string process_quoted_value(std::string_view value) {
   auto res = std::string{};
   auto end = cend(value);
   auto first = cbegin(value);
   for (auto i = first;; ++i) {
      auto last = std::find(i, end, '\\');
      res.append(first, last);
      if (last == end)
         break;
      assert(std::distance(last, end) >= 2 && "this should be handled by tokenizer");
      i = first = ++last;
   }
   return res;
}

std::string process_key(token const& key) {
   switch (key.first) {
      case 't':
         return std::string{trim(key.second)};
      case '"': {
         auto res = process_quoted_value(key.second);
         if (res.empty() || trim(res).empty())
            throw std::runtime_error{"empty keys are not supported"};
         return res;
       }
   }
   throw std::runtime_error{std::format("unexpected \"{}\" at the beginning of line", key.second)};
}

void ini_parser::process_include(std::vector<token> const& tokens) {
   switch (tokens.size()) {
      case 2: {
         auto filename = std::filesystem::path{};
         switch (tokens[1].first) {
            case 't':
               filename = trim(tokens[1].second);
               break;
            case '"': {
               auto unquoted = process_quoted_value(tokens[1].second);
               filename = trim(unquoted);
               break;
            }
            default:
               throw std::runtime_error{std::format("filename expected after \"#include\", \"{}\" - found", tokens[1].second)};
         }
         if (!filename.empty()) { 
            auto file_path = filename.is_absolute() ? filename : includes_.back().parent_path() / filename;
            auto f = std::ifstream{file_path};
            if (!f)
               throw std::runtime_error{std::format("can't open #include file \"{}\" -> \"{}\"", filename.string(), file_path.string())};            
            file_path = canonical(file_path);
            if (auto i = std::ranges::find(includes_, file_path); i != cend(includes_)) {
               auto cycle = std::accumulate(i, end(includes_), std::string{}, [](std::string res, std::filesystem::path const& path) {
                  res += "\n\t";
                  res += path.string();
                  return res;
               });
               cycle += "\n\t-->";
               cycle += filename.string();
               cycle += " [";
               cycle += file_path.string();
               cycle += ']';
               throw std::runtime_error{"include cycle : " + cycle};
            }
            (*this)(f, std::move(file_path));
            //parse_ini(f, search_path, values);
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

//template <char C = '\"'>
//std::string_view::size_type find_char(std::string_view input) {
std::string_view::size_type find_quote(std::string_view input) {
   auto const n = input.size();
   for (auto i = decltype(n){0}; i < n; ++i) {
      switch (input[i]) {
         case /*C*/'"':
            return i;
         case '\\':
            if (++i == n)
               throw std::runtime_error{"missing '\"' or '\\' after '\\'"};
            if (escapee().find(input[i]) == std::string_view::npos)
               throw std::runtime_error{std::format("invalid escaped symbol \"\\{}\"", input[i])};
      }
   }
   return std::string_view::npos;
}

void ini_parser::operator()(std::istream& in, std::filesystem::path const& filename) {
   assert(filename.is_absolute());
   struct sentry{
      sentry(decltype(ini_parser::includes_)& includes, std::filesystem::path const& filename) : includes_(includes) {
         includes_.push_back(filename);
      }
      ~sentry() {
         includes_.pop_back();
      }
      decltype(ini_parser::includes_)& includes_;
   } snt{includes_, filename};
   
   auto prefix = std::string{};
   auto line = std::string{};
   line.reserve(512);
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
                  process_include(tokens);
                  break;
               case '[': {
                  auto name = trim(tokens[0].second);
                  auto name_size = name.size();
                  if (name_size == 0)
                     throw std::runtime_error{"empty section"};
                  if (size > 1) 
                      throw std::runtime_error{std::format("unexpected token found after section [{}]", tokens[0].second)};
                  if (name[0] == '"') {
                     auto pos = find_quote(name.substr(1));
                     if (pos == std::string_view::npos || pos != name_size - 2)
                        throw std::runtime_error{std::format("inconsistent quotes in section [{}]", tokens[0].second)};
                     prefix = process_quoted_value(name.substr(1, name_size - 2));
                     if (trim(prefix).empty())
                        throw std::runtime_error{"empty section"};
                  } else
                     prefix = name;
                  if (!prefix.ends_with('.'))
                    prefix += '.';
                  break;
               }
               default:
                  auto key = prefix + process_key(tokens[0]);
                  switch (size) {
                    default: {
                        assert(size >= 3);
                        if (tokens[1].first != '=')
                           throw std::runtime_error{std::format("\"{}\" - unexpected", tokens[1].second)};
                        switch (tokens[2].first) {
                           case 't': {
                              if (size != 3)
                                 throw std::runtime_error{std::format("\"{}\" - unexpected", tokens[3].second)};
                              values_.emplace_back(key, std::string{trim(tokens[2].second)});
                              break;
                           }
                           case '"': {
                              if (size != 3)
                                 throw std::runtime_error{std::format("\"{}\" - unexpected", tokens[3].second)};
                              auto value = process_quoted_value(/*tokens*/ tokens[2].second);
                              values_.emplace_back(key, std::move(value));
                              break;
                           }
                           default : 
                             throw std::runtime_error{std::format("\"{}\" - unexpected after \"=\"", tokens[2].second)};
                        }
                        break;
                     }
                     case 2: {
                        if (tokens[1].first != '=')
                           throw std::runtime_error{std::format("\"{}\" - unexpected", tokens[1].second)};
                        // empty values are fine
                        values_.emplace_back(key, std::string{});
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

utils::v1::ini::pairs parse_(std::istream& in, std::filesystem::path const& filename /*= std::filesystem::current_path()*/) {
   auto parser = ini_parser{};
   parser(in, filename);
   return parser.values();
}

} // namespace

utils::v1::ini::pairs utils::v1::ini::parse(std::istream& in, std::filesystem::path const& filename /*= std::filesystem::current_path()*/) {
   return parse_(in, weakly_canonical(is_directory(filename) ? filename / "this.ini" : filename));
}

utils::v1::ini::pairs utils::v1::ini::parse(std::filesystem::path const& filename) {
   auto f = std::ifstream{filename};
   if (!f)
      throw std::runtime_error{std::format("unable open file \"{}\"", filename.string())};
   try {
      return parse_(f, canonical(filename));
   } catch (std::runtime_error& e) {
      throw std::runtime_error{filename.string() + e.what()};
   }
}

std::vector<token> utils::v1::ini::detail::tokenize(std::string_view input) {
   auto res = std::vector<token>{};
   auto const n = input.size();
   auto last = decltype(n){0};
   for (auto first = input.find_first_not_of(space()); first < n; first = input.find_first_not_of(space(), last + 1)) {
      switch (auto c = input[first]) {
         case '"': { 
            auto pos = find_quote(input.substr(++first));
            if (pos == std::string_view::npos)
               throw std::runtime_error{"unmatched quote"};
            res.emplace_back('"', input.substr(first, pos));
            last = first + pos;
            break;
         }
         case '#': {
            using std::operator""sv;
            static constexpr auto incl = "include"sv;
            auto comment = input.substr(first + 1);
            if (res.empty() && comment.starts_with(incl)) {
               last = first + incl.size();
               if (last + 1 == n || is_space(input[last + 1]) /*|| input[last + 1] == '"'*/) {
                  res.emplace_back('i', input.substr(first + 1, incl.size()));
                  break;
               }
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
         case '[':
            //if it's not first token, treating it as value 
            if (res.empty()) {
               if (last + 1 == n || (last = input.find(']', first + 1)) == std::string_view::npos)
                  throw std::runtime_error{"unmatched '['"};
               res.emplace_back('[', input.substr(first + 1, last - first - 1));
               break;
            }
            [[fallthrough]];
         default: {
            auto const delimiters = res.empty() ? token_delimiters() : comment(); // unless first token '=' treated as normal character 
            if (last + 1 == n || (last = input.find_first_of(delimiters, first + 1)) == std::string_view::npos) {
               res.emplace_back('t', input.substr(first));
               return res;
             }
             res.emplace_back('t', input.substr(first, last - first));
             if (!is_space(input[last]))
                --last;
         }
      }
   }

   return res;
}
