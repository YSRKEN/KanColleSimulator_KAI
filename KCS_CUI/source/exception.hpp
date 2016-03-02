#pragma once
#include <iostream>
#include <exceprion>
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
	class kcs_except : public std::exception
	{
	public:
		invalid_argument(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg) noexcept
			: msg(
				std::string("excetopn : invalid_argument\n")
				+ "  in " + sorce_name + "\n"
				+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
				+ "  follow by below\n"
				+ "    " + expression
				+ ((msg.empty() || msg[0] == '\0') ? "\n" : "\n  MESSAGE : " + msg + "\n")
			)
		{}
		invalid_argument(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg) noexcept
			: msg(
				std::string("excetopn : invalid_argument\n")
				+ " in " + sorce_name +  "\n"
				+ "  " + func_name + "() (line." + std::to_string(line) + ")\n"
				+ ((msg.empty() || msg[0] == '\0') ? "\n" : " MESSAGE : " + msg + "\n")
			)
		{}
        const char* what() const noexcept {
			return msg.c_str();
		}
    private:
        std::string msg;
    };
	class config_error : public kcs_except
	{
	public:
        config_error(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg) noexcept
            : kcs_except(sorce_name, func_name, line, expression, msg) {}
        config_error(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg) noexcept
            : kcs_except(sorce_name, func_name, line, msg) {}
	};
	class missing_rand_generator : public kcs_except
	{
	public:
        missing_rand_generator(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg) noexcept
            : kcs_except(sorce_name, func_name, line, expression, msg) {}
        missing_rand_generator(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg) noexcept
            : kcs_except(sorce_name, func_name, line, msg) {}

	};
	class file_error : public kcs_except
	{
	public:
        file_error(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg) noexcept
            : kcs_except(sorce_name, func_name, line, expression, msg) {}
        file_error(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg) noexcept
            : kcs_except(sorce_name, func_name, line, msg) {}
	};
	class encode_error : public kcs_except
	{
	public:
        encode_error(const char* sorce_name, const char* func_name, uint64_t line, const char* expression, const std::string& msg) noexcept
            : kcs_except(sorce_name, func_name, line, expression, msg) {}
        encode_error(const char* sorce_name, const char* func_name, uint64_t line, const std::string& msg) noexcept
            : kcs_except(sorce_name, func_name, line, msg) {}
	};

}
