﻿#pragma once

class Result {
	vector<vector<vector<int>>> hp_;
	vector<vector<vector<int>>> damage_;
public:
	// コンストラクタ
	Result() {
		hp_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize)));
		damage_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, 0)));
	}
	// setter
	void SetHP(const int bi, const int fi, const int ui, const int hp) { hp_[bi][fi][ui] = hp; }
	void AddDamage(const int bi, const int fi, const int ui, const int damage) { damage_[bi][fi][ui] += damage; }
	// その他
	string Put() const{
		std::stringstream output;
		output << "残りHP：\n";
		for (auto bi = 0; bi < kBattleSize; ++bi) {
			for (auto fi = 0u; fi < kMaxFleetSize; ++fi) {
				for (auto ui = 0u; ui < kMaxUnitSize; ++ui) {
					output << hp_[bi][fi][ui] << ",";
				}
			}
			output << "\n";
		}
		output << "ダメージ量：\n";
		for (auto bi = 0; bi < kBattleSize; ++bi) {
			for (auto fi = 0u; fi < kMaxFleetSize; ++fi) {
				for (auto ui = 0u; ui < kMaxUnitSize; ++ui) {
					output << damage_[bi][fi][ui] << ",";
				}
			}
			output << "\n";
		}
		return output.str();
	}
};
