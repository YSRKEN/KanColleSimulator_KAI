#include "base.hpp"
#include "config.hpp"
#include "fleet.hpp"
#include "result.hpp"
#include "simulator.hpp"

// 計算用メソッド
Result Simulator::Calc() {
	result_ = Result();

	// 索敵フェイズ
	auto search_result = SearchPhase();

	for (auto i = 0; i < kBattleSize; ++i) {
		cout << search_result[i] << " " << fleet_[i].SearchValue() << " " << fleet_[i].HasAir() << "\n";
	}
	cout << "\n";

	// 航空戦フェイズ
	auto air_war_result = AirWarPhase(search_result);

	cout << get<0>(air_war_result) << " " << get<1>(air_war_result)[0] << " " << get<1>(air_war_result)[1] << "\n\n";

	// 交戦形態の決定

	// 支援艦隊攻撃フェイズ(未実装)

	// 開幕雷撃フェイズ

	// 砲撃戦フェイズ(1巡目)

	// 砲撃戦フェイズ(2巡目)

	// 雷撃フェイズ

	// 夜戦フェイズ

	// 結果を出力する
	for (auto bi = 0; bi < kBattleSize; ++bi) {
		for (auto fi = 0u; fi < fleet_[bi].FleetSize(); ++fi) {
			for (auto ui = 0u; ui < fleet_[bi].UnitSize(fi); ++ui){
				result_.SetHP(bi, fi, ui, fleet_[bi].GetUnit(fi, ui).GetHP());
//				result.AddDamage(bi, fi, ui, RandInt(100));
			}
		}
	}
	return result_;
}

// 索敵フェイズ
bitset<kBattleSize> Simulator::SearchPhase() {
	// 索敵の成功条件がよく分からないので、とりあえず次のように定めた
	// ・艦載機があれば無条件で成功
	// ・艦載機が存在しない場合、索敵値が0より大なら成功
	bitset<kBattleSize> search_result(kBattleSize);
	for (auto i = 0; i < kBattleSize; ++i) {
		auto search_value = fleet_[i].SearchValue();
		search_result[i] = (search_value > 0.0 || fleet_[i].HasAir());
	}
	return search_result;
}

// 航空戦フェイズ
tuple<AirWarStatus, vector<double>> Simulator::AirWarPhase(const bitset<kBattleSize> &search_result) {
	// 制空状態の決定

	// 触接判定

	// 空中戦

	// 対空砲火

	// 開幕爆撃

	return tuple <AirWarStatus, vector<double>>(kAirWarStatusNormal, { 1.0, 1.0 }) ;
}
