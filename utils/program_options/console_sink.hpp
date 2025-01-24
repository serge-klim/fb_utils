#pragma once
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/log/sinks/sink.hpp>

namespace utils { inline namespace v1 {

//void make_console_sink();

boost::shared_ptr<boost::log::sinks::sink> console_sink();
void register_console_sink_factory();

}} // namespace utils::v1

