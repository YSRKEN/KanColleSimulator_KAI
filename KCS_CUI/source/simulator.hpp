#pragma once

class Simulator {
	vector<Fleet> sfleet_;
	std::mt19937 mt;
public:
	// コンストラクタ
	Simulator(){}
	Simulator(const vector<Fleet> &fleet, const unsigned int &seed) {
		sfleet_ = fleet;
		mt = std::mt19937(seed);
	}
	// 計算用メソッド
	Result Calc();
};
