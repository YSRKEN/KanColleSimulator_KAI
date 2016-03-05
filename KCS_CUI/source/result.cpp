#include "base.hpp"
#include "result.hpp"
#include <array>
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
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		for (size_t fi = 0; fi < kMaxFleetSize; ++fi) {
			for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
				output << hp_[bi][fi][ui] << "/" << hp_before_[bi][fi][ui] << ",";
			}
		}
		output << endl;
	}
	output << "与ダメージ：" << endl;
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		for (size_t fi = 0; fi < kMaxFleetSize; ++fi) {
			for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
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
	// 自艦隊および敵艦隊の生存数を数える
	std::array<int, kBattleSize> alived_count_before = {};	//戦闘開始時の生存艦数
	std::array<int, kBattleSize> alived_count = {};			//生存艦数
	std::array<int, kBattleSize> dead_count;				//撃沈艦数
	std::array<int, kBattleSize> alived_count_head = {};		//敵旗艦を沈めたか？
	std::array<int, kBattleSize> hp_before_sum = {};			//戦闘開始時の残耐久合計
	std::array<int, kBattleSize> hp_sum = {};					//残耐久合計
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		for (size_t fi = 0; fi < kMaxFleetSize; ++fi) {
			for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
				hp_before_sum[bi] += hp_before_[bi][fi][ui];
				hp_sum[bi] += hp_[bi][fi][ui];
				if (hp_before_[bi][fi][ui] == 0) continue;
				++alived_count_before[bi];
				if (hp_[bi][fi][ui] == 0) continue;
				++alived_count[bi];
				if (ui == 0) alived_count_head[bi] = 1;
			}
		}
		dead_count[bi] = alived_count_before[bi] - alived_count[bi];
	}
	// 戦果ゲージを算出する
	vector<double> result_per(kBattleSize);				//戦果ゲージ(敵艦隊の撃沈割合であることに注意)
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		auto other_side = kBattleSize - bi - 1;
		result_per[bi] = 1.0 * hp_sum[other_side] / hp_before_sum[other_side];
	}
	// 上記の条件から勝利判定を行う
	static const int kill_half[] = {2, 1, 1, 2, 2, 3, 4};
	if (dead_count[kFriendSide] == 0) {
		// 自艦隊に撃沈艦が存在しない場合
		// 　敵艦隊を全滅させた場合
		if (alived_count[kEnemySide] == 0) {
			// 自艦隊のダメージ0ならば完全勝利S、そうでない場合は勝利S
			return (result_per[kEnemySide] == 0.0 ? WinReason::SS : WinReason::S);
		}
		// 　敵艦隊を半分以上沈めた場合
		if (kill_half[alived_count_before[kEnemySide]] <= dead_count[kEnemySide]) return WinReason::A;
		// 　敵旗艦を沈めた場合
		if (!alived_count_head[kEnemySide]) return WinReason::B;
		// 戦果ゲージによる判定
		if (result_per[kFriendSide] >= result_per[kEnemySide] * 2.5) {
			return WinReason::B;
		}
		else return (result_per[kFriendSide] >= result_per[kEnemySide] || result_per[kFriendSide] >= 0.5 ? WinReason::C : WinReason::D);
	} else {
		// 自艦隊に撃沈艦が存在する場合
		// 　敵旗艦を沈めた場合
		if (!alived_count_head[kEnemySide]) {
			return (dead_count[kFriendSide] < dead_count[kEnemySide] ? WinReason::B : WinReason::C);
		}
		// 自艦隊が多く沈められた場合
		if (kill_half[alived_count_before[kFriendSide]] <= dead_count[kFriendSide]) return WinReason::E;
		// 戦果ゲージによる判定
		if (result_per[kFriendSide] >= result_per[kEnemySide] * 2.5) {
			return WinReason::B;
		} else if (result_per[kFriendSide] >= result_per[kEnemySide]) {
			return WinReason::C;
		} else return (result_per[kFriendSide] >= 0.5 ? WinReason::D : WinReason::E);
	}
}
