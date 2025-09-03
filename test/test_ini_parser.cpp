#include "test.hpp"
#include "utils/ini_parser/ini_parser.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/test_tools.hpp>

#include "utils/program_options/ini_parser.hpp"
#include <boost/program_options.hpp>
#include <fstream>

#include <filesystem>
#include <strstream>
#include <algorithm>
#include <ranges>
#include <format>
#include <vector>
#include <string>
#include <utility>
#include <stdexcept>

namespace {
class check_message {
 public:
   check_message(std::string msg) : expected_{std::move(msg)} {}
   bool operator()(std::exception const& e) const {
      auto message = e.what();
      auto res = expected_.compare(message) == 0;
      BOOST_CHECK_EQUAL(message, expected_);
      return res;
   }

 private:
   std::string expected_;
};

std::filesystem::path test_data() {
   static auto res = [] -> std::filesystem::path {
       auto test_data = std::filesystem::path("test") / "data";
       auto path = std::filesystem::current_path();
       while (path.has_relative_path()) {
          auto search_path = path / test_data;
          if (exists(search_path)) 
             return search_path;
      
          path = path.parent_path();
          //BOOST_REQUIRE();
       }
      return {};
   }();
   return res;
}

}
using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(utils_ini_parser_test_suite)

std::pair<std::string, std::vector<std::pair<char, std::string_view>>> tokens[] = {
    {"\"test\\\"+++\"", {{'"', "test\\\"+++"} }},
    {"=\"test\\\"+++\"", {{'=', "="}, {'"', "test\\\"+++"} }},
    {"[\"test\\\"+++\"]", {{'[', "\"test\\\"+++\""}  }}
};

BOOST_DATA_TEST_CASE(tokenizer_test, boost::unit_test::data::make(tokens), data) {
   auto tokens = utils::v1::ini::detail::tokenize(data.first);
   BOOST_REQUIRE_EQUAL(tokens.size(), data.second.size());
   for (auto ix : std::views::iota(static_cast<decltype(tokens)::size_type>(0), tokens.size())) {
      auto parsed = std::format("{}:{}", tokens[ix].first, tokens[ix].second);
      auto expected = std::format("{}:{}",data.second[ix].first,data.second[ix].second);
      BOOST_CHECK_EQUAL(parsed, expected);
   }
   // BOOST_CHECK_EQUAL_COLLECTIONS(cbegin(res), cend(res), cbegin(data.second), cend(data.second));
}


std::pair<std::string, utils::ini::pairs> input_output[] = {
    {""s, {}},
    {"\n   "s, {}},
    {" #\n######\n   "s, {}},
    {"      a=b #     "s, {{"a"s, "b"s}}},
    {"a=b"s, {{"a"s, "b"s}}},
    {"a\"=\"b\""s, {{"a\""s, "b"s}}},
    {"a=\"b\""s, {{"a"s, "b"s}}},
    {"a=\"\""s, {{"a"s, ""s}}},
    {"a=b#"s, {{"a"s, "b"s}}},
    {"a=b #"s, {{"a"s, "b"s}}},
    //{"a="s, {{"a"s, ""s}}},
    //{"a=\""s, {{"a"s, "\""s}}},
    {"a=b ###########"s, {{"a"s, "b"s}}},
    {"a=b ##include"s, {{"a"s, "b"s}}},
    {"[section.1]\n"
      "a=b"s, 
                {{"section.1.a"s, "b"s}}},
    //{"[\"section.1\"]\n"
    //  "a=b"s, 
    //            {{"section.1.a"s, "b"s}}},
    {"[section.1]\n"
      "x.y.a=b"s, 
                {{"section.1.x.y.a"s, "b"s}}},
    {"x = \"Filter=%Channel% != \\\"stats\\\" | %Severity% >= info\""s, {{"x"s, "Filter=%Channel% != \"stats\" | %Severity% >= info"s}}}
};


BOOST_DATA_TEST_CASE(parse_test, boost::unit_test::data::make(input_output), data) {
   auto in = std::stringstream{data.first};
   auto res = utils::ini::parse(in);
   BOOST_REQUIRE_EQUAL(res.size(), data.second.size());
   for (auto ix : std::views::iota(static_cast<decltype(res)::size_type>(0), res.size())) {
      auto parsed = res[ix].first + '=' + res[ix].second;
      auto expected = data.second[ix].first + '=' + data.second[ix].second;
      BOOST_CHECK_EQUAL(parsed, expected);
   }
   //BOOST_CHECK_EQUAL_COLLECTIONS(cbegin(res), cend(res), cbegin(data.second), cend(data.second));
}

std::pair<std::string, std::string> broken_input[] = {
    {"\n\n=b"s,"(3): unexpected '=' at the begining of line"s},
    {"["s, "(1): unmatched '['"s},
    //{"[#]"s, "(1"s}, // not sure how to treat it
    {"[#"s, "(1): unmatched '['"s}, // not sure how to treat it
    {"\n[]"s,"(2): empty section"s},
    {"#include"s, "(1): filename expected after \"#include\""s},
    {"#include \""s, "(1): unmatched quote"s},
    {"#include \"\""s, "(1): filename expected after \"#include\""s},
    {"#include \"     \""s, "(1): filename expected after \"#include\""s},
    {"#include test#xxx"s, "(1): can't open #include file \"test\""s},
    {"#include \"test#xxx\""s, "(1): can't open #include file \"test#xxx\""s}
};

BOOST_DATA_TEST_CASE(parse_broken_input_test, boost::unit_test::data::make(broken_input), data) {
   auto in = std::stringstream{data.first};
   BOOST_CHECK_EXCEPTION(utils::ini::parse(in), std::runtime_error, check_message{data.second});
}

BOOST_AUTO_TEST_CASE(include_test)
{
   auto path = test_data();
   auto in_broken = std::stringstream{"#include missing.conf  # not existing file"};
   BOOST_CHECK_EXCEPTION(utils::ini::parse(in_broken, path), std::runtime_error, check_message{"(1): can't open #include file \"missing.conf\""});
   auto in_broken2 = std::stringstream{"\n#include \"missing.conf\""};
   BOOST_CHECK_EXCEPTION(utils::ini::parse(in_broken2, path), std::runtime_error, check_message{"(2): can't open #include file \"missing.conf\""});

   auto in = std::stringstream{"a=b   # some comment\n"
                               "  [ section a           ]\n"
                               " a=\"b\"       \n"
                                "#include extra.conf      ; this file should exists"};
   auto values = utils::ini::parse(in, path);
   BOOST_REQUIRE_EQUAL(values.size(), 5);
   BOOST_CHECK_EQUAL(values[0].first, "a");
   BOOST_CHECK_EQUAL(values[0].second, "b");
   BOOST_CHECK_EQUAL(values[1].first, "section a.a");
   BOOST_CHECK_EQUAL(values[1].second, "b");

   BOOST_CHECK_EQUAL(values[2].first, "extra");
   BOOST_CHECK_EQUAL(values[2].second, "ok");
   BOOST_CHECK_EQUAL(values[3].first, "extra.1.a");
   BOOST_CHECK_EQUAL(values[3].second, "a");
   BOOST_CHECK_EQUAL(values[4].first, "extra.1.b");
   BOOST_CHECK_EQUAL(values[4].second, "other stuf");
}

//BOOST_AUTO_TEST_CASE(program_options_test)
//{
//   auto const file_path = test_data() / "sockperf-ping-lx4.config";
//   auto values = utils::ini::parse(file_path);
//   auto ifs = std::ifstream{test_data() / "sockperf-ping-lx4.expected.config"};
//   BOOST_REQUIRE(static_cast<bool>(ifs));
//   auto description = boost::program_options::options_description{"configuration"};
//   auto parsed = parse_config_file(ifs, description, true);
//   for (auto const& option : parsed.options) {
//      auto i = std::ranges::find_if(values, [&option](auto const& v) {
//         return v.first == option.string_key; 
//      });
//      if (i == cend(values)) {
//         BOOST_CHECK_EQUAL("<not found>", option.string_key);
//      }
//      BOOST_REQUIRE(!option.value.empty());
//      BOOST_CHECK_EQUAL(std::string{i->second}, std::string{option.value.front()});
//   }
//   for (auto const& value : values) {
//      auto i = std::ranges::find_if(parsed.options, [&value](auto const& option) {
//         return value.first == option.string_key;
//      });
//      if (i == cend(parsed.options)) {
//         BOOST_CHECK_EQUAL("<not found>", value.first);
//      }
//   }
//   BOOST_CHECK_EQUAL(parsed.options.size(), values.size());
//}


BOOST_AUTO_TEST_SUITE_END()
