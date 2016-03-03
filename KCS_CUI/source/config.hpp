#pragma once
#include <iostream>
#include <memory>
// 設定クラス
class Config {
	struct Impl;
	std::unique_ptr<Impl> pimpl;
	//vector<string> input_filename_;	//入力ファイル名
	//vector<Formation> formation_;		//陣形指定
	//int times_;		//試行回数
	//int threads_;	//スレッド数
	//string output_filename_;	//出力ファイル名
	//bool json_prettify_flg_;	//出力ファイルを整形するか
public:
	// コンストラクタ
	Config();
	Config(int argc, char *argv[]);
	~Config();
	Config(const Config&) = delete;
	Config(Config&&);
	Config& operator=(const Config&) = delete;
	Config& operator=(Config&&);
	// 中身を表示する
	void Put() const;
	// getter
	const string& GetInputFilename(const int n) const noexcept;
	std::wstring GetInputFilenameW(const int n) const;
	Formation GetFormation(const int n) const noexcept;
	int GetTimes() const noexcept;
	int GetThreads() const noexcept;
	const string& GetOutputFilename() noexcept;
	bool GetJsonPrettifyFlg() const noexcept;
	//
	friend std::ostream& operator<<(std::ostream& os, const Config& conf);
	friend std::wostream& operator<<(std::wostream& os, const Config& conf);
};
std::ostream& operator<<(std::ostream& os, const Config& conf);
std::wostream& operator<<(std::wostream& os, const Config& conf);
