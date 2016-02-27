#pragma once

class Result {
	vector<vector<int>> hp_;
	vector<vector<int>> damages_;
public:
	// コンストラクタ
	Result() {
		hp_.resize(kBattleSize, vector<int>(kMaxFleetSize));
		damages_.resize(kBattleSize, vector<int>(kMaxFleetSize, 0));
	}
};
