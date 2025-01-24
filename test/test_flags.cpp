#include "test.hpp"
#include "utils/flags/parser/flags.hpp"
#include "utils/flags/flags.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/test_tools.hpp>
#include <array>

namespace {

static constexpr char const* strings[] = {
    "RTE_ETH_RX_OFFLOAD_VLAN_STRIP",       // RTE_BIT64(0)
    "RTE_ETH_RX_OFFLOAD_IPV4_CKSUM",       // RTE_BIT64(1)
    "RTE_ETH_RX_OFFLOAD_UDP_CKSUM",        // RTE_BIT64(2)
    "RTE_ETH_RX_OFFLOAD_TCP_CKSUM",        // RTE_BIT64(3)
    "RTE_ETH_RX_OFFLOAD_TCP_LRO",          // RTE_BIT64(4)
    "RTE_ETH_RX_OFFLOAD_QINQ_STRIP",       // RTE_BIT64(5)
    "RTE_ETH_RX_OFFLOAD_OUTER_IPV4_CKSUM", // RTE_BIT64(6)
    "RTE_ETH_RX_OFFLOAD_MACSEC_STRIP",     // RTE_BIT64(7)
    "0x00100",
    "RTE_ETH_RX_OFFLOAD_VLAN_FILTER", // RTE_BIT64(9)
    "RTE_ETH_RX_OFFLOAD_VLAN_EXTEND", // RTE_BIT64(10)
    "0x00800",
    "0x01000"
    "RTE_ETH_RX_OFFLOAD_SCATTER", // RTE_BIT64(13)
    /**
     * Timestamp is set by the driver in RTE_MBUF_DYNFIELD_TIMESTAMP_NAME
     * and RTE_MBUF_DYNFLAG_RX_TIMESTAMP_NAME is set in ol_flags.
     * The mbuf field and flag are registered when the offload is configured.
     */
    "RTE_ETH_RX_OFFLOAD_TIMESTAMP",       // RTE_BIT64(14)
    "RTE_ETH_RX_OFFLOAD_SECURITY",        // RTE_BIT64(15)
    "RTE_ETH_RX_OFFLOAD_KEEP_CRC",        // RTE_BIT64(16)
    "RTE_ETH_RX_OFFLOAD_SCTP_CKSUM",      // RTE_BIT64(17)
    "RTE_ETH_RX_OFFLOAD_OUTER_UDP_CKSUM", // RTE_BIT64(18)
    "RTE_ETH_RX_OFFLOAD_RSS_HASH",        // RTE_BIT64(19)
    "RTE_ETH_RX_OFFLOAD_BUFFER_SPLIT",    // RTE_BIT64(20)
};

static constexpr auto strings_array = std::array<char const*, 20>{
    "RTE_ETH_RX_OFFLOAD_VLAN_STRIP",       // RTE_BIT64(0)
    "RTE_ETH_RX_OFFLOAD_IPV4_CKSUM",       // RTE_BIT64(1)
    "RTE_ETH_RX_OFFLOAD_UDP_CKSUM",        // RTE_BIT64(2)
    "RTE_ETH_RX_OFFLOAD_TCP_CKSUM",        // RTE_BIT64(3)
    "RTE_ETH_RX_OFFLOAD_TCP_LRO",          // RTE_BIT64(4)
    "RTE_ETH_RX_OFFLOAD_QINQ_STRIP",       // RTE_BIT64(5)
    "RTE_ETH_RX_OFFLOAD_OUTER_IPV4_CKSUM", // RTE_BIT64(6)
    "RTE_ETH_RX_OFFLOAD_MACSEC_STRIP",     // RTE_BIT64(7)
    "0x00100",
    "RTE_ETH_RX_OFFLOAD_VLAN_FILTER", // RTE_BIT64(9)
    "RTE_ETH_RX_OFFLOAD_VLAN_EXTEND", // RTE_BIT64(10)
    "0x00800",
    "0x01000"
    "RTE_ETH_RX_OFFLOAD_SCATTER", // RTE_BIT64(13)
    /**
     * Timestamp is set by the driver in RTE_MBUF_DYNFIELD_TIMESTAMP_NAME
     * and RTE_MBUF_DYNFLAG_RX_TIMESTAMP_NAME is set in ol_flags.
     * The mbuf field and flag are registered when the offload is configured.
     */
    "RTE_ETH_RX_OFFLOAD_TIMESTAMP",       // RTE_BIT64(14)
    "RTE_ETH_RX_OFFLOAD_SECURITY",        // RTE_BIT64(15)
    "RTE_ETH_RX_OFFLOAD_KEEP_CRC",        // RTE_BIT64(16)
    "RTE_ETH_RX_OFFLOAD_SCTP_CKSUM",      // RTE_BIT64(17)
    "RTE_ETH_RX_OFFLOAD_OUTER_UDP_CKSUM", // RTE_BIT64(18)
    "RTE_ETH_RX_OFFLOAD_RSS_HASH",        // RTE_BIT64(19)
    "RTE_ETH_RX_OFFLOAD_BUFFER_SPLIT",    // RTE_BIT64(20)
};
} // namespace

BOOST_AUTO_TEST_SUITE(utils_flags_test_suite)

BOOST_AUTO_TEST_CASE(flags_to_string_test)
{
   BOOST_CHECK(utils::flags::to_string(0, strings).empty());
   BOOST_CHECK_EQUAL(utils::flags::to_string(1, strings), "RTE_ETH_RX_OFFLOAD_VLAN_STRIP");
   BOOST_CHECK_EQUAL(utils::flags::to_string(1 << 2, strings), "RTE_ETH_RX_OFFLOAD_UDP_CKSUM");
   BOOST_CHECK_EQUAL(utils::flags::to_string(1 << 7, strings), "RTE_ETH_RX_OFFLOAD_MACSEC_STRIP");
   BOOST_CHECK_EQUAL(utils::flags::to_string(1 << 8, strings), "0x00100");
   BOOST_CHECK_EQUAL(utils::flags::to_string(1 << 10, strings), "RTE_ETH_RX_OFFLOAD_VLAN_EXTEND");

   BOOST_CHECK_EQUAL(utils::flags::to_string(1 << 27, strings), "134217728");
}

BOOST_AUTO_TEST_CASE(combined_flags_to_string_test)
{
   BOOST_CHECK(utils::flags::to_string(0, strings).empty());
   BOOST_CHECK_EQUAL(utils::flags::to_string((1 << 8) | (1 << 10) | (1 << 27), strings), "0x00100|RTE_ETH_RX_OFFLOAD_VLAN_EXTEND|134217728");
   BOOST_CHECK_EQUAL(utils::flags::to_string(1 << 27, strings), "134217728");
}

std::pair<std::string, unsigned int> flags_tuples[] = {
    {"", 0},
    {"0", 0},
    {"0|0|0", 0},
    {"0xa|0xa|10", 10},
    {"RTE_ETH_RX_OFFLOAD_VLAN_STRIP", 1},
    {"0|RTE_ETH_RX_OFFLOAD_VLAN_STRIP", 1},
    {"0|RTE_ETH_RX_OFFLOAD_VLAN_STRIP|0", 1},
    {"0|RTE_ETH_RX_OFFLOAD_VLAN_STRIP|0xa", 1 | 10},
    {"0xB|RTE_ETH_RX_OFFLOAD_VLAN_STRIP", 1 | 11},
    {"RTE_ETH_RX_OFFLOAD_VLAN_EXTEND", 1 << 10},
    {"RTE_ETH_RX_OFFLOAD_VLAN_EXTEND|RTE_ETH_RX_OFFLOAD_MACSEC_STRIP|RTE_ETH_RX_OFFLOAD_VLAN_STRIP", (1 << 10) | (1 << 7) | 1}};

BOOST_DATA_TEST_CASE(parse_test, boost::unit_test::data::make(flags_tuples), data)
{
   BOOST_CHECK_EQUAL(utils::flags::from_string<std::uint64_t>(data.first, strings), data.second);
}

BOOST_DATA_TEST_CASE(parse_from_array_test, boost::unit_test::data::make(flags_tuples), data)
{
   BOOST_CHECK_EQUAL(utils::flags::from_string<std::uint64_t>(data.first, strings_array), data.second);
}

BOOST_DATA_TEST_CASE(parse_from_vector_test, boost::unit_test::data::make(flags_tuples), data)
{
   BOOST_CHECK_EQUAL(utils::flags::from_string<std::uint64_t>(data.first, std::vector<char const*>{cbegin(strings_array), cend(strings_array)}), data.second);
}

BOOST_DATA_TEST_CASE(x3_parser_test, boost::unit_test::data::make(flags_tuples), data)
{
   auto const& [input, expected] = data;
   auto begin = std::begin(input);
   namespace x3 = boost::spirit::x3;
   unsigned int output;
   BOOST_CHECK(x3::phrase_parse(begin, end(input), parser::make_flags_parser<decltype(output)>(strings), x3::blank | x3::eol, output));
   BOOST_CHECK_EQUAL(output, expected);
   BOOST_CHECK(begin == end(input));
}

auto broken_flags_tuples = std::vector<std::string>{
    {"  "},
    {"0x"},
    {"00x"},
    {"0X"},
    {"###|RTE_ETH_RX_OFFLOAD_VLAN_STRIP"},
    {"RTE_ETH_RX_OFFLOAD_VLAN_EXTEND____"},
    {"RTE_ETH_RX_OFFLOAD_VLAN_EXTENDRTE_ETH_RX_OFFLOAD_MACSEC_STRIP|RTE_ETH_RX_OFFLOAD_VLAN_STRIP"}};

BOOST_DATA_TEST_CASE(parse_negative_test, boost::unit_test::data::make(broken_flags_tuples), data)
{
   BOOST_CHECK_THROW(utils::flags::from_string<std::uint64_t>(data, strings), std::runtime_error);
}

BOOST_DATA_TEST_CASE(x3_parser_negative_test, boost::unit_test::data::make(broken_flags_tuples), input)
{
   auto begin = std::begin(input);
   namespace x3 = boost::spirit::x3;
   unsigned int output = 0xabcd;
   BOOST_CHECK(!x3::phrase_parse(begin, end(input), parser::make_flags_parser<decltype(output)>(strings), x3::eol /* parser::skipper()*/, output) || begin != end(input));
}

BOOST_DATA_TEST_CASE(x3_parser_error_handler_negative_test, boost::unit_test::data::make(broken_flags_tuples), input)
{
   auto begin = std::begin(input);
   namespace x3 = boost::spirit::x3;
   unsigned int output = 0xabcd;
   BOOST_CHECK(!x3::phrase_parse(begin, end(input), parser::make_flags_parser<decltype(output)>(strings), x3::eol /* parser::skipper()*/, output) || begin != end(input));
}

BOOST_AUTO_TEST_SUITE_END()
