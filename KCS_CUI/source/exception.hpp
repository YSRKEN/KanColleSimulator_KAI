#pragma once
#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <string>
#define INVAID_ARGUMENT_THROW_WITH_MESSAGE( MESSAGE ) throw KCS_except::invalid_argument(__FILE__, __FUNCTION__, __LINE__, MESSAGE);
#define INVAID_ARGUMENT_THROW_WITH_MESSAGE_IF( EXPR, MESSAGE ) if( EXPR ){ throw KCS_except::invalid_argument(__FILE__, __FUNCTION__, __LINE__, #EXPR, MESSAGE); }
#define CONFIG_THROW_WITH_MESSAGE( MESSAGE ) throw KCS_except::config_error(__FILE__, __FUNCTION__, __LINE__, MESSAGE);
#define CONFIG_THROW_WITH_MESSAGE_IF( EXPR, MESSAGE ) if( EXPR ){ throw KCS_except::config_error(__FILE__, __FUNCTION__, __LINE__, #EXPR, MESSAGE); }
#define MISSING_RAND_GENERATOR_THROW_WITH_MESSAGE( MESSAGE ) throw KCS_except::missing_rand_generator(__FILE__, __FUNCTION__, __LINE__, MESSAGE);
#define MISSING_RAND_GENERATOR_THROW_WITH_MESSAGE_IF( EXPR, MESSAGE ) if( EXPR ){ throw KCS_except::missing_rand_generator(__FILE__, __FUNCTION__, __LINE__, #EXPR, MESSAGE); }
#define FILE_THROW_WITH_MESSAGE( MESSAGE ) throw KCS_except::file_error(__FILE__, __FUNCTION__, __LINE__, MESSAGE);
#define FILE_THROW_WITH_MESSAGE_IF( EXPR, MESSAGE ) if( EXPR ){ throw KCS_except::file_error(__FILE__, __FUNCTION__, __LINE__, #EXPR, MESSAGE); }
#define ENCODE_THROW_WITH_MESSAGE( MESSAGE ) throw KCS_except::encode_error(__FILE__, __FUNCTION__, __LINE__, MESSAGE);
#define ENCODE_THROW_WITH_MESSAGE_IF( EXPR, MESSAGE ) if( EXPR ){ throw KCS_except::encode_error(__FILE__, __FUNCTION__, __LINE__, #EXPR, MESSAGE); }

namespace KCS_except {
	class invalid_argument : public std::invalid_argument
	{
	public:
		invalid_argument(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg)
			: std::invalid_argument(
				std::string("excetopn : invalid_argument\n") 
				+ "  in " + sorce_name + "\n" 
				+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
				+ "  follow by below\n"
				+ "    " + expression 
				+ ((msg.empty() || msg[0] == '\0') ? "\n" : "\n  MESSAGE : " + msg + "\n")
			)
		{}
		invalid_argument(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg)
			: std::invalid_argument(
				std::string("excetopn : invalid_argument\n")
				+ " in " + sorce_name +  "\n" 
				+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
				+ ((msg.empty() || msg[0] == '\0') ? "\n" : " MESSAGE : " + msg + "\n")
			)
		{}
	};
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
	class missing_rand_generator : public std::runtime_error
	{
	public:
		missing_rand_generator(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg)
			: std::runtime_error(
				std::string("excetopn : missing_rand_generator\n") 
				+ "  in " + sorce_name + "\n" 
				+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
				+ "  follow by below\n"
				+ "    " + expression 
				+ ((msg.empty() || msg[0] == '\0') ? "\n" : "\n  MESSAGE : " + msg + "\n")
			)
		{}
		missing_rand_generator(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg)
			: std::runtime_error(
				std::string("excetopn : missing_rand_generator\n")
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