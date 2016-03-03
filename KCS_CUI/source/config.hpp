#pragma once
#include <iostream>
// 設定クラス
class Config {
	vector<string> input_filename_;	//入力ファイル名
	vector<Formation> formation_;		//陣形指定
	int times_;		//試行回数
	int threads_;	//スレッド数
	string output_filename_;	//出力ファイル名
	bool json_prettify_flg_;	//出力ファイルを整形するか
public:
	// コンストラクタ
	Config() {}
	Config(int argc, char *argv[]);
	// 中身を表示する
	void Put() const;
	// getter
	const string& GetInputFilename(const int n) const noexcept { return input_filename_[n]; }
	std::wstring GetInputFilenameW(const int n) const;
	Formation GetFormation(const int n) const noexcept { return formation_[n]; }
	int GetTimes() const noexcept { return times_; }
	int GetThreads() const noexcept { return threads_; }
	const string& GetOutputFilename() noexcept { return output_filename_; }
	bool GetJsonPrettifyFlg() const noexcept { return json_prettify_flg_; }
	//
	friend std::ostream& operator<<(std::ostream& os, const Config& conf);
	friend std::wostream& operator<<(std::wostream& os, const Config& conf);
};
std::ostream& operator<<(std::ostream& os, const Config& conf);
std::wostream& operator<<(std::wostream& os, const Config& conf);
