#pragma once
#include <boost/program_options/parsers.hpp>
#include <string>

namespace utils { inline namespace v1 {

boost::program_options::basic_parsed_options<char> parse_ini(std::string const& filename, boost::program_options::options_description const& description, bool allow_unregistered);


}} // namespace utils::v1::ini
