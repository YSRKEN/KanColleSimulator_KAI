﻿#pragma once

// 設定クラス
class Config {
	vector<string> input_filename_;	//入力ファイル名
	vector<Formation> formation_;	//陣形指定
	int times_;		//試行回数
	int threads_;	//スレッド数
	string output_filename_;	//出力ファイル名
public:
	// コンストラクタ
	Config(int argc, char *argv[]);
	// 中身を表示する
	void Put() const;
	// getter
	string InputFilename(const int n) { return input_filename_[n]; }
	Formation GetFormation(const int n) { return formation_[n]; }
};
