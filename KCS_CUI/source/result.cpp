#include "base.hpp"
#include "result.hpp"

// コンストラクタ
Result::Result() {
	hp_before_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize)));
	hp_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize)));
	damage_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, 0)));
	damage_night_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, 0)));
}

// getter
int Result::GetHP(const int bi, const int fi, const int ui) const noexcept { return hp_[bi][fi][ui]; }
int Result::GetDamage(const int bi, const int fi, const int ui, const bool special_mvp_flg) const noexcept {
	return (special_mvp_flg ? damage_night_[bi][fi][ui] : damage_[bi][fi][ui]);
}
int Result::GerParam(const int type, const int bi, const int fi, const int ui) const noexcept { return (type == 0 ? GetHP(bi, fi, ui) : GetDamage(bi, fi, ui)); }
bool Result::GetNightFlg() const noexcept { return night_flg_;}

// その他
string Result::Put() const {
	std::stringstream output;
	output << "残/元HP：" << endl;
	for (auto bi = 0; bi < kBattleSize; ++bi) {
		for (auto fi = 0u; fi < kMaxFleetSize; ++fi) {
			for (auto ui = 0u; ui < kMaxUnitSize; ++ui) {
				output << hp_[bi][fi][ui] << "/" << hp_before_[bi][fi][ui] << ",";
			}
		}
		output << endl;
	}
	output << "与ダメージ：" << endl;
	for (auto bi = 0; bi < kBattleSize; ++bi) {
		for (auto fi = 0u; fi < kMaxFleetSize; ++fi) {
			for (auto ui = 0u; ui < kMaxUnitSize; ++ui) {
				output << damage_[bi][fi][ui] << ",";
			}
		}
		output << endl;
	}
	output << "勝利判定：" << kWinReasonStr[uint_fast8_t(JudgeWinReason())] << endl;
	return output.str();
}

// 勝利判定
WinReason Result::JudgeWinReason() const noexcept {

	return WinReason::S;	//仮置き
}
