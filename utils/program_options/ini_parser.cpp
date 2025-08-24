#include "ini_parser.hpp"
#include "utils/ini_parser/ini_parser.hpp"
#include <unordered_set>
#include <set>
#include <string>
#include <string_view>
#include <ranges>
#include <algorithm>


boost::program_options::basic_parsed_options<char> utils::v1::parse_ini(std::string const& filename,
                                                                        boost::program_options::options_description const& description,
                                                                        bool allow_unregistered) {
   auto allowed_options = std::unordered_set<std::string>{};
   auto allowed_prefixes = std::set<std::string>{};

   auto const& options = description.options();
   for (unsigned i = 0; i < options.size(); ++i) {
      auto const& description = *options[i];
      if (description.long_name().empty())
         boost::throw_exception(
             boost::program_options::error("abbreviated option names are not permitted in options configuration files"));
      //stolen from boost
      auto const& long_name = description.long_name();
      allowed_options.insert(long_name);
      if (long_name.back() == '*') {
         auto prefix = long_name.substr(0, long_name.size() - 1);
         auto bad_prefixes = false;
         // If 's' is a prefix of one of allowed suffix, then
         // lower_bound will return that element.
         // If some element is prefix of 's', then lower_bound will
         // return the next element.
         auto i = allowed_prefixes.lower_bound(prefix);
         if (i != allowed_prefixes.end()) {
            if (i->find(prefix) == 0)
               bad_prefixes = true;
         }
         if (i != allowed_prefixes.begin()) {
            --i;
            if (prefix.find(*i) == 0)
               bad_prefixes = true;
         }
         if (bad_prefixes)
            boost::throw_exception(boost::program_options::error("options '" + long_name + "' and '" +
                                         *i + "*' will both match the same "
                                              "arguments from the configuration file"));
         allowed_prefixes.insert(std::move(prefix));
      }
   }

   boost::program_options::parsed_options result{&description};
   auto value = utils::ini::parse(filename);
   std::ranges::copy(value | std::views::transform([&allowed_options, &allowed_prefixes, allow_unregistered](auto const& pair) {
         auto registered = allowed_options.contains(pair.first);
         if (!registered) {
            // stolen from boost:
            // If s is "pa" where "p" is allowed prefix then
            // lower_bound should find the element after "p".
            // This depends on 'allowed_prefixes' invariant.
            auto i = allowed_prefixes.lower_bound(pair.first);
            registered = (i != allowed_prefixes.begin() && pair.first.find(*--i) == 0);
            if (!registered && !allow_unregistered)
                boost::throw_exception(boost::program_options::unknown_option(pair.first));
         }
         auto res = boost::program_options::basic_option<char>{pair.first, {pair.second}};
         res.unregistered = !registered; 
         res.original_tokens = { pair.first, pair.second};
         //if (!pair.second.empty())
         //   res.original_tokens.emplace_back(pair.second);
         return res;
       }), std::back_inserter(result.options));
   return {std::move(result)};
}
