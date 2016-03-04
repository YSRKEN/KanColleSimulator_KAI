#include "base.hpp"
#include "result.hpp"

// getter


// コンストラクタ

Result::Result() {
	hp_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize)));
	damage_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, 0)));
}

int Result::GetHP(const int bi, const int fi, const int ui) const noexcept { return hp_[bi][fi][ui]; }
int Result::GetDamage(const int bi, const int fi, const int ui) const noexcept { return damage_[bi][fi][ui]; }
int Result::GerParam(const int type, const int bi, const int fi, const int ui) const noexcept { return (type == 0 ? GetHP(bi, fi, ui) : GetDamage(bi, fi, ui)); }

// その他
string Result::Put() const {
	std::stringstream output;
	output << "残りHP：" << endl;
	for (auto bi = 0; bi < kBattleSize; ++bi) {
		for (auto fi = 0u; fi < kMaxFleetSize; ++fi) {
			for (auto ui = 0u; ui < kMaxUnitSize; ++ui) {
				output << hp_[bi][fi][ui] << ",";
			}
		}
		output << endl;
	}
	output << "ダメージ量：" << endl;
	for (auto bi = 0; bi < kBattleSize; ++bi) {
		for (auto fi = 0u; fi < kMaxFleetSize; ++fi) {
			for (auto ui = 0u; ui < kMaxUnitSize; ++ui) {
				output << damage_[bi][fi][ui] << ",";
			}
		}
		output << endl;
	}
	return output.str();
}
