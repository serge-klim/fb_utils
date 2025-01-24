#include "console_sink.hpp"
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/from_settings.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <iostream>



namespace {

struct console_sink_factory : boost::log::sink_factory<char>
{
   using backend_t = boost::log::sinks::basic_text_ostream_backend<char>;
   using sync_t = boost::log::sinks::asynchronous_sink<backend_t>;

   boost::shared_ptr<boost::log::sinks::sink> create_sink(settings_section const& params) override
   {
      auto backend = boost::make_shared<backend_t>();
      backend->add_stream(boost::shared_ptr<typename backend_t::stream_type>(&std::clog, boost::null_deleter()));

      //// Auto newline mode
      // if (auto auto_newline_param = params["AutoNewline"])
      //     backend->set_auto_newline_mode(param_cast_to_auto_newline_mode("AutoNewline", auto_newline_param.get()));

      //// Auto flush
      // if (auto auto_flush_param = params["AutoFlush"])
      //     backend->auto_flush(param_cast_to_bool("AutoFlush", auto_flush_param.get()));

      // Filter
      boost::log::filter filt;
      if (auto const& filter_param = params["Filter"]) {
         filt = boost::log::parse_filter(*filter_param.get());
      }

      //bool async = false;
      //if (optional<string_type> async_param = params["Asynchronous"]) {
      //   async = param_cast_to_bool("Asynchronous", async_param.get());
      //}

      auto sink = boost::make_shared<boost::log::sinks::asynchronous_sink<backend_t>>(backend);
      //auto sink = console_sink();
      if (auto const& format_param = params["Format"]) {
         // auto format_str = std::string{};
         // boost::log::aux::code_convert(format_param.get(), format_str);
         sink->set_formatter(boost::log::parse_formatter(/*format_str*/ *format_param.get()));
      }
      sink->set_filter(filt);
      console_sink_ = sink;
      return sink;
   }
   boost::shared_ptr<sync_t> console_sink() {
      return console_sink_.lock();
   }
 private:
   boost::weak_ptr<sync_t> console_sink_;
};

auto get_console_sink_factory() {
   static auto factory = boost::make_shared<console_sink_factory>();
   return factory;
}

}

void utils::v1::register_console_sink_factory() {
   boost::log::register_sink_factory("Console", get_console_sink_factory());
}

boost::shared_ptr<boost::log::sinks::sink> utils::v1::console_sink() {
   return get_console_sink_factory()->console_sink();
   //   return boost::log::add_console_log(std::clog, boost::log::keywords::format = "%TimeStamp%: %Message% <<<<<<<<<");
}

