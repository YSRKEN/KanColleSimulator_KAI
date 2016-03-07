#pragma once
#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <string>
#define SUCCESSFUL_TERMINATION_THROW_WITH_MESSAGE( MESSAGE ) throw KCS_except::successful_termination(__FILE__, __FUNCTION__, __LINE__, MESSAGE);
#define INVAID_ARGUMENT_THROW_WITH_MESSAGE( MESSAGE ) throw KCS_except::invalid_argument(__FILE__, __FUNCTION__, __LINE__, MESSAGE);
#define INVAID_ARGUMENT_THROW_WITH_MESSAGE_IF( EXPR, MESSAGE ) if( EXPR ){ throw KCS_except::invalid_argument(__FILE__, __FUNCTION__, __LINE__, #EXPR, MESSAGE); }
#define RUNTIME_ERROR_THROW_WITH_MESSAGE( MESSAGE ) throw KCS_except::runtime_error(__FILE__, __FUNCTION__, __LINE__, MESSAGE);
#define RUNTIME_ERROR_THROW_WITH_MESSAGE_IF( EXPR, MESSAGE ) if( EXPR ){ throw KCS_except::runtime_error(__FILE__, __FUNCTION__, __LINE__, #EXPR, MESSAGE); }
#define CONFIG_THROW_WITH_MESSAGE( MESSAGE ) throw KCS_except::config_error(__FILE__, __FUNCTION__, __LINE__, MESSAGE);
#define CONFIG_THROW_WITH_MESSAGE_IF( EXPR, MESSAGE ) if( EXPR ){ throw KCS_except::config_error(__FILE__, __FUNCTION__, __LINE__, #EXPR, MESSAGE); }
#define MISSING_RAND_GENERATOR_THROW_WITH_MESSAGE( MESSAGE ) throw KCS_except::missing_rand_generator(__FILE__, __FUNCTION__, __LINE__, MESSAGE);
#define MISSING_RAND_GENERATOR_THROW_WITH_MESSAGE_IF( EXPR, MESSAGE ) if( EXPR ){ throw KCS_except::missing_rand_generator(__FILE__, __FUNCTION__, __LINE__, #EXPR, MESSAGE); }
#define FILE_THROW_WITH_MESSAGE( MESSAGE ) throw KCS_except::file_error(__FILE__, __FUNCTION__, __LINE__, MESSAGE);
#define FILE_THROW_WITH_MESSAGE_IF( EXPR, MESSAGE ) if( EXPR ){ throw KCS_except::file_error(__FILE__, __FUNCTION__, __LINE__, #EXPR, MESSAGE); }
#define ENCODE_THROW_WITH_MESSAGE( MESSAGE ) throw KCS_except::encode_error(__FILE__, __FUNCTION__, __LINE__, MESSAGE);
#define ENCODE_THROW_WITH_MESSAGE_IF( EXPR, MESSAGE ) if( EXPR ){ throw KCS_except::encode_error(__FILE__, __FUNCTION__, __LINE__, #EXPR, MESSAGE); }

namespace KCS_except {
	class successful_termination : public std::exception {
		std::string hold_;
	public:
		successful_termination(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg);
		virtual char const* what() const noexcept override {
			return hold_.c_str();
		}
	};
	class invalid_argument : public std::invalid_argument
	{
	public:
		invalid_argument(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg);
		invalid_argument(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg);
	};
	class config_error : public std::invalid_argument
	{
	public:
		config_error(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg);
		config_error(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg);
	};
	class runtime_error : public std::runtime_error {
	public:
		runtime_error(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg);
		runtime_error(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg);
	};
	class missing_rand_generator : public std::runtime_error
	{
	public:
		missing_rand_generator(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg);
		missing_rand_generator(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg);
	};
	class file_error : public std::runtime_error
	{
	public:
		file_error(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg);
		file_error(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg);
	};
	class encode_error : public std::runtime_error
	{
	public:
		encode_error(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg);
		encode_error(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg);
	};

}
