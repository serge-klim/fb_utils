#include "utils/program_options/validators/chrono.hpp"
#include "utils/program_options/validators/net.hpp"
#include "utils/program_options/log.hpp"
#include <boost/format.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>

void setup_common_options(boost::program_options::options_description& description)
{
   // clang-format off
   description.add_options()
       ("numa-node,n", boost::program_options::value<unsigned int>(/*&params.source*/), "\tselect numa node")
       ("cpu-number,c", boost::program_options::value<std::string>(/*&params.source*/), "\tnumber of cpu to allocate")
       ;
   // clang-format on
}

bool parse_config(boost::program_options::variables_map& vm)
{
   auto res = false;
   auto config_filename = vm.count("config") == 0 ? std::make_pair(false, std::string{"config.ini"}) : std::make_pair(true, vm["config"].as<std::string>());
   if (auto ifs = std::ifstream{config_filename.second}) {       
      auto description = boost::program_options::options_description{"configuration"};     
      setup_common_options(description);

      boost::program_options::options_description config_file_options;
      config_file_options.add(description);
      auto parsed = parse_config_file(ifs, config_file_options, true);
      store(parsed, vm);

      //////////////////////////////////////
      std::clog << "parsed options:\n";
      for (auto const& o : parsed.options) {
         if (vm.find(o.string_key) == vm.end()) {
            std::clog << o.string_key << " = "; // o.value - std::vector<std::string>
            std::copy(begin(o.value), end(o.value), std::ostream_iterator<std::string>{std::clog, ","});
            std::clog << std::endl;
            // manually add to map!!!!!!!!!!
            //vm.emplace(o.string_key, boost::program_options::variable_value{boost::any{o.value}, false});
         }
      }
      std::clog << "\n\nunrecognized options:\n";
       auto /*std::vector<std::string>*/ additional = collect_unrecognized(parsed.options, boost::program_options::include_positional);
       for( auto a : additional)
           std::cout << a << std::endl;
      init_log_from_unrecognized_program_options(additional);

      ////////////////////////////////////

      //TODO: init boost::log::basic_settings_section<char> from unrecognized options
      //eg: e:/backup/Documents/projects/svchub/lib/config/log.cpp
      ////////////////////////////////////
      // ifs.clear();
      // if(!ifs.seekg(0/*, std::ios_base::beg*/))
      //     throw std::runtime_error{"unable to reset settings file position"};
      // void init_log(std::istream&);
      // init_log(ifs);     
      ////////////////////////////////////
      
      notify(vm); // check config file options sanity
      res = true;
   } else if (config_filename.first)
      throw std::runtime_error{str(boost::format("can't open configuration file \"%1%\"") % config_filename.second)};
   return res;
}


std::ostream& help(std::ostream& out, boost::program_options::options_description& description)
{
   // clang-format off
   out << description << "\neg:\n"
                         "app ...blah";
   // clang-format on
   return out;
}

//#include <boost/log/trivial.hpp>
//#include <boost/log/core.hpp>
//#include <boost/log/expressions.hpp>

int main(int argc, char* argv[])
{
   //boost::log::core::get()->set_filter(
   //    boost::log::trivial::severity >= boost::log::trivial::error);

   auto description = boost::program_options::options_description{"options"};
   try {
      // clang-format off
        description.add_options()
            ("help,h", "\tprint usage message")
            ("config,c", boost::program_options::value<std::string>())
            ;
      // clang-format on
      setup_common_options(description);
      //boost::program_options::positional_options_description positional;
      //positional.add("config", -1);

      boost::program_options::variables_map vm;
      store(boost::program_options::command_line_parser(
                argc, argv)
                .options(description) /*.positional(positional)*/.run(),
            vm);

      if (vm.count("help") != 0) {
         help(std::cout, description);
         return 0;
      }

      void run(boost::program_options::variables_map const& config);
      run(vm);
   } catch (boost::program_options::error& e) {
      std::cerr << "error : " << e.what() << "\n\n";
      help(std::cerr, description);
   } catch (std::exception& e) {
      std::cerr << "error : " << e.what() << std::endl;
   } catch (...) {
      std::cerr << "miserably failed:(" << std::endl;
   }

   return 0;
}
