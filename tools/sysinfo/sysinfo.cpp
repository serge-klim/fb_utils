#include "utils/cpu_info.hpp"
#include "utils/cpu_clock.hpp"
#include "utils/sysinfo.hpp"
#include <boost/program_options.hpp>
#include <iostream>

void run(boost::program_options::variables_map const& /*config*/)
{
   auto freq = utils::estimate_cpu_frequency();
   std::cout << "cpu: " << utils::cpu_id_string() << '\n'
             << "estimated frequency: " << freq / 1000000 << " MHz (" << freq / 1000 << " KHz)"
             << std::endl;
}
