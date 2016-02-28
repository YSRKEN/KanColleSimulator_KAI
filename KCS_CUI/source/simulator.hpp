#pragma once

class Simulator {
	vector<Fleet> fleet_;	//シミュレーションに使用する艦隊
	Result result_;			//シミュレーション結果を保存するクラス
	// 乱数用メンバ関数
	std::mt19937 mt;										//
	std::uniform_real_distribution<double> rand;			//
	double RandReal() { return rand(mt); }					//一様実数乱数を取り出す
	int RandInt(const int n) { return int(rand(mt) * n); }	//一様整数乱数を取り出す
	// 計算用メソッド(内部)
	bitset<kBattleSize> SearchPhase();
public:
	// コンストラクタ
	Simulator(){}
	Simulator(const vector<Fleet> &fleet, const unsigned int &seed) {
		fleet_ = fleet;
		mt = std::mt19937(seed);
		rand = std::uniform_real_distribution<double>(0.0, 1.0);
	}
	// 計算用メソッド
	Result Calc();
};
