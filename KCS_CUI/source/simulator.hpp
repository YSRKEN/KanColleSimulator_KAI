#pragma once

class Simulator {
	Config config_;
	vector<Fleet> fleet_;
public:
	// コンストラクタ
	Simulator(){}
	Simulator(const Config &config, const vector<Fleet> &fleet) {
		config_ = config;
		fleet_ = fleet;
	}
	// 計算用メソッド
	Result Calc();
};
