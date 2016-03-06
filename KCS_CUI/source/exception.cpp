#include "exception.hpp"
namespace KCS_except {
	successful_termination::successful_termination(const char * sorce_name, const char * func_name, uint64_t line, const std::string & msg)
		: hold_(
			std::string("excetopn : successful_termination\n")
			+ "  in " + sorce_name + "\n"
			+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
			+ ((msg.empty() || msg[0] == '\0') ? "\n" : "\n  MESSAGE : " + msg + "\n")
			)
	{}
	invalid_argument::invalid_argument(const char * sorce_name, const char * func_name, uint64_t line, const char * expression, const std::string & msg)
		: std::invalid_argument(
			std::string("excetopn : invalid_argument\n")
			+ "  in " + sorce_name + "\n"
			+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
			+ "  follow by below\n"
			+ "    " + expression
			+ ((msg.empty() || msg[0] == '\0') ? "\n" : "\n  MESSAGE : " + msg + "\n")
			)
	{}

	invalid_argument::invalid_argument(const char * sorce_name, const char * func_name, uint64_t line, const std::string & msg)
		: std::invalid_argument(
			std::string("excetopn : invalid_argument\n")
			+ " in " + sorce_name + "\n"
			+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
			+ ((msg.empty() || msg[0] == '\0') ? "\n" : " MESSAGE : " + msg + "\n")
			)
	{}
	runtime_error::runtime_error(const char * sorce_name, const char * func_name, uint64_t line, const char * expression, const std::string & msg)
		: std::runtime_error(
			std::string("excetopn : runtime_error\n")
			+ "  in " + sorce_name + "\n"
			+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
			+ "  follow by below\n"
			+ "    " + expression
			+ ((msg.empty() || msg[0] == '\0') ? "\n" : "\n  MESSAGE : " + msg + "\n")
			)
	{}

	runtime_error::runtime_error(const char * sorce_name, const char * func_name, uint64_t line, const std::string & msg)
		: std::runtime_error(
			std::string("excetopn : runtime_error\n")
			+ " in " + sorce_name + "\n"
			+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
			+ ((msg.empty() || msg[0] == '\0') ? "\n" : " MESSAGE : " + msg + "\n")
			)
	{}

	config_error::config_error(const char * sorce_name, const char * func_name, uint64_t line, const char * expression, const std::string & msg)
		: std::invalid_argument(
			std::string("excetopn : config_error\n")
			+ "  in " + sorce_name + "\n"
			+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
			+ "  follow by below\n"
			+ "    " + expression
			+ ((msg.empty() || msg[0] == '\0') ? "\n" : "\n  MESSAGE : " + msg + "\n")
			)
	{}

	inline config_error::config_error(const char * sorce_name, const char * func_name, uint64_t line, const std::string & msg)
		: std::invalid_argument(
			std::string("excetopn : config_error\n")
			+ " in " + sorce_name + "\n"
			+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
			+ ((msg.empty() || msg[0] == '\0') ? "\n" : " MESSAGE : " + msg + "\n")
			)
	{}

	missing_rand_generator::missing_rand_generator(const char * sorce_name, const char * func_name, uint64_t line, const char * expression, const std::string & msg)
		: std::runtime_error(
			std::string("excetopn : missing_rand_generator\n")
			+ "  in " + sorce_name + "\n"
			+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
			+ "  follow by below\n"
			+ "    " + expression
			+ ((msg.empty() || msg[0] == '\0') ? "\n" : "\n  MESSAGE : " + msg + "\n")
			)
	{}

	inline missing_rand_generator::missing_rand_generator(const char * sorce_name, const char * func_name, uint64_t line, const std::string & msg)
		: std::runtime_error(
			std::string("excetopn : missing_rand_generator\n")
			+ " in " + sorce_name + "\n"
			+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
			+ ((msg.empty() || msg[0] == '\0') ? "\n" : " MESSAGE : " + msg + "\n")
			)
	{}

	file_error::file_error(const char * sorce_name, const char * func_name, uint64_t line, const char * expression, const std::string & msg)
		: std::runtime_error(
			std::string("excetopn : file_error\n")
			+ "  in " + sorce_name + "\n"
			+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
			+ "  follow by below\n"
			+ "    " + expression
			+ ((msg.empty() || msg[0] == '\0') ? "\n" : "\n  MESSAGE : " + msg + "\n")
			)
	{}

	file_error::file_error(const char * sorce_name, const char * func_name, uint64_t line, const std::string & msg)
		: std::runtime_error(
			std::string("excetopn : file_error\n")
			+ " in " + sorce_name + "\n"
			+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
			+ ((msg.empty() || msg[0] == '\0') ? "\n" : " MESSAGE : " + msg + "\n")
			)
	{}


	encode_error::encode_error(const char * sorce_name, const char * func_name, uint64_t line, const char * expression, const std::string & msg)
		: std::runtime_error(
			std::string("excetopn : encode_error\n")
			+ "  in " + sorce_name + "\n"
			+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
			+ "  follow by below\n"
			+ "    " + expression
			+ ((msg.empty() || msg[0] == '\0') ? "\n" : "\n  MESSAGE : " + msg + "\n")
			)
	{}

	encode_error::encode_error(const char * sorce_name, const char * func_name, uint64_t line, const std::string & msg)
		: std::runtime_error(
			std::string("excetopn : encode_error\n")
			+ " in " + sorce_name + "\n"
			+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
			+ ((msg.empty() || msg[0] == '\0') ? "\n" : " MESSAGE : " + msg + "\n")
			)
	{}
}

