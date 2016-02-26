#pragma once

// 陣形
enum Formation { kFormationTrail, kFormationSubTrail, kFormationCircle, kFormationEchelon, kFormationAbreast };
const string kFormationStr[] = { "単縦陣", "複縦陣", "輪形陣", "梯形陣", "単横陣" };

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
};
