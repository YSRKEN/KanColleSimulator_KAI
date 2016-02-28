#include "base.hpp"
#include "config.hpp"
#include "fleet.hpp"
#include "result.hpp"
#include "simulator.hpp"

// 計算用メソッド
Result Simulator::Calc() {
	Result result;

	// 索敵フェイズ
	//まず索敵値を計算する
	vector<double> search_value(kBattleSize);
	for (auto i = 0; i < kBattleSize; ++i) {
		search_value[i] = fleet_[i].SearchValue();
	}
	cout << search_value[0] << " " << search_value[1] << "\n\n";

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
