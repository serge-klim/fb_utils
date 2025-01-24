#pragma once
#include "rte_log.h"
#include <format>
#include <vector>
#include <iterator>
#include <cstdint>



namespace rtexx {

enum class log_level : std::uint32_t {
	emergancy = RTE_LOG_EMERG, 
	alert     = RTE_LOG_ALERT,
	critical  = RTE_LOG_CRIT,
	error     = RTE_LOG_ERR,
	warning   = RTE_LOG_WARNING,
	notice    = RTE_LOG_NOTICE,
	info      = RTE_LOG_INFO,
	debug     = RTE_LOG_DEBUG
};

struct null_logger
{
	null_logger() = default;
	template <typename... Args>
	constexpr null_logger(Args&&...) noexcept {}
	template <typename... Args>
	constexpr void log(Args&&...) const noexcept {}
	template <typename... Args>
	constexpr void trace(Args&&...) const noexcept {}
	template <typename... Args>
	constexpr void debug(Args&&...) const noexcept {}
	template <typename... Args>
	constexpr void info(Args&&...) const noexcept {}
	template <typename... Args>
	constexpr void warn(Args&&...) const noexcept {}
	template <typename... Args>
	constexpr void error(Args&&...) const noexcept {}
	template <typename... Args>
	constexpr void critical(Args&&...) const noexcept {}

	//void set_level(level level);
	//// return the active log level
	//[[nodiscard]] level log_level() const;
	//// return the name of the logger
	//[[nodiscard]] const std::string& name() const;
};


//RTE_LOG_REGISTER(hash_logtype_test, test.hash, INFO);

template<typename Format, typename ...Args>
void log(std::uint32_t logtype, log_level level, Format&& fmt,  Args&& ...args) {

	if (!rte_log_can_log(logtype, static_cast<std::uint32_t>(level)))
		return;

	auto out = rte_log_get_stream();
	///* save loglevel and logtype in a global per-lcore variable */
	//RTE_PER_LCORE(log_cur_msg).loglevel = level;
	//RTE_PER_LCORE(log_cur_msg).logtype = logtype;

	static thread_local auto buffer = [] {
		auto res = std::vector<char>{};
		res.reserve(512);
		return res;
		}();
	buffer.clear();
	try {
		std::vformat_to(std::back_inserter(buffer), std::forward<Format>(fmt), std::make_format_args(std::forward<Args>(args)...)) = '\n';
	}catch(std::format_error const& error) {
		auto msg = std::string{ "unable format log entry : " } + error.what();
		buffer.assign(begin(msg), end(msg));
	}
	//buffer.push_back('\0');
	//rte_log(static_cast<std::uint32_t>(level), logtype, buffer.data());
	fwrite(buffer.data(), sizeof(char), buffer.size(), out);
	//fflush(out); // not sure it make sense at the point
}

class rte_logger
{
public:
	constexpr rte_logger(int logtype) noexcept : logtype_{ logtype } {}
	rte_logger(const char* name, std::uint32_t loglevel = RTE_LOG_INFO);

	template<typename ...Args>
	void log(log_level level, Args&& ...args) { rtexx::log(logtype_, level, std::forward<Args>(args)...); }
	template <typename... Args>
	 void trace(Args&&...args)  { log(log_level::debug, std::forward<Args>(args)...); }
	template <typename... Args>
	 void debug(Args&&...args)  { log(log_level::debug, std::forward<Args>(args)...); }
	template <typename... Args>
	void info(Args&&...args)  { log(log_level::info, std::forward<Args>(args)...); }
	template <typename... Args>
	 void warn(Args&&...args)  { log(log_level::warning, std::forward<Args>(args)...); }
	template <typename... Args>
	 void error(Args&&...args)  { log(log_level::error, std::forward<Args>(args)...); }
	template <typename... Args>
	 void critical(Args&&...args)  { log(log_level::critical, std::forward<Args>(args)...); }


	template <typename... Args>
	 void alert(Args&&...args)  { log(log_level::alert, std::forward<Args>(args)...); }
	template <typename... Args>
	 void emergency(Args&&...args)  { log(log_level::emergancy, std::forward<Args>(args)...); }
private:
	int logtype_;
};

template<typename>
struct logger_type_traits {
	using type = rte_logger;
};

template<typename T>
using logger = typename logger_type_traits<T>::type;

template<typename T>
auto make_logger(const char* name, std::uint32_t loglevel = RTE_LOG_INFO){
	return logger<T>{ name, loglevel };
}

template<typename T>
auto make_logger(std::string const& name, std::uint32_t loglevel = RTE_LOG_INFO) {
	return logger<T>{ name.c_str(), loglevel };
}


} // namespace rtex

