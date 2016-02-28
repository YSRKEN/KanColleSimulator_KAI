#include "base.hpp"
#include "config.hpp"
#include "fleet.hpp"
#include "result.hpp"
#include "simulator.hpp"

// 計算用メソッド
Result Simulator::Calc() {
	Result result;

	// 索敵フェイズ
	auto search_result = SearchPhase();
	//まず索敵値を計算する
	vector<double> search_value(kBattleSize);
	for (auto i = 0; i < kBattleSize; ++i) {
		cout << search_result[i] << " " << fleet_[i].SearchValue() << " " << fleet_[i].HasAir() << "\n";
	}
	cout << "\n";

	// 結果を出力する
	for (auto bi = 0; bi < kBattleSize; ++bi) {
		for (auto fi = 0u; fi < fleet_[bi].FleetSize(); ++fi) {
			for (auto ui = 0u; ui < fleet_[bi].UnitSize(fi); ++ui){
				result.SetHP(bi, fi, ui, fleet_[bi].GetUnit(fi, ui).GetHP());
//				result.AddDamage(bi, fi, ui, RandInt(100));
			}
		}
	}
	return result;
}

vector<bool> Simulator::SearchPhase() {
	// 索敵の成功条件がよく分からないので、とりあえず次のように定めた
	// ・艦載機があれば無条件で成功
	// ・艦載機が存在しない場合、索敵値が0より大なら成功
	vector<bool> search_result(kBattleSize);
	for (auto i = 0; i < kBattleSize; ++i) {
		auto search_value = fleet_[i].SearchValue();
		search_result[i] = (search_value > 0.0 || fleet_[i].HasAir());
	}
	return search_result;
}
