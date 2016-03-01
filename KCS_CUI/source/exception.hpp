#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <string>
#define CONFIG_THROW_WITH_MESSAGE( MSG ) throw KCS_except::config_error(__FILE__, __FUNCTION__, __LINE__, MSG);
#define CONFIG_THROW_WITH_MESSAGE_IF( EXPR, MSG ) if( EXPR ){ throw KCS_except::config_error(__FILE__, __FUNCTION__, __LINE__, #EXPR, MSG); }
#define FILE_THROW_WITH_MESSAGE( MSG ) throw KCS_except::file_error(__FILE__, __FUNCTION__, __LINE__, MSG);
#define FILE_THROW_WITH_MESSAGE_IF( EXPR, MSG ) if( EXPR ){ throw KCS_except::file_error(__FILE__, __FUNCTION__, __LINE__, #EXPR, MSG); }
#define ENCODE_THROW_WITH_MESSAGE( MSG ) throw KCS_except::encode_error(__FILE__, __FUNCTION__, __LINE__, MSG);
#define ENCODE_THROW_WITH_MESSAGE_IF( EXPR, MSG ) if( EXPR ){ throw KCS_except::encode_error(__FILE__, __FUNCTION__, __LINE__, #EXPR, MSG); }

namespace KCS_except {
	class config_error : public std::invalid_argument
	{
	public:
		config_error(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg) 
			: std::invalid_argument(
				std::string("excetopn : config_error\n") 
				+ "  in " + sorce_name + "\n" 
				+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
				+ "  follow by below\n"
				+ "    " + expression 
				+ ((msg.empty() || msg[0] == '\0') ? "\n" : "\n  MESSAGE : " + msg + "\n")
			)
		{}
		config_error(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg)
			: std::invalid_argument(
				std::string("excetopn : config_error\n")
				+ " in " + sorce_name +  "\n" 
				+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
				+ ((msg.empty() || msg[0] == '\0') ? "\n" : " MESSAGE : " + msg + "\n")
			)
		{}
	};
	class file_error : public std::runtime_error
	{
	public:
		file_error(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg) 
			: std::runtime_error(
				std::string("excetopn : file_error\n") 
				+ "  in " + sorce_name + "\n" 
				+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
				+ "  follow by below\n"
				+ "    " + expression 
				+ ((msg.empty() || msg[0] == '\0') ? "\n" : "\n  MESSAGE : " + msg + "\n")
			)
		{}
		file_error(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg)
			: std::runtime_error(
				std::string("excetopn : file_error\n")
				+ " in " + sorce_name +  "\n" 
				+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
				+ ((msg.empty() || msg[0] == '\0') ? "\n" : " MESSAGE : " + msg + "\n")
			)
		{}
	};
	class encode_error : public std::runtime_error
	{
	public:
		encode_error(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg) 
			: std::runtime_error(
				std::string("excetopn : encode_error\n") 
				+ "  in " + sorce_name + "\n" 
				+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
				+ "  follow by below\n"
				+ "    " + expression 
				+ ((msg.empty() || msg[0] == '\0') ? "\n" : "\n  MESSAGE : " + msg + "\n")
			)
		{}
		encode_error(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg)
			: std::runtime_error(
				std::string("excetopn : encode_error\n")
				+ " in " + sorce_name +  "\n" 
				+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
				+ ((msg.empty() || msg[0] == '\0') ? "\n" : " MESSAGE : " + msg + "\n")
			)
		{}
	};

}