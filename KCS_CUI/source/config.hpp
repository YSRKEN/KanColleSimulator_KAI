#pragma once
#include <iostream>
// 設定クラス
class Config {
	vector<string> input_filename_;	//入力ファイル名
	vector<Formation> formation_;		//陣形指定
	int times_;		//試行回数
	int threads_;	//スレッド数
	string output_filename_;	//出力ファイル名
public:
	// コンストラクタ
	Config() {}
	Config(int argc, char *argv[]);
	// 中身を表示する
	void Put() const;
	// getter
	string GetInputFilename(const int n) const { return input_filename_[n]; }
	std::wstring GetInputFilenameW(const int n) const;
	Formation GetFormation(const int n) const { return formation_[n]; }
	int GetTimes() const { return times_; }
	int GetThreads() const { return threads_; }
	string GetOutputFilename() { return output_filename_; }
	friend std::ostream& operator<<(std::ostream& os, const Config& conf);
	friend std::wostream& operator<<(std::wostream& os, const Config& conf);
};
std::ostream& operator<<(std::ostream& os, const Config& conf);
std::wostream& operator<<(std::wostream& os, const Config& conf);
