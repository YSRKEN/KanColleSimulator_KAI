﻿#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "../../version.h"
#include "base.hpp"
#include "config.hpp"
#include <codecvt>
#include "char_convert.hpp"
#include <unordered_map>
#include <functional>
#include <array>
#include <algorithm>
#include "exception.hpp"
#include <iostream>
#include "../../version.h"

struct ForConfigImpl {
	ForConfigImpl() : input_filename_(), formation_({{ kFormationTrail , kFormationTrail }}), times_(1), threads_(1), json_prettify_flg_(true) {}
	std::array<string, kBattleSize> input_filename_;	//入力ファイル名
	std::array<Formation, kBattleSize> formation_;		//陣形指定
	size_t times_;		//試行回数
	size_t threads_;	//スレッド数
	string output_filename_;	//出力ファイル名
	bool json_prettify_flg_;	//出力ファイルを整形するか
};
struct Config::Impl {
	ForConfigImpl e;
};
namespace detail {
	void print_verison() noexcept(false)
	{
		using std::endl;
		std::cout
			<< KCS_FILE_DESCRIPTION << endl
			<< KCS_COPYRIGHT_STR << endl
			<< "version " KCS_VERSION_STR << endl;
	}
	void print_commandline_help() noexcept(false)
	{
		using std::endl;
		detail::print_verison();
		std::cout 
			<< endl
			<< "Usage: KCS_CUI -i input1.json input2.json|input2.map [-f formation1 formation2]" << endl
			<< "        [-n times] [-t threads] [-o output.json] [--result-json-prettify | --no-result-json-prettify]" << endl
			<< endl
			<< "-i input1.json input2.json|input2.map  : input file path" << endl
			<< "-f formation1 formation2               : fleet formation(0-5)" << endl
			<< "-n times                               : number of trials" << endl
			<< "-t threads                             : using threads" << endl
			<< "-o output.json                         : output file path" << endl
			<< "--result-json-prettify                 : prettify result json" << endl
			<< "--no-result-json-prettify              : no prettify result json" << endl
			<< endl;
	}
}
void print_commandline_help() noexcept(false)
{
	detail::print_commandline_help();
	SUCCESSFUL_TERMINATION_THROW_WITH_MESSAGE("");
}

void print_verison() noexcept(false)
{
	detail::print_verison();
	SUCCESSFUL_TERMINATION_THROW_WITH_MESSAGE("");
}


namespace detail {
	ForConfigImpl commandline_analyzer(int argc, char* argv[]) noexcept(false) {
		//CONFIG_THROW_WITH_MESSAGE_IF(argc < 4, "引数の数が足りていません.")
		using std::unordered_map;
		using std::function;
		ForConfigImpl re = {};
		unordered_map<string, function<void(const char*, const char*)>> case_two_arg = {
			{ "-i", [&re](const char* arg1, const char* arg2) {
				re.input_filename_[0] = arg1;
				re.input_filename_[1] = arg2;
			}},
			{ "-f", [&re](const char* arg1, const char* arg2) {
				re.formation_[0] = static_cast<Formation>(arg1 | to_i());
				re.formation_[1] = static_cast<Formation>(arg2 | to_i());
			}}
		};
		unordered_map<string, function<void(const char*)>> case_one_arg = {
			{ "-n", [&re](const char*arg) {
				re.times_ = std::max<size_t>(1, arg | to_sz());
			}},
			{ "-t", [&re](const char*arg) {
				re.threads_ = std::max<size_t>(1, arg | to_sz());
			}},
			{ "-o", [&re](const char*arg) {
				re.output_filename_ = arg;
			}}
		};
		unordered_map<string, function<void()>> case_no_arg = {
			{ "--no-result-json-prettify", [&re]() {
				re.json_prettify_flg_ = false;
			}},
			{ "--result-json-prettify", [&re]() {
				re.json_prettify_flg_ = true;
			} }
		};
		unordered_map<string, function<void()>> case_exist = {
			{ "-h", []() {
				detail::print_commandline_help();
				SUCCESSFUL_TERMINATION_THROW_WITH_MESSAGE("");
			} },
			{ "--help", []() {
				detail::print_commandline_help();
				SUCCESSFUL_TERMINATION_THROW_WITH_MESSAGE("");
			} },
			{ "-v", []() {
				detail::print_verison();
				SUCCESSFUL_TERMINATION_THROW_WITH_MESSAGE("");
			} },
			{ "--version", []() {
				detail::print_verison();
				SUCCESSFUL_TERMINATION_THROW_WITH_MESSAGE("");
			} }
		};
		for (int i = 1; i < argc; ++i) {
			if (case_two_arg.count(argv[i])) {
				INVAID_ARGUMENT_THROW_WITH_MESSAGE_IF(i + 2 >= argc, string("Incorrect usage :") + argv[i])
				INVAID_ARGUMENT_THROW_WITH_MESSAGE_IF('-' == argv[i + 1][0] || '-' == argv[i + 2][0], string("Incorrect usage :") + argv[i])
				case_two_arg[argv[i]](argv[i + 1], argv[i + 2]);
				i += 2;
			}
			else if (case_one_arg.count(argv[i])) {
				INVAID_ARGUMENT_THROW_WITH_MESSAGE_IF(i + 1 >= argc, string("Incorrect usage :") + argv[i])
				INVAID_ARGUMENT_THROW_WITH_MESSAGE_IF('-' == argv[i + 1][0], string("Incorrect usage :") + argv[i])
				case_one_arg[argv[i]](argv[i + 1]);
				++i;
			}
			else if (case_no_arg.count(argv[i])) case_no_arg[argv[i]]();
			else if (case_exist.count(argv[i])) case_exist[argv[i]]();//throw successful_termination
			else {
				detail::print_commandline_help();
				INVAID_ARGUMENT_THROW_WITH_MESSAGE(string("unknown option : ") + argv[i]);
			}
		}
		CONFIG_THROW_WITH_MESSAGE_IF(argc < 4, "引数の数が足りていません.")
		return re;
	}
}

// コンストラクタ
Config::Config() : pimpl(new Impl()){}

// コンストラクタ
Config::Config(int argc, char *argv[]) : pimpl(new Impl({ detail::commandline_analyzer(argc, argv) })) {
	// 入力ファイル名は必須であることに注意する
	CONFIG_THROW_WITH_MESSAGE_IF(this->pimpl->e.input_filename_[0].empty() || this->pimpl->e.input_filename_[1].empty(), "入力ファイル名は必ず指定してください." )
}

Config::~Config() = default;

Config::Config(Config && o) noexcept : pimpl(std::move(o.pimpl)) {}

Config & Config::operator=(Config && o) noexcept
{
	this->pimpl = std::move(o.pimpl);
	return *this;
}

// 中身を表示する
void Config::Put() const{
	cout << *this << endl;
}


// getter

const string & Config::GetInputFilename(const size_t n) const noexcept { return this->pimpl->e.input_filename_[n]; }

std::wstring Config::GetInputFilenameW(const int n) const
{
	return char_cvt::string2wstring(this->pimpl->e.input_filename_[n]);
}

Formation Config::GetFormation(const size_t n) const noexcept { return this->pimpl->e.formation_[n]; }

size_t Config::GetTimes() const noexcept { return this->pimpl->e.times_; }

size_t Config::GetThreads() const noexcept { return this->pimpl->e.threads_; }

const string & Config::GetOutputFilename() const noexcept { return this->pimpl->e.output_filename_; }

bool Config::GetJsonPrettifyFlg() const noexcept { return this->pimpl->e.json_prettify_flg_; }

std::ostream & operator<<(std::ostream & os, const Config & conf)
{
	os 
		<< "入力ファイル名：" << conf.GetInputFilename(0) << ", " << conf.GetInputFilename(1) << endl
		<< "陣形指定：" << endl;
	for (auto &it : conf.pimpl->e.formation_) {
		os << "　" << char_cvt::wstring2string(kFormationStr[it]) << endl;
	}
	os
		<< "試行回数：" << conf.pimpl->e.times_ << endl
		<< "スレッド数：" << conf.pimpl->e.threads_ << endl
		<< "出力ファイル名：\n　" << (conf.pimpl->e.output_filename_ != "" ? conf.pimpl->e.output_filename_ : "<なし>") << endl;
	return os;
}

std::wostream & operator<<(std::wostream & os, const Config & conf)
{
	os 
		<< L"入力ファイル名：" << conf.GetInputFilenameW(0) << L", " << conf.GetInputFilenameW(1) << endl
		<< L"陣形指定：" << endl;
	for (auto &it : conf.pimpl->e.formation_) {
		os << L"　" << kFormationStr[it] << endl;
	}
	os
		<< L"試行回数：" << conf.pimpl->e.times_ << endl
		<< L"スレッド数：" << conf.pimpl->e.threads_ << endl
		<< L"出力ファイル名：" << endl
		<< L"　" << (conf.pimpl->e.output_filename_ != "" ? char_cvt::string2wstring(conf.pimpl->e.output_filename_) : L"<なし>") << endl;
	return os;
}
