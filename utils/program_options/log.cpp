#include "log.hpp"
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/log/utility/setup/from_settings.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <fstream>

namespace x3 = boost::spirit::x3;

// //boost::property_tree::ptree log_sinks_from_unrecognized(std::vector<boost::program_options::basic_option<char>> const& parsed_options)
// void init_log_from_unrecognized_program_options(std::vector<std::string> const& unrecognized)
// {
//    auto res = boost::property_tree::ptree{};
//    assert(unrecognized.size() % 2 == 0);
//    auto size = unrecognized.size() / 2;
//    for (auto i = 0; i != size; ++i) {
//       auto const& name = unrecognized[i * 2];
//       auto begin = cbegin(name);
//       auto end = cend(name);
//       //auto value = std::pair<boost::iterator_range<decltype(begin)>, boost::iterator_range<decltype(begin)>>{};
//       auto section_name = std::pair<std::string, std::string>{};
//       if (x3::parse(begin, end, x3::raw["Sinks." >> x3::uint_ ]>> '.' >>x3::raw[ +(x3::char_ - '.')], section_name) || begin == end)
//       //if (x3::parse(begin, end, "Sinks." >> x3::uint_ >> '.' >> +(x3::char_ - '.')/*, value*/ ) || begin == end)
//       {
//           auto ai = res.find(section_name.first);
//           auto ii = ai == res.not_found() ? res.push_back(std::make_pair(section_name.first, boost::property_tree::ptree{})) : res.to_iterator(ai);
//           ii->second.put(section_name.second, unrecognized[i*2+1]);

//           // std::clog << name << " : " << unrecognized[i*2+1] << std::endl;
//           // res.put(name, unrecognized[i*2+1]);
//       }
//    }
//     struct LogSectionWrapper : boost::log::basic_settings_section<char>
//     {
//         LogSectionWrapper(boost::property_tree::ptree* section) : boost::log::basic_settings_section<char>{ section } {}
//     };

//     //write_ini( "/mnt/c/Users/serge/Documents/projects/tools/project/scel/out.ini", res );
//     //SeverityFilterFactory::Register();
//     boost::log::init_from_settings(LogSectionWrapper{&res});
// }

// void init_log_from_unrecognized_program_options(std::vector<std::string> const& unrecognized)
// {
//    auto sections = std::unordered_map<std::string, boost::property_tree::ptree>{};
//    assert(unrecognized.size() % 2 == 0);
//    auto size = unrecognized.size() / 2;
//    for (auto i = 0; i != size; ++i) {
//       auto const& name = unrecognized[i * 2];
//       auto begin = cbegin(name);
//       auto end = cend(name);
//       //auto value = std::pair<boost::iterator_range<decltype(begin)>, boost::iterator_range<decltype(begin)>>{};
//       auto section_name = std::pair<std::string, std::string>{};
//       if (x3::parse(begin, end, x3::raw["Sinks." >> x3::uint_ ]>> '.' >>x3::raw[ +(x3::char_ - '.')], section_name) || begin == end)
//       //if (x3::parse(begin, end, "Sinks." >> x3::uint_ >> '.' >> +(x3::char_ - '.')/*, value*/ ) || begin == end)
//       {
//           // auto ai = res.find(section_name.first);
//           // auto ii = ai == res.not_found() ? res.push_back(std::make_pair(section_name.first, boost::property_tree::ptree{})) : res.to_iterator(ai);
//           // ii->second.put(section_name.second, unrecognized[i*2+1]);
//           auto isection = sections.find(section_name.first);
//           if(isection == std::end(sections))
//               isection = sections.emplace(std::move(section_name.first), boost::property_tree::ptree{}).first;
//           isection->second.put(section_name.second, unrecognized[i*2+1]);
//           // std::clog << name << " : " << unrecognized[i*2+1] << std::endl;
//           // res.put(name, unrecognized[i*2+1]);
//       }
//    }
//     struct LogSectionWrapper : boost::log::basic_settings_section<char>
//     {
//         LogSectionWrapper(boost::property_tree::ptree* section) : boost::log::basic_settings_section<char>{ section } {}
//     };

//     //write_ini( "/mnt/c/Users/serge/Documents/projects/tools/project/scel/out.ini", res );
//     //SeverityFilterFactory::Register();
//     for(auto& section : sections)
//         boost::log::init_from_settings(LogSectionWrapper{&section.second});
// }

bool init_log_from_unrecognized_program_options(std::vector<std::string> const& unrecognized) {
   auto has_log_settings = false;
   auto tree = boost::property_tree::ptree{};
   assert(unrecognized.size() % 2 == 0);
   auto size = unrecognized.size() / 2;
   for (auto i = decltype(size){0}; i != size; ++i) {
      auto const& name = unrecognized[i * 2];
      auto begin = cbegin(name);
      auto end = cend(name);
      // //auto value = std::pair<boost::iterator_range<decltype(begin)>, boost::iterator_range<decltype(begin)>>{};
      // auto section_name = std::pair<std::string, std::string>{};
      // if (x3::parse(begin, end, x3::raw["Sinks." >> x3::uint_ ]>> '.' >>x3::raw[ +(x3::char_ - '.')], section_name) || begin == end)
      if (boost::spirit::x3::parse(begin, end, (("Sinks." >> boost::spirit::x3::uint_ >> '.') | "Core.") >> +(boost::spirit::x3::char_ - '.') /*, value*/) && begin == end) {
         tree.put(name, unrecognized[i * 2 + 1]);
         has_log_settings = true;
      }
   }
   struct LogSectionWrapper : boost::log::basic_settings_section<char>
   {
      LogSectionWrapper(boost::property_tree::ptree* section) : boost::log::basic_settings_section<char>{section} {}
   };

   boost::log::init_from_settings(LogSectionWrapper{&tree});
   boost::log::add_common_attributes();
   return has_log_settings;
}


bool init_log_from_unrecognized_program_options(boost::program_options::basic_parsed_options<char> const& parsed_options, boost::program_options::variables_map& options_map) {
   auto has_log_settings = false;
   auto tree = boost::property_tree::ptree{};
   // extension of boost::program_options::collect_unrecognized(parsed.options, boost::program_options::include_positional)
   for (auto const& option : parsed_options.options) {
      if (option.unregistered /* ||
           (mode == include_positional && options[i].position_key != -1)
           */
      ) {
         if (option.original_tokens.size() == 2) {
            auto begin = cbegin(option.original_tokens[0]);
            auto end = cend(option.original_tokens[0]);
            if (boost::spirit::x3::parse(begin, end, (("Sinks." >> boost::spirit::x3::uint_ >> '.') | "Core.") >> +(boost::spirit::x3::char_ - '.') /*, value*/) && begin == end) {
               // if (boost::spirit::x3::parse(begin, end, "Sinks." >> boost::spirit::x3::uint_ >> '.' >> +(boost::spirit::x3::char_ - '.') /*, value*/) || begin == end)
               tree.put(option.original_tokens[0], option.original_tokens[1]);
               has_log_settings = true;
            } else
               options_map.emplace(option.original_tokens[0], boost::program_options::variable_value{boost::any{option.original_tokens[1]}, false});
         }
      }
   }
   struct LogSectionWrapper : boost::log::basic_settings_section<char>
   {
      LogSectionWrapper(boost::property_tree::ptree* section) : boost::log::basic_settings_section<char>{section} {}
   };

   boost::log::init_from_settings(LogSectionWrapper{&tree});
   boost::log::add_common_attributes();
   return has_log_settings;
}

