#include "ini_parser.hpp"
#include "utils/ini_parser/ini_parser.hpp"
#include <unordered_set>
#include <string>
#include <ranges>
#include <algorithm>


boost::program_options::basic_parsed_options<char> utils::v1::parse_ini(std::string const& filename,
                                                                        boost::program_options::options_description const& description,
                                                                        bool allow_unregistered) {
   auto allowed_options = std::unordered_set<std::string>{};

   auto const& options = description.options();
   for (unsigned i = 0; i < options.size(); ++i) {
      auto const& description = *options[i];
      if (description.long_name().empty())
         boost::throw_exception(
             boost::program_options::error("abbreviated option names are not permitted in options configuration files"));

      allowed_options.insert(description.long_name());
   }

   boost::program_options::parsed_options result{&description};
   auto value = utils::ini::parse(filename);
   std::ranges::copy(value | std::views::transform([&allowed_options, allow_unregistered](auto const& pair) {
         auto registered = allowed_options.contains(pair.first);
         if (!registered && !allow_unregistered)
            boost::throw_exception(boost::program_options::unknown_option(pair.first));
         auto res = boost::program_options::basic_option<char>{pair.first, {pair.second}};
         res.unregistered = !registered; 
         res.original_tokens = { pair.first, pair.second};
         //if (!pair.second.empty())
         //   res.original_tokens.emplace_back(pair.second);
         return res;
       }), std::back_inserter(result.options));
   return {std::move(result)};
}
