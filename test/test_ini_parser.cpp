#include "test.hpp"
#include "utils/ini_parser/ini_parser.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/test_tools.hpp>

#include <fstream>

#include <filesystem>
#include <strstream>
#include <algorithm>
#include <ranges>
#include <format>
#include <vector>
#include <string>
#include <string_view>
#include <utility>
#include <stdexcept>

namespace {
class check_message {
 public:
   check_message(std::string msg) : expected_{std::move(msg)} {}
   bool operator()(std::exception const& e) const {
      auto message = std::string_view{e.what()};
      auto res = message.compare(expected_) == 0;
      if (!res)
         BOOST_CHECK_EQUAL(message, expected_);
      return res;
   }

 private:
   std::string expected_;
};

class check_message_relaxed {
 public:
   check_message_relaxed(std::string msg) : expected_{std::move(msg)} {}
   bool operator()(std::exception const& e) const {
      auto message = std::string_view{e.what()};
      auto res = message.contains(expected_);
      if (!res)
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

} // namespace

using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(utils_ini_parser_test_suite)



std::pair<std::string, std::vector<std::pair<char, std::string_view>>> tokens[] = {
    {"\"test\\\"+++\"", {{'"', "test\\\"+++"} }},
    {"=\"test\\\"+++\"", {{'=', "="}, {'"', "test\\\"+++"} }},
    {"[\"test\\\"+++\"]", {{'[', "\"test\\\"+++\""}  }},
    {"key=value", {{'t', "key"}, {'=', "="}, {'t', "value"}}},
    {"  key  =  value  ", {{'t', "key"}, {'=', "="}, {'t', "value  "}}},
    {"#comment", {{'#', "comment"}}},
    {";comment", {{';', "comment"}}},
    {"[section]", {{'[', "section"}}},
    {"#include file.conf", {{'i', "include"}, {'t', "file.conf"}}},
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
    {"path=c:\\"s, {{"path"s, "c:\\"s}}},
    {"path=\"c:\\\\\""s, {{"path"s, "c:\\"s}}},
    {"#included"s, {}},
    {"#included something"s, {}},
    {"\n   "s, {}},
    {" #\n######\n   "s, {}},
    {"      a=b #     "s, {{"a"s, "b"s}}},
    {" \ta \t=\tb\t c\t #     "s, {{"a"s, "b\t c"s}}},
    {"a=b"s, {{"a"s, "b"s}}},
    {"\" a\"=b"s, {{" a"s, "b"s}}},
    {"\"o\\\\k\"=b"s, {{"o\\k"s, "b"s}}},
    {"\"o\\\"k\"=b"s, {{"o\"k"s, "b"s}}},
    {"a\"=\"b\""s, {{"a\""s, "b"s}}},
    {"a=\"b\""s, {{"a"s, "b"s}}},
    {"a=\"\""s, {{"a"s, ""s}}},
    {"a=b#"s, {{"a"s, "b"s}}},
    {"a=b #"s, {{"a"s, "b"s}}},
    {"a=b==c"s, {{"a"s, "b==c"s}}},
    {"a=  b==c "s, {{"a"s, "b==c"s}}},
    {"a="s, {{"a"s, ""s}}},
    {"a=\"\""s, {{"a"s, ""s}}},
    {"a=b ###########"s, {{"a"s, "b"s}}},
    {"a=b #include"s, {{"a"s, "b"s}}},
    {"a=b ##include"s, {{"a"s, "b"s}}},
    {"Filter=%Channel% = \"stats\""s, {{"Filter"s, "%Channel% = \"stats\""s}}},
    {"Filter=\"%Channel% = \\\"stats\\\"\""s, {{"Filter"s, "%Channel% = \"stats\""s}}},
    {"[section.1.]\na=b", {{"section.1.a"s, "b"s}}},
    {"[section.1]\t#ok\na=b", {{"section.1.a"s, "b"s}}},
    {"[section.1]\n"
      "a=b"s, 
                {{"section.1.a"s, "b"s}}},
    {"[\"section.1\"]\n"
      "a=b"s, 
                {{"section.1.a"s, "b"s}}},
    {"[\t\"section.1\"   ]\n"
     "a=b"s,
     {{"section.1.a"s, "b"s}}},
    {"[section.1]\n"
      "x.y.a=b"s, 
                {{"section.1.x.y.a"s, "b"s}}},
    {"[Sinks.1]\n"
     "Format = [% Channel % ] % TimeStamp % % Message %"s,
     {{"Sinks.1.Format"s, "[% Channel % ] % TimeStamp % % Message %"s}}},
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
    {"\n\n=b"s,"(3): unexpected \"=\" at the beginning of line"s},
    {"a=\"\\b\""s, "(1): invalid escaped symbol \"\\b\""s},
    {"a=\"\\"s, "(1): missing '\"' or '\\' after '\\'"s},
    {"a=\"b\"c"s, "(1): \"c\" - unexpected"s},
    {"a=\"b\"cd"s, "(1): \"cd\" - unexpected"s},
    {"a=\"b\" junk"s, "(1): \"junk\" - unexpected"s},
    {"\"\"=\"b\""s, "(1): empty keys are not supported"s},
    {"\"  \"=\"b\""s, "(1): empty keys are not supported"s},
    {"=\"b\""s, "(1): unexpected \"=\" at the beginning of line"s},
    {"["s, "(1): unmatched '['"s},
    {"a==b"s, "(1): \"=\" - unexpected after \"=\""s},
    //{"[#]"s, "(1"s}, // not sure how to treat it
    {"[#"s, "(1): unmatched '['"s}, // not sure how to treat it
    {"\n[]"s,"(2): empty section"s},
    {"\n[    ]"s, "(2): empty section"s},
    {"\n[  \t\r  ]"s, "(2): empty section"s},
    {"\n[\"\"]"s, "(2): empty section"s},
    {"\n[\"              \"]"s, "(2): empty section"s},
    {"[\"\"\"]"s, "(1): inconsistent quotes in section [\"\"\"]"s},
    {"[\"]"s, "(1): inconsistent quotes in section [\"]"s},
    {"#include"s, "(1): filename expected after \"#include\""s},
    {"#include \""s, "(1): unmatched quote"s},
    {"#include \"\""s, "(1): filename expected after \"#include\""s},
    {"#include \"     \""s, "(1): filename expected after \"#include\""s}
};

BOOST_DATA_TEST_CASE(parse_broken_input_test, boost::unit_test::data::make(broken_input), data) {
   auto in = std::stringstream{data.first};
   BOOST_CHECK_EXCEPTION(utils::ini::parse(in), std::runtime_error, check_message{data.second});
}

std::pair<std::string, std::string> broken_includes[] = {
    {"#include test#xxx"s, "(1): can't open #include file \"test\""s},
    {"#include \"test#xxx\""s, "(1): can't open #include file \"test#xxx\""s},
    {"#include \"test#xxx\"  ####"s, "(1): can't open #include file \"test#xxx\""s}
};

BOOST_DATA_TEST_CASE(parse_broken_include_test, boost::unit_test::data::make(broken_includes), data) {
   auto in = std::stringstream{data.first};
   BOOST_CHECK_EXCEPTION(utils::ini::parse(in), std::runtime_error, check_message_relaxed{data.second});
}


BOOST_AUTO_TEST_CASE(include_test)
{
   auto path = test_data();
   auto in_broken = std::stringstream{"#include missing.conf  # not existing file"};
   BOOST_CHECK_EXCEPTION(utils::ini::parse(in_broken, path), std::runtime_error, check_message_relaxed{"(1): can't open #include file \"missing.conf\" -> "});
   auto in_broken2 = std::stringstream{"\n#include \"missing.conf\""};
   BOOST_CHECK_EXCEPTION(utils::ini::parse(in_broken2, path), std::runtime_error, check_message_relaxed{"(2): can't open #include file \"missing.conf\" -> "});

   auto in_broken3 = std::stringstream{"\n#include \"\\\"somewhere\\\\missing.conf\""};
   BOOST_CHECK_EXCEPTION(utils::ini::parse(in_broken3, path), std::runtime_error, check_message_relaxed{"(2): can't open #include file \"\"somewhere\\missing.conf\" -> "});

   auto in = std::stringstream{"a=b   # some comment\n"
                               "  [ section a           ]\n"
                               " a=\"b\"       \n"
                              "#include extra.conf      ; this file should exist"};
   auto values = utils::ini::parse(in, path);
   BOOST_REQUIRE_EQUAL(values.size(), 6);
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
   BOOST_CHECK_EQUAL(values[5].first, "extra.1.a");
   BOOST_CHECK_EQUAL(values[5].second, "X");
}

BOOST_AUTO_TEST_CASE(include_cycle_test) {
   auto path = test_data();
   path /= "cycle.conf";
   BOOST_CHECK_EXCEPTION(utils::ini::parse(path), std::runtime_error, check_message_relaxed{"include cycle :"});
}

BOOST_AUTO_TEST_CASE(empty_file_test)
{
   auto in = std::stringstream{""};
   auto res = utils::ini::parse(in);
   BOOST_CHECK(res.empty());
}

BOOST_AUTO_TEST_CASE(only_comments_test)
{
   auto in = std::stringstream{"# comment 1\n; comment 2\n# comment 3"};
   auto res = utils::ini::parse(in);
   BOOST_CHECK(res.empty());
}

BOOST_AUTO_TEST_CASE(whitespace_only_test)
{
   auto in = std::stringstream{"   \n\t\n  \t  "};
   auto res = utils::ini::parse(in);
   BOOST_CHECK(res.empty());
}

BOOST_AUTO_TEST_CASE(section_without_trailing_dot_test)
{
   // Section names without trailing dot should have one appended
   auto in = std::stringstream{"[mysection]\nkey=value"};
   auto res = utils::ini::parse(in);
   BOOST_REQUIRE_EQUAL(res.size(), 1);
   BOOST_CHECK_EQUAL(res[0].first, "mysection.key");
   BOOST_CHECK_EQUAL(res[0].second, "value");
}

BOOST_AUTO_TEST_CASE(section_with_trailing_dot_test)
{
   // Section names with trailing dot should not get another
   auto in = std::stringstream{"[mysection.]\nkey=value"};
   auto res = utils::ini::parse(in);
   BOOST_REQUIRE_EQUAL(res.size(), 1);
   BOOST_CHECK_EQUAL(res[0].first, "mysection.key");
   BOOST_CHECK_EQUAL(res[0].second, "value");
}

BOOST_AUTO_TEST_CASE(multiple_sections_test)
{
   auto in = std::stringstream{
       "global=value1\n"
       "[section1]\n"
       "key1=value2\n"
       "[section2]\n"
       "key2=value3"};
   auto res = utils::ini::parse(in);
   BOOST_REQUIRE_EQUAL(res.size(), 3);
   BOOST_CHECK_EQUAL(res[0].first, "global");
   BOOST_CHECK_EQUAL(res[0].second, "value1");
   BOOST_CHECK_EQUAL(res[1].first, "section1.key1");
   BOOST_CHECK_EQUAL(res[1].second, "value2");
   BOOST_CHECK_EQUAL(res[2].first, "section2.key2");
   BOOST_CHECK_EQUAL(res[2].second, "value3");
}

BOOST_AUTO_TEST_CASE(escaped_quotes_in_values_test)
{
   auto in = std::stringstream{"key=\"value with \\\"quotes\\\" inside\""};
   auto res = utils::ini::parse(in);
   BOOST_REQUIRE_EQUAL(res.size(), 1);
   BOOST_CHECK_EQUAL(res[0].first, "key");
   BOOST_CHECK_EQUAL(res[0].second, "value with \"quotes\" inside");
}

BOOST_AUTO_TEST_CASE(escaped_backslashes_test)
{
   auto in = std::stringstream{"path=\"C:\\\\Users\\\\test\""};
   auto res = utils::ini::parse(in);
   BOOST_REQUIRE_EQUAL(res.size(), 1);
   BOOST_CHECK_EQUAL(res[0].first, "path");
   BOOST_CHECK_EQUAL(res[0].second, "C:\\Users\\test");
}

BOOST_AUTO_TEST_CASE(hash_in_value_test)
{
   // # in the middle of an unquoted value should be treated as comment start
   auto in = std::stringstream{"key=value#comment"};
   auto res = utils::ini::parse(in);
   BOOST_REQUIRE_EQUAL(res.size(), 1);
   BOOST_CHECK_EQUAL(res[0].first, "key");
   BOOST_CHECK_EQUAL(res[0].second, "value");
}

BOOST_AUTO_TEST_CASE(hash_in_quoted_value_test)
{
   // # in a quoted value should be preserved
   auto in = std::stringstream{"key=\"value#notacomment\""};
   auto res = utils::ini::parse(in);
   BOOST_REQUIRE_EQUAL(res.size(), 1);
   BOOST_CHECK_EQUAL(res[0].first, "key");
   BOOST_CHECK_EQUAL(res[0].second, "value#notacomment");
}

BOOST_AUTO_TEST_CASE(equals_in_value_test)
{
   // = in value should be preserved
   auto in = std::stringstream{"key=a=b=c"};
   auto res = utils::ini::parse(in);
   BOOST_REQUIRE_EQUAL(res.size(), 1);
   BOOST_CHECK_EQUAL(res[0].first, "key");
   BOOST_CHECK_EQUAL(res[0].second, "a=b=c");
}

BOOST_AUTO_TEST_SUITE_END()
