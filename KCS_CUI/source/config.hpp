#ifndef KCS_KAI_INC_CONFIG_HPP_
#define KCS_KAI_INC_CONFIG_HPP_

#include <iostream>
#include <memory>
[[noreturn]] void print_commandline_help() noexcept(false);
[[noreturn]] void print_verison() noexcept(false);
// 設定クラス
class Config {
	struct Impl;
	std::unique_ptr<Impl> pimpl;
public:
	// コンストラクタ
	Config();
	Config(int argc, char *argv[]);
	~Config();
	Config(const Config&) = delete;
	Config(Config&&) noexcept;
	Config& operator=(const Config&) = delete;
	Config& operator=(Config&&) noexcept;
	// 中身を表示する
	void Put() const;
	// getter
	const string& GetInputFilename(const size_t n) const noexcept;
	std::wstring GetInputFilenameW(const int n) const;
	Formation GetFormation(const size_t n) const noexcept;
	size_t GetTimes() const noexcept;
	size_t GetThreads() const noexcept;
	const string& GetOutputFilename() noexcept;
	bool GetJsonPrettifyFlg() const noexcept;
	//
	friend std::ostream& operator<<(std::ostream& os, const Config& conf);
	friend std::wostream& operator<<(std::wostream& os, const Config& conf);
};
std::ostream& operator<<(std::ostream& os, const Config& conf);
std::wostream& operator<<(std::wostream& os, const Config& conf);

#endif //KCS_KAI_INC_CONFIG_HPP_
