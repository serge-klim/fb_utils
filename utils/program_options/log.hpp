#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/program_options.hpp>
#include <vector>
#include <string>


//namespace utils { inline namespace v1 {
// returns true if options contains log configuration
bool init_log_from_unrecognized_program_options(std::vector<std::string> const& /*unrecognized*/);
bool init_log_from_unrecognized_program_options(boost::program_options::basic_parsed_options<char> const& parsed_options, boost::program_options::variables_map& options_map);
//}} // namespace utils::v1
