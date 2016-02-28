#include "base.hpp"
#include "config.hpp"
#include "fleet.hpp"
#include "result.hpp"
#include "simulator.hpp"

// 計算用メソッド
Result Simulator::Calc() {
	Result result;
	for (auto bi = 0; bi < kBattleSize; ++bi) {
		for (auto fi = 0u; fi < sfleet_[bi].FleetSize(); ++fi) {
			for (auto ui = 0u; ui < sfleet_[bi].UnitSize(fi); ++ui){
				result.SetHP(bi, fi, ui, sfleet_[bi].GetUnit(fi, ui).GetHP());
			}
		}
	}
	return result;
}
