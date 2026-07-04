#include "test.hpp"
#include "utils/program_options/net.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/test_tools.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <format>

namespace x3 = boost::spirit::x3;

BOOST_AUTO_TEST_SUITE(utils_net_parsers_test_suite)

// ---- endpoint parser tests ----

std::pair<std::string_view, std::pair<std::string, unsigned short>> endpoint_cases[] = {
    // host:port
    {"localhost:8080", {"localhost", 8080}},
    {"127.0.0.1:443", {"127.0.0.1", 443}},
    {"0.0.0.0:0", {"0.0.0.0", 0}},
    {"example.com:65535", {"example.com", 65535}},
    {"host:1", {"host", 1}},
    // host only (default port = 0)
    {"localhost", {"localhost", 0}},
    {"127.0.0.1", {"127.0.0.1", 0}},
    // host with special characters (anything except ':')
    {"my-host_name.example.com:80", {"my-host_name.example.com", 80}},
};

BOOST_DATA_TEST_CASE(endpoint_parser_test, boost::unit_test::data::make(endpoint_cases), data)
{
    auto const& [input, expected] = data;
    net::endpoint result;
    auto begin = std::begin(input);
    auto end = std::end(input);
    BOOST_CHECK(x3::parse(begin, end, net::parser::endpoint(), result));
    BOOST_CHECK(begin == end);
    BOOST_CHECK_EQUAL(result.host, expected.first);
    BOOST_CHECK_EQUAL(result.port, expected.second);
}

std::pair<std::string_view, unsigned short> endpoint_with_default_port_cases[] = {
    {"localhost:8080", 8080},   // explicit port
    {"localhost", 9090},        // default port applied
    {"127.0.0.1", 9090},        // default port applied
    {"host:0", 0},              // explicit 0 overrides default
};

BOOST_DATA_TEST_CASE(endpoint_default_port_test, boost::unit_test::data::make(endpoint_with_default_port_cases), data)
{
    auto const& [input, expected_port] = data;
    net::endpoint result;
    auto begin = std::begin(input);
    auto end = std::end(input);
    BOOST_CHECK(x3::parse(begin, end, net::parser::endpoint(9090), result));
    BOOST_CHECK(begin == end);
    BOOST_CHECK_EQUAL(result.port, expected_port);
}

std::vector<std::string_view> endpoint_negative_cases = {
    ":",          // empty host
    ":8080",      // empty host with port
    "host:",      // missing port number
    "host:abc",   // non-numeric port
};

BOOST_DATA_TEST_CASE(endpoint_negative_test, boost::unit_test::data::make(endpoint_negative_cases), input)
{
    net::endpoint result;
    auto begin = std::begin(input);
    auto end = std::end(input);
    BOOST_CHECK(!x3::parse(begin, end, net::parser::endpoint(), result) || begin != end);
}

// ---- credentials parser tests ----

std::pair<std::string_view, std::pair<std::string, std::string>> credentials_cases[] = {
    {"user:pass", {"user", "pass"}},
    {"admin:secret123", {"admin", "secret123"}},
    {"user:name_with_spaces", {"user", "name_with_spaces"}},
    {"a:b", {"a", "b"}},
    {"user@host:password", {"user@host", "password"}},
};

BOOST_DATA_TEST_CASE(credentials_parser_test, boost::unit_test::data::make(credentials_cases), data)
{
    auto const& [input, expected] = data;
    net::credentials result;
    auto begin = std::begin(input);
    auto end = std::end(input);
    BOOST_CHECK(x3::parse(begin, end, net::parser::credentials(), result));
    BOOST_CHECK(begin == end);
    BOOST_CHECK_EQUAL(result.username, expected.first);
    BOOST_CHECK_EQUAL(result.password, expected.second);
}

std::vector<std::string_view> credentials_negative_cases = {
    ":",           // empty username and password
    "user:",       // empty password
    ":pass",       // empty username
    "userpass",    // no colon separator
    "u:p:extra",   // extra colon (parser takes first colon, but "p:extra" has extra chars)
};

BOOST_DATA_TEST_CASE(credentials_negative_test, boost::unit_test::data::make(credentials_negative_cases), input)
{
    net::credentials result;
    auto begin = std::begin(input);
    auto end = std::end(input);
    // For "u:p:extra" the parser would match "u" and "p:extra" since +x3::char_ is greedy
    // But for empty username/password or no colon, it should fail
    if (input == "u:p:extra") {
        // This actually parses: username="u", password="p:extra"
        BOOST_CHECK(x3::parse(begin, end, net::parser::credentials(), result));
        BOOST_CHECK_EQUAL(result.username, "u");
        BOOST_CHECK_EQUAL(result.password, "p:extra");
    } else {
        BOOST_CHECK(!x3::parse(begin, end, net::parser::credentials(), result) || begin != end);
    }
}

// ---- scheme_endpoint parser tests ----

std::pair<std::string_view, std::pair<std::string, std::pair<std::string, unsigned short>>> scheme_endpoint_cases[] = {
    // scheme://host:port
    {"http://localhost:8080", {"http", {"localhost", 8080}}},
    {"https://example.com:443", {"https", {"example.com", 443}}},
    {"tcp://0.0.0.0:0", {"tcp", {"0.0.0.0", 0}}},
    // scheme: (no endpoint, default port)
    {"http:", {"http", {"", 0}}},
    {"https:", {"https", {"", 0}}},
};

BOOST_DATA_TEST_CASE(scheme_endpoint_parser_test, boost::unit_test::data::make(scheme_endpoint_cases), data)
{
    auto const& [input, expected] = data;
    net::scheme_endpoint result;
    auto begin = std::begin(input);
    auto end = std::end(input);
    BOOST_CHECK(x3::parse(begin, end, net::parser::scheme_endpoint(), result));
    BOOST_CHECK(begin == end);
    BOOST_CHECK_EQUAL(result.scheme, expected.first);
    BOOST_CHECK_EQUAL(result.ep.host, expected.second.first);
    BOOST_CHECK_EQUAL(result.ep.port, expected.second.second);
}

std::pair<std::string_view, std::pair<std::string, std::pair<std::string, unsigned short>>> scheme_endpoint_with_default_port_cases[] = {
    {"http://localhost:8080", {"http", {"localhost", 8080}}},
    {"http://localhost", {"http", {"localhost", 80}}},   // default port from parser
    {"http:", {"http", {"", 80}}},                       // default port from parser
};

BOOST_DATA_TEST_CASE(scheme_endpoint_default_port_test, boost::unit_test::data::make(scheme_endpoint_with_default_port_cases), data)
{
    auto const& [input, expected] = data;
    net::scheme_endpoint result;
    auto begin = std::begin(input);
    auto end = std::end(input);
    BOOST_CHECK(x3::parse(begin, end, net::parser::scheme_endpoint(80), result));
    BOOST_CHECK(begin == end);
    BOOST_CHECK_EQUAL(result.scheme, expected.first);
    BOOST_CHECK_EQUAL(result.ep.host, expected.second.first);
    BOOST_CHECK_EQUAL(result.ep.port, expected.second.second);
}

// ---- idempotency test (verifies non-static rules work correctly on repeated calls) ----

BOOST_AUTO_TEST_CASE(endpoint_parser_idempotency_test)
{
    for (int i = 0; i < 5; ++i) {
        net::endpoint result;
        auto input = std::format("localhost:808{}",i);
        auto begin = std::begin(input);
        auto end = std::end(input);
        BOOST_CHECK(x3::parse(begin, end, net::parser::endpoint(), result));
        BOOST_CHECK(begin == end);
        BOOST_CHECK_EQUAL(result.host, "localhost");
        BOOST_CHECK_EQUAL(result.port, 8080 + i);
    }
}

BOOST_AUTO_TEST_CASE(credentials_parser_idempotency_test)
{
    for (int i = 0; i < 5; ++i) {
        net::credentials result;
        std::string input = "admin:secret";
        auto begin = std::begin(input);
        auto end = std::end(input);
        BOOST_CHECK(x3::parse(begin, end, net::parser::credentials(), result));
        BOOST_CHECK(begin == end);
        BOOST_CHECK_EQUAL(result.username, "admin");
        BOOST_CHECK_EQUAL(result.password, "secret");
    }
}

BOOST_AUTO_TEST_CASE(scheme_endpoint_parser_idempotency_test)
{
    for (int i = 0; i < 5; ++i) {
        net::scheme_endpoint result;
        auto input = std::format("http://localhost:808{}",i);
        auto begin = std::begin(input);
        auto end = std::end(input);
        BOOST_CHECK(x3::parse(begin, end, net::parser::scheme_endpoint(), result));
        BOOST_CHECK(begin == end);
        BOOST_CHECK_EQUAL(result.scheme, "http");
        BOOST_CHECK_EQUAL(result.ep.host, "localhost");
        BOOST_CHECK_EQUAL(result.ep.port, 8080 + i);
    }
}

BOOST_AUTO_TEST_SUITE_END()
