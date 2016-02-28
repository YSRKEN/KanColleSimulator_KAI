#pragma once

class Result {
	vector<vector<vector<int>>> hp_;
	vector<vector<vector<int>>> damages_;
public:
	// コンストラクタ
	Result() {
		hp_.resize(kBattleSize, vector<vector<int>>(2, vector<int>(kMaxFleetSize)));
		damages_.resize(kBattleSize, vector<vector<int>>(2, vector<int>(kMaxFleetSize, 0)));
	}
	// setter
	void SetHP(const int bi, const int fi, const int ui, const int hp) { hp_[bi][fi][ui] = hp; }
	void AddDamage(const int bi, const int fi, const int ui, const int damage) { damages_[bi][fi][ui] += damage; }
};
