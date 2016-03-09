#include "base.hpp"
#include "config.hpp"
#include "fleet.hpp"
#include "simulator.hpp"
#include <algorithm>
#include <cassert>
using namespace std::string_literals;

Simulator::Simulator(const vector<Fleet>& fleet, const unsigned int seed, const SimulateMode& simulate_mode)
	: fleet_(fleet), result_(), rand(seed), simulate_mode_(simulate_mode), search_result_(),
	stopper_(),
	is_calculated()
{
	for (auto& f : this->fleet_) f.SetRandGenerator(rand);
}

SharedRand Simulator::GetGenerator() noexcept { return this->rand; }

inline void Simulator::Flush_Calc_Result(const vector<Fleet>& fleet) {
	this->fleet_ = fleet;
	this->is_calculated = false; 
}

// 計算用メソッド
tuple<Result, vector<Fleet>> Simulator::Calc() {
	// 今のところ、敵が連合艦隊である場合には対応していない
	if (fleet_[kEnemySide].GetFleetType() != FleetType::Normal) return {};
	// すでに計算されていたら再計算しない
	RUNTIME_ERROR_THROW_WITH_MESSAGE_IF(this->is_calculated, "call Flush_Calc_Result() to re-calclate.");

	// 戦闘開始時の体力を入力する
	//stopper_ is initialised at ctor
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		for (size_t fi = 0; fi < fleet_[bi].FleetSize(); ++fi) {
			for (size_t ui = 0; ui < fleet_[bi].UnitSize(fi); ++ui) {
				result_.SetBeforeHP(bi, fi, ui, fleet_[bi].GetUnit()[fi][ui].GetHP());
				if (bi == kFriendSide && fleet_[bi].GetUnit()[fi][ui].Status() < kStatusHeavyDamage) {
					stopper_[bi][fi][ui] = true;
				}
				else {
					stopper_[bi][fi][ui] = false;
				}
			}
		}
	}

#ifdef _DEBUG
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		for (size_t fi = 0; fi < fleet_[bi].FleetSize(); ++fi) {
			for (size_t ui = 0; ui < fleet_[bi].UnitSize(fi); ++ui) {
				result_.SetHP(bi, fi, ui, fleet_[bi].GetUnit()[fi][ui].GetHP());
			}
		}
	}
	cout << result_.Put() << endl;
#endif

	// 索敵フェイズ
	SearchPhase();
#ifdef _DEBUG
	cout << "索敵結果・索敵値・艦載機を持っているか：\n";
	for (size_t i = 0; i < kBattleSize; ++i) {
		cout << search_result_[i] << " " << fleet_[i].SearchValue() << " " << fleet_[i].HasAir() << endl;
	}
	cout << endl;
#endif
	if (simulate_mode_ != kSimulateModeN) {
		// 航空戦フェイズ(連合艦隊では第一艦隊のみ関わるが、第二艦隊も開幕爆撃の被害対象になる)
		AirWarPhase();
		if (IsBattleTerminate()) goto SimulatorCalcExit;
#ifdef _DEBUG
		cout << "制空状態・自艦隊倍率・敵艦隊倍率：\n";
		cout << std::get<condition>(air_war_result_) << " " << std::get<magnification>(air_war_result_)[0] << " " << std::get<magnification>(air_war_result_)[1] << "\n" << endl;
#endif

		// 交戦形態の決定
		BattlePositionOracle();
#ifdef _DEBUG
		cout << "交戦形態：" << battle_position_ << "\n" << endl;
#endif

		// 支援艦隊攻撃フェイズ(未実装)

		// 開幕雷撃フェイズ(連合艦隊では第2艦隊のみ)
		TorpedoPhase(kTorpedoFirst);
		if (IsBattleTerminate()) goto SimulatorCalcExit;

		// 砲撃戦フェイズ
		switch (fleet_[kFriendSide].GetFleetType()) {
		case FleetType::Normal:
			// 通常艦隊：1巡目→2巡目
			FirePhase(kFireFirst);
			if (IsBattleTerminate()) goto SimulatorCalcExit;
			FirePhase(kFireSecond);
			if (IsBattleTerminate()) goto SimulatorCalcExit;
			break;
		case FleetType::CombinedAir:
		case FleetType::CombinedDrum:
			// 空母機動・輸送護衛：第2艦隊(1巡目順)→第1艦隊1巡目→第1艦隊2巡目
			FirePhase(kFireFirst, 1);
			if (IsBattleTerminate()) goto SimulatorCalcExit;
			FirePhase(kFireFirst, 0);
			if (IsBattleTerminate()) goto SimulatorCalcExit;
			FirePhase(kFireSecond, 0);
			if (IsBattleTerminate()) goto SimulatorCalcExit;
			break;
		case FleetType::CombinedGun:
			// 水上打撃：第1艦隊1巡目→第1艦隊2巡目→第2艦隊(1巡目順)
			FirePhase(kFireFirst, 0);
			if (IsBattleTerminate()) goto SimulatorCalcExit;
			FirePhase(kFireSecond, 0);
			if (IsBattleTerminate()) goto SimulatorCalcExit;
			FirePhase(kFireFirst, 1);
			if (IsBattleTerminate()) goto SimulatorCalcExit;
			break;
		}

		// 雷撃フェイズ(連合艦隊では第2艦隊のみ)
		TorpedoPhase(kTorpedoSecond);
		if (IsBattleTerminate()) goto SimulatorCalcExit;

		// 夜戦フェイズ(連合艦隊では第2艦隊のみ)
		NightPhase();
	}
	else {
		// 夜戦フェイズ(連合艦隊では第2艦隊のみ)
		NightPhase();
	}

	// 終了処理
SimulatorCalcExit:
	// 燃料・弾薬を減少させる
	//連合艦隊かつ対潜マスなら弾薬消費無し
	bool combined_ss_flg = fleet_[kFriendSide].GetFleetType() != FleetType::Normal && fleet_[kEnemySide].GetUnit().front()[0].IsSubmarine();
	//計算
	for (auto &it_u : fleet_[kFriendSide].GetUnit()) {
		for (auto &it_k : it_u) {
			it_k.ConsumeMaterial(result_.GetNightFlg(), combined_ss_flg);
		}
	}

	//フラグ切り替え
	this->is_calculated = true;
	// 結果を出力する
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		for (size_t fi = 0; fi < fleet_[bi].FleetSize(); ++fi) {
			for (size_t ui = 0; ui < fleet_[bi].UnitSize(fi); ++ui){
				result_.SetHP(bi, fi, ui, fleet_[bi].GetUnit()[fi][ui].GetHP());
			}
		}
	}
#ifdef _DEBUG
	cout << result_.Put() << endl;
#endif

	// 勝利判定によって、cond値を変化させる
	fleet_[kFriendSide].ChangeCond(simulate_mode_, result_);

	return std::make_tuple(result_, move(this->fleet_));
}

// 索敵フェイズ
void Simulator::SearchPhase() {
	// 索敵の成功条件がよく分からないので、とりあえず次のように定めた
	// ・艦載機があれば無条件で成功
	// ・艦載機が存在しない場合、索敵値が0より大なら成功
	assert(kBattleSize == search_result_.size());
	for (size_t i = 0; i < kBattleSize; ++i) {
		auto search_value = fleet_[i].SearchValue();
		search_result_[i] = (search_value > 0.0 || fleet_[i].HasAir());
	}
}

// 航空戦フェイズ
void Simulator::AirWarPhase() {
	// 制空状態の決定
	//制空値を計算する
	vector<int> anti_air_score(2);
	for (size_t i = 0; i < kBattleSize; ++i) {
		anti_air_score[i] = fleet_[i].AntiAirScore();
	}
#ifdef _DEBUG
	cout << "制空値：" << anti_air_score[0] << " " << anti_air_score[1] << "\n" << endl;
#endif
	//制空状態を判断する
	auto air_war_status = JudgeAirWarStatus(anti_air_score);

	// 空中戦
	double killed_airs_per[kBattleSize];
	switch (air_war_status) {
	case kAirWarStatusBest:
		killed_airs_per[kFriendSide] = rand.RandReal(7, 15) / 256;
		killed_airs_per[kEnemySide] = rand.RandReal();
		break;
	case kAirWarStatusGood:
		killed_airs_per[kFriendSide] = rand.RandReal(20, 45) / 256;
		killed_airs_per[kEnemySide] = rand.RandReal() * 0.8;
		break;
	case kAirWarStatusNormal:
		killed_airs_per[kFriendSide] = rand.RandReal(30, 75) / 256;
		killed_airs_per[kEnemySide] = rand.RandReal() * 0.6;
		break;
	case kAirWarStatusBad:
		killed_airs_per[kFriendSide] = rand.RandReal(45, 105) / 256;
		killed_airs_per[kEnemySide] = rand.RandReal() * 0.4;
		break;
	case kAirWarStatusWorst:
		killed_airs_per[kFriendSide] = rand.RandReal(65, 150) / 256;
		killed_airs_per[kEnemySide] = rand.RandReal() * 0.1;
		break;
	}
	for (size_t i = 0; i < kBattleSize; ++i) {
		for (auto &it_k : fleet_[i].GetUnit().front()) {
			if (it_k.Status() == kStatusLost) continue;
			for (auto& it_w : it_k.GetWeapon()) {
				if (!it_w.AnyOf(WeaponClass::AirFight)) continue;
				it_w.SetAir(it_w.GetAir() - int(it_w.GetAir() * killed_airs_per[i]));
			}
		}
	}

	// 触接判定
	vector<double> all_attack_plus(2, 1.0);
	for (size_t i = 0; i < kBattleSize; ++i) {
		// 触接発生条件
		if (!search_result_[i]) continue;
		if ((i == kFriendSide && air_war_status == kAirWarStatusWorst)
			|| (i == kEnemySide && air_war_status == kAirWarStatusBest)) continue;
		if (!fleet_[i].HasAirTrailer()) continue;
		// 触接の開始率を計算する
		auto trailer_aircraft_prob = fleet_[i].TrailerAircraftProb(air_war_status);
		if (trailer_aircraft_prob < rand.RandReal()) continue;	//触接は確率的に開始される
		// 触接の選択率を計算する
		all_attack_plus[i] = fleet_[i].TrailerAircraftPlus();
	}

	// 対空砲火
	for (size_t i = 0; i < kBattleSize; ++i) {
		// 艦隊対空ボーナス値を決定
		int anti_air_bonus = fleet_[i].AntiAirBonus();
		// 対空カットイン判定を行う
		int aac_type = fleet_[i].AacType();
		// 迎撃！
		//加重対空値
		//                             0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
		const int aac_bonus_add1[] = { 0,7,6,4,6,4,4,3,4,2,8, 6, 3, 0, 4, 3, 4, 2, 2};
		auto other_side = kBattleSize - i - 1;	//自分にとっての敵、敵にとっての自分
		for (auto &it_k : fleet_[other_side].GetUnit().front()) {
			if (it_k.Status() == kStatusLost) continue;
			for (auto& it_w : it_k.GetWeapon()) {
				if (!it_w.AnyOf(WeaponClass::AirFight)) continue;				//航空戦に参加しないものは落とされない
				if (it_w.GetTorpedo() == 0 && it_w.GetBomb() == 0) continue;	//雷装も爆装もないものは落とされない
				auto intercept_index = fleet_[i].RandomKammusu();
				if (!std::get<is_alive>(intercept_index)) continue;
				Kammusu &intercept_kammusu = fleet_[i].GetUnit().front()[std::get<selected>(intercept_index)];	//迎撃艦
				auto all_anti_air = intercept_kammusu.AllAntiAir();								//加重対空値
				int killed_airs = 0;
				//固定撃墜
				if (rand.RandBool()) {
					if (intercept_kammusu.IsKammusu()) {
						killed_airs += int(0.1 * (all_anti_air + anti_air_bonus));
					}
					else {
						killed_airs += int(1.0 * (all_anti_air + anti_air_bonus) / 10.6);
					}
					// 対空カットイン成功時における固定撃墜の追加ボーナス
					/* | 種類／対空 | 11 | 45 | 48 | 69.4 | 75 | 91 | 102 | 108 | 126 | 142 |
					 * |------------|----|----|----|------|----|----|-----|-----|-----|-----|
					 * | 第9種(+2)  |    | +1 | +2 | +2   |    |    | +3  |     |     | +4  |
					 * | 第7種(+3)  |    |    |    |      |    | +3 | +3  | +4  | +5  | +5  |
					 * | 第8種(+4)  |    |    |    | +3   | +3 |    | +4  |     |     |     |
					 * | 第4種(+6)  |    |    |    |      |    |    | +5  |     |     |     |
					 * | 第1種(+7)  | +0 |    |    |      |    |    |     |     |     |     |
					 */
					if (aac_type > 0) {
						killed_airs += int(-1.1376 + 0.2341 * aac_bonus_add1[aac_type] + 0.0392 * all_anti_air + 0.5);
					}
				}
				//割合撃墜
				if (rand.RandBool()) killed_airs += int(int(0.9 * all_anti_air) * it_w.GetAir() / 360);
				//対空カットイン成功時の固定ボーナス
				killed_airs += aac_bonus_add1[aac_type];
				//艦娘限定ボーナス
				if (intercept_kammusu.IsKammusu()) killed_airs += 1;
				//撃墜処理
				if (it_w.GetAir() > killed_airs) {
					it_w.SetAir(it_w.GetAir() - killed_airs);
				}
				else {
					it_w.SetAir(0);
				}
			}
		}
	}

	// 開幕爆撃
	//ダメージ計算
	vector<vector<vector<int>>> all_damage(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, 0)));
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		auto other_side = kBattleSize - bi - 1;
		auto &friend_unit = fleet_[bi].GetUnit().front();
		for (size_t ui = 0; ui < friend_unit.size(); ++ui) {
			auto &hunter_kammusu = friend_unit[ui];
			// 既に沈んでいる場合は攻撃できない
			if (hunter_kammusu.Status() == kStatusLost) continue;
			// 敵に攻撃できない場合は次の艦娘にバトンタッチ
			auto has_attacker = fleet_[other_side].RandomKammusuNonSS(false, kTargetTypeAll);
			if (!std::get<is_attackable>(has_attacker)) continue;
			// そうでない場合は、各スロットに対して攻撃対象を選択する
			for (const auto& it_w : hunter_kammusu.GetWeapon()) {
				if (it_w.GetAir() == 0 || !it_w.AnyOf(WeaponClass::AirBomb)) continue;
				// 爆撃する対象を決定する(各スロット毎に、ランダムに対象を選択しなければならない)
				auto target = std::get<selected>(fleet_[other_side].RandomKammusuNonSS(false, kTargetTypeAll));
				// 基礎攻撃力を算出する
				int base_attack;
				switch (it_w.GetWeaponClass()) {
				case WeaponClass::PBF:
				case WeaponClass::PB:
				case WeaponClass::WB:
					// 爆撃は等倍ダメージ
					base_attack = int(it_w.GetBomb() * sqrt(it_w.GetAir()) + 25);
					break;
				case WeaponClass::PA:
					// 雷撃は150％か80％かがランダムで決まる
					base_attack = int((rand.RandBool() ? 1.5 : 0.8) * (it_w.GetTorpedo() * sqrt(it_w.GetAir()) + 25));
					break;
				default:
					base_attack = 0;
					break;
				}
				// 与えるダメージを計算する
				KammusuIndex enemy_index = target;
				auto damage = CalcDamage(kBattlePhaseAir, bi, KammusuIndex{ 0, ui }, enemy_index, base_attack, all_attack_plus, kBattlePositionSame, false, 1.0);
				result_.AddDamage(bi, 0, ui, damage);
				all_damage[other_side][enemy_index.fleet_no][enemy_index.fleet_i] += damage;
			}
		}
	}
	// ダメージ処理
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		for (size_t fi = 0; fi < fleet_[bi].FleetSize(); ++fi) {
			auto &friend_unit = fleet_[bi].GetUnit()[fi];
			for (size_t ui = 0; ui < friend_unit.size(); ++ui) {
				friend_unit[ui].SetRandGenerator(this->rand);
				friend_unit[ui].MinusHP(all_damage[bi][fi][ui], stopper_[bi][fi][ui]);
			}
		}
	}

#ifdef _DEBUG
	cout << "残機：" << endl;
	for (size_t i = 0; i < kBattleSize; ++i) {
		for (auto &it_k : fleet_[i].GetUnit().front()) {
			for (const auto& it_w : it_k.GetWeapon()) {
				cout << it_w.GetAir() << " ";
			}
			cout << endl;
		}
	}
	cout << endl;
	cout << "航空戦で受けたダメージ：" << endl;
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		for (size_t fi = 0; fi < fleet_[bi].FleetSize(); ++fi) {
			auto &friend_unit = fleet_[bi].GetUnit()[fi];
			for (size_t ui = 0; ui < friend_unit.size(); ++ui) {
				cout << all_damage[bi][fi][ui] << ",";
			}
			cout << endl;
		}
	}
	cout << endl;
#endif

	air_war_result_ = std::make_tuple(air_war_status, move(all_attack_plus));
}

// 交戦形態を確率的に決定する
void Simulator::BattlePositionOracle() noexcept {
	// 彩雲を持っているかを判断する
	auto has_pss = fleet_[0].HasAirPss();
	int dice = rand.RandInt(100);
	if (dice < 45) {
		battle_position_ = kBattlePositionSame;
	}
	else if (dice < 60) {
		battle_position_ = kBattlePositionGoodT;
	}
	else {
		if (dice < 90 || has_pss) {
			battle_position_ = kBattlePositionReverse;
		}
		else {
			battle_position_ = kBattlePositionBadT;
		}
	}
}

// (開幕)雷撃フェイズ
void Simulator::TorpedoPhase(const TorpedoTurn &torpedo_turn) {
	// ダメージ計算
	vector<vector<int>> all_damage(kBattleSize, vector<int>(kMaxUnitSize, 0));
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		auto other_side = kBattleSize - bi - 1;
		auto &friend_unit = fleet_[bi].GetUnit().back();
		for (size_t ui = 0; ui < friend_unit.size(); ++ui) {
			auto &hunter_kammusu = friend_unit[ui];
			// 魚雷が発射できるかどうかを判定する
			if (!hunter_kammusu.IsFireTorpedo(torpedo_turn)) continue;
			// 既に沈んでいる場合は攻撃できない
			if (hunter_kammusu.Status() == kStatusLost) continue;
			// 敵に攻撃できない場合は次の艦娘にバトンタッチ
			auto target = fleet_[other_side].RandomKammusuNonSS(true, kTargetTypeSecond);
			if(!std::get<is_attackable>(target)) continue;
			// 基礎攻撃力を算出する
			int base_attack = hunter_kammusu.AllTorpedo(true);
			if(fleet_[bi].FleetSize() == 1) base_attack += 5;
			// 与えるダメージを計算する
			KammusuIndex enemy_index = std::get<selected>(target);
			auto damage = CalcDamage((torpedo_turn == kTorpedoFirst ? kBattlePhaseFirstTorpedo : kBattlePhaseTorpedo), bi, KammusuIndex{ fleet_[bi].SecondIndex(), ui }, enemy_index, base_attack, false, 1.0);
			result_.AddDamage(bi, fleet_[bi].SecondIndex(), ui, damage);
			all_damage[other_side][enemy_index.fleet_i] += damage;
		}
	}
	// ダメージ処理
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		auto &friend_unit = fleet_[bi].GetUnit().back();
		for (size_t ui = 0; ui < friend_unit.size(); ++ui) {
			friend_unit[ui].SetRandGenerator(this->rand);
			friend_unit[ui].MinusHP(all_damage[bi][ui], stopper_[bi][fleet_[bi].SecondIndex()][ui]);
		}
	}

#ifdef _DEBUG
	cout << (torpedo_turn == kTorpedoFirst ? "開幕雷撃で" : "雷撃戦で") << "受けたダメージ：" << endl;
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		auto &friend_unit = fleet_[bi].GetUnit().back();
		for (size_t ui = 0; ui < friend_unit.size(); ++ui) {
			cout << all_damage[bi][ui] << ",";
		}
		cout << endl;
	}
	cout << endl;
#endif
}

vector<vector<std::pair<KammusuIndex, Range>>> Simulator::DetermineAttackOrder(FireTurn fire_turn, const size_t fleet_index) const
{
	vector<vector<std::pair<KammusuIndex, Range>>> attack_list(kBattleSize);
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		auto &unit = this->fleet_[bi].GetUnit();
		// 行動可能な艦娘一覧を作成する
		if (bi == kFriendSide) {
			auto &unit2 = unit[fleet_index];
			for (size_t ui = 0; ui < unit2.size(); ++ui) {
				if (!unit2[ui].IsMoveGun()) continue;
				attack_list[bi].push_back({ { fleet_index, ui }, unit2[ui].MaxRange() });
			}
		}
		else {
			auto &unit2 = unit[0];
			for (size_t ui = 0; ui < unit2.size(); ++ui) {
				if (!unit2[ui].IsMoveGun()) continue;
				attack_list[bi].push_back({ { 0, ui }, unit2[ui].MaxRange() });
			}
		}
		if (fire_turn == kFireFirst) {
			// 一覧をシャッフルする
			std::shuffle(attack_list[bi].begin(), attack_list[bi].end(), this->rand.get());
			// 一覧を射程で安定ソートする
			std::stable_sort(attack_list[bi].begin(), attack_list[bi].end(), [](const auto& a, const auto& b) {return (b.second < a.second); });
		}
	}
	return attack_list;
}
std::tuple<bool, bool, double> Simulator::CalcLandingObservationShootingCorrection(
	DayFireType fire_type, size_t bi, const Kammusu& hunter_kammusu, size_t other_side,	KammusuIndex friend_index
) const {
	const auto default_re = std::make_tuple(false, false, 1.0);
	// 砲撃時にのみ適用される
	if (fire_type != kDayFireGun) return default_re;
	// 索敵に成功していないとダメな上、大破状態でも使えない
	if (!this->search_result_[bi] || hunter_kammusu.Status() >= kStatusHeavyDamage) return default_re;
	// 航空優勢以上でないと使えない
	if (bi == kFriendSide && std::get<condition>(air_war_result_) > kAirWarStatusGood) return default_re;
	if (bi == other_side  && std::get<condition>(air_war_result_) < kAirWarStatusBad)  return default_re;
	// 発動可能な弾着の種類を判断する
	const auto special_attack = this->JudgeDaySpecialAttack(bi, friend_index);
	if (std::get<condition>(special_attack) == 1.0) return default_re;
	// 弾着観測射撃による補正
	return std::make_tuple(std::get<condition>(special_attack), true, std::get<magnification>(special_attack));
}
// 砲撃戦フェイズ
void Simulator::FirePhase(const FireTurn &fire_turn, const size_t &fleet_index) {
#ifdef _DEBUG
	cout << "【砲撃フェイズ(" << (fire_turn == kFireFirst ? "1巡目" : "2巡目") << ", 第" << (fleet_index + 1) << "艦隊)】" << endl << endl;
#endif
	// 2巡目は、どちらかの艦隊に戦艦か陸上型が存在していない場合には実行されない
	if (fire_turn == kFireSecond) {
		const bool has_bb = [] (const vector<Fleet>& fleet) -> bool {
			for (auto &it_b : fleet) {
				for (auto &it_k : it_b.GetUnit().front()) {
					if (it_k.AnyOf(ShipClass::BB | ShipClass::BBV | ShipClass::AF))
						return true;
				}
			}
			return false;
		}(this->fleet_);
		if(!has_bb) return;
	}
	// 攻撃順を決定する
	const auto attack_list = DetermineAttackOrder(fire_turn, fleet_index);
	// 決定した巡目に基づいて攻撃を行う
	for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
		// 基本的に、敵と味方が交互に砲撃を行う
		for (size_t bi = 0; bi < kBattleSize; ++bi) {
			auto other_side = kBattleSize - bi - 1;
			// 一覧の範囲外なら飛ばす
			if (attack_list[bi].size() <= ui) continue;
			// 担当する艦娘が攻撃参加できない場合は飛ばす
			const auto &friend_index = attack_list[bi][ui].first;
			const auto &hunter_kammusu = fleet_[bi].GetUnit()[friend_index.fleet_no][friend_index.fleet_i];
			if (!hunter_kammusu.IsFireGun()) continue;
			// 攻撃の種類を判断し、攻撃対象を選択する
			tuple<bool, KammusuIndex> target(false, {0, 0});
			if (hunter_kammusu.IsAntiSubDay()) {
				target = fleet_[other_side].RandomKammusuSS(fleet_index);
			}
			if (!std::get<is_attackable>(target)) {
				target = fleet_[other_side].RandomKammusuNonSS(hunter_kammusu.HasAirBomb(), TargetType(fleet_index));
			}
			if (!std::get<is_attackable>(target)) continue;
			// 攻撃対象の種類によって、攻撃の種類を選ぶ
			auto &enemy_index = std::get<selected>(target);
			const auto &target_kammusu = fleet_[other_side].GetUnit()[enemy_index.fleet_no][enemy_index.fleet_i];
			auto fire_type = JudgeDayFireType(bi, friend_index, enemy_index);
			// 攻撃の種類によって、基本攻撃力および倍率を算出する
			int base_attack = hunter_kammusu.DayAttack(fire_type, target_kammusu.AnyOf(ShipClass::AF), fleet_[bi].GetFleetType(), friend_index.fleet_no);
			bool special_attack_flg, double_flg;
			double multiple;
			// 弾着観測射撃補正
			std::tie(special_attack_flg, double_flg, multiple) = this->CalcLandingObservationShootingCorrection(fire_type, bi, hunter_kammusu, other_side, friend_index);
			// 与えるダメージを計算し、処理を行う
#ifdef _DEBUG
			wcout << hunter_kammusu.GetName() << L"(" << (bi == kFriendSide ? L"自" : L"敵") << L")が" << target_kammusu.GetName() << L"に攻撃！ " << endl;
			cout << "基礎攻撃力：" << base_attack << " 弾着観測射撃：" << special_attack_flg << " 連撃：" << double_flg << "倍率：" << multiple << endl;
#endif
			auto damage = CalcDamage(kBattlePhaseGun, bi, friend_index, enemy_index, base_attack, special_attack_flg, multiple);
#ifdef _DEBUG
			cout << damage << "ダメージ！" << endl;
#endif
			result_.AddDamage(bi, friend_index.fleet_no, friend_index.fleet_i, damage);
			fleet_[other_side].GetUnit()[enemy_index.fleet_no][enemy_index.fleet_i].MinusHP(damage, stopper_[other_side][enemy_index.fleet_no][enemy_index.fleet_i]);
			if (double_flg && fleet_[other_side].GetUnit()[enemy_index.fleet_no][enemy_index.fleet_i].GetHP() > 0) {
				// 連撃
				damage = CalcDamage(kBattlePhaseGun, bi, friend_index, enemy_index, base_attack, special_attack_flg, multiple);
#ifdef _DEBUG
				cout << damage << "ダメージ！" << endl;
#endif
				result_.AddDamage(bi, friend_index.fleet_no, friend_index.fleet_i, damage);
				fleet_[other_side].GetUnit()[enemy_index.fleet_no][enemy_index.fleet_i].MinusHP(damage, stopper_[other_side][enemy_index.fleet_no][enemy_index.fleet_i]);
			}
#ifdef _DEBUG
			cout << endl;
#endif
		}
	}
	return;
}

// 夜戦フェイズ
void Simulator::NightPhase() {
#ifdef _DEBUG
	cout << "【夜戦フェイズ】" << endl << endl;
#endif
	result_.SetNightFlg(true);
	// 夜偵が発動するかを確率的に判断する
	bool wsn_flg = false;
	auto aws = std::get<0>(air_war_result_);
	//制空権確保・航空優勢・航空劣勢・開幕夜戦時に発動判定を行う
	if (aws == kAirWarStatusBest || aws == kAirWarStatusGood || aws == kAirWarStatusBad || simulate_mode_ == kSimulateModeN) {
		for (auto &it_k : fleet_[kFriendSide].GetUnit().back()) {
			for (auto &it_w : it_k.GetWeapon()) {
				if (it_w.GetWeaponClass() != WeaponClass::WSN) continue;
				double prob = ((sqrt(50 * it_k.GetLevel()) - 3) / 100) | limit(0.0, 0.99);
				if (!rand.RandBool(prob)) continue;
				wsn_flg = true;
				break;
			}
			if (wsn_flg) break;
		}
	}
	// 夜戦でのダメージは、Result#damage_だけでなくResult#damage_night_にも代入すること
	for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
		for (size_t bi = 0; bi < kBattleSize; ++bi) {
			auto other_side = kBattleSize - bi - 1;
			// 単純に範囲外なら飛ばす
			if (fleet_[bi].GetUnit().back().size() <= ui) continue;
			// 担当する艦娘が攻撃参加できない場合は飛ばす
			auto friend_index = KammusuIndex{ fleet_[bi].SecondIndex(), ui };
			auto &hunter_kammusu = fleet_[bi].GetUnit().back()[ui];
			if (!hunter_kammusu.IsFireNight()) continue;
			// 攻撃の種類を判断し、攻撃対象を選択する
			tuple<bool, KammusuIndex> target(false, { 0, 0 });
			if (hunter_kammusu.IsAntiSubNight()) {
				target = fleet_[other_side].RandomKammusuSS(1);
			}
			if (!std::get<is_attackable>(target)) {
				target = fleet_[other_side].RandomKammusuNonSS(hunter_kammusu.HasAirBomb(), kTargetTypeSecond, true);
			}
			if (!std::get<is_attackable>(target)) continue;
			// 攻撃対象の種類によって、攻撃の種類を選ぶ
			auto &enemy_index = std::get<selected>(target);
			auto &target_kammusu = fleet_[other_side].GetUnit()[enemy_index.fleet_no][enemy_index.fleet_i];
			auto fire_type = JudgeNightFireType(bi, enemy_index);
			// 攻撃の種類によって、基本攻撃力および倍率を算出する
			// 夜戦速吸は対潜を常に爆雷で行う
			auto base_attack = hunter_kammusu.NightAttack(fire_type, target_kammusu.AnyOf(ShipClass::AF)) + (wsn_flg ? 5 : 0);
			bool special_attack_flg = false;
			bool double_flg = false;
			auto multiple = 1.0;
			// 夜間特殊攻撃補正
			[&] {
				// 砲撃時にのみ適用される
				if (fire_type != kNightFireGun) return;
				// 発動可能な弾着の種類を判断する
				auto special_attack = JudgeNightSpecialAttack(bi, friend_index, target_kammusu.AnyOf(ShipClass::AF));
				if (std::get<condition>(special_attack) == 1.0) return;
				// 弾着観測射撃による補正
				double_flg = std::get<condition>(special_attack);
				special_attack_flg = true;
				multiple = std::get<magnification>(special_attack);
			}();
			// 与えるダメージを計算し、処理を行う
#ifdef _DEBUG
			wcout << hunter_kammusu.GetName() << L"(" << (bi == kFriendSide ? L"自" : L"敵") << L")が" << target_kammusu.GetName() << L"に攻撃！ " << endl;
			cout << "基礎攻撃力：" << base_attack << " 夜間特殊攻撃：" << special_attack_flg << " 連撃：" << double_flg << "倍率：" << multiple << endl;
#endif
			auto damage = CalcDamage(kBattlePhaseNight, bi, friend_index, enemy_index, base_attack, special_attack_flg, multiple);
#ifdef _DEBUG
			cout << damage << "ダメージ！" << endl;
#endif
			result_.AddDamage(bi, friend_index.fleet_no, friend_index.fleet_i, damage, true);
			fleet_[other_side].GetUnit()[enemy_index.fleet_no][enemy_index.fleet_i].MinusHP(damage, stopper_[other_side][enemy_index.fleet_no][enemy_index.fleet_i]);
			if (double_flg && fleet_[other_side].GetUnit()[enemy_index.fleet_no][enemy_index.fleet_i].GetHP() > 0) {
				// 連撃
				damage = CalcDamage(kBattlePhaseNight, bi, friend_index, enemy_index, base_attack, special_attack_flg, multiple);
#ifdef _DEBUG
				cout << damage << "ダメージ！" << endl;
#endif
				result_.AddDamage(bi, friend_index.fleet_no, friend_index.fleet_i, damage, true);
				fleet_[other_side].GetUnit()[enemy_index.fleet_no][enemy_index.fleet_i].MinusHP(damage, stopper_[other_side][enemy_index.fleet_no][enemy_index.fleet_i]);
			}
#ifdef _DEBUG
			cout << endl;
#endif
#ifdef _DEBUG
	wcout << hunter_kammusu.GetNameLv() << endl << endl;
#endif
		}
	}
}

// 制空状態を判断する
AirWarStatus Simulator::JudgeAirWarStatus(const vector<int> &anti_air_score) {
	// どちらも航空戦に参加する艦載機を持っていなかった場合は航空均衡
	if (!fleet_[kFriendSide].HasAirFight() && !fleet_[kEnemySide].HasAirFight()) {
		return kAirWarStatusNormal;
	}
	// 敵が制空値0か索敵失敗していた場合は制空権確保
	else if (anti_air_score[kEnemySide] == 0 || !search_result_[kEnemySide]) {
		return kAirWarStatusBest;
	}
	// 味方が索敵失敗していた場合は制空権喪失
	else if (!search_result_[kFriendSide]) {
		return kAirWarStatusWorst;
	}
	// 後は普通に判定する
	else if (anti_air_score[kFriendSide] * 3 <= anti_air_score[kEnemySide]) {
		return kAirWarStatusWorst;
	}
	else if (anti_air_score[kFriendSide] * 3 <= anti_air_score[kEnemySide] * 2) {
		return kAirWarStatusBad;
	}
	else if (anti_air_score[kFriendSide] * 2 < anti_air_score[kEnemySide] * 3) {
		return kAirWarStatusNormal;
	}
	else if (anti_air_score[kFriendSide] < anti_air_score[kEnemySide] * 3) {
		return kAirWarStatusGood;
	}
	else {
		return kAirWarStatusBest;
	}
}

// 与えるダメージ量を計算する
int Simulator::CalcDamage(
	const BattlePhase &battle_phase, const size_t turn_player, const KammusuIndex &friend_index, KammusuIndex &enemy_index,
	const int &base_attack, const vector<double> &all_attack_plus, const BattlePosition &battle_position,
	const bool &is_special_attack, const double &multiple
) const {
	auto other_side = kBattleSize - turn_player - 1;
	// 旗艦相手への攻撃に限り、「かばい」が確率的に発生する
	ProtectOracle(other_side, enemy_index);
	const auto &friend_side = fleet_[turn_player];
	const auto &enemy_side  = fleet_[other_side];
	const auto &hunter_kammusu = friend_side.GetUnit()[friend_index.fleet_no][friend_index.fleet_i];
	const auto &target_kammusu = enemy_side.GetUnit()[enemy_index.fleet_no][enemy_index.fleet_i];
	// 攻撃の命中率を計算する
	double hit_prob = CalcHitProb(friend_side.GetFormation(), enemy_side.GetFormation(), hunter_kammusu, target_kammusu, battle_phase, turn_player, friend_index.fleet_no);
	// 対潜攻撃かどうかを判断する
	auto is_target_submarine = target_kammusu.IsSubmarine();
	if (is_target_submarine && battle_phase != kBattlePhaseGun
		&& battle_phase != kBattlePhaseNight) return 0;		//砲撃戦および夜戦以外ではそもそも対潜攻撃を行わない
	// 三式弾・WG42による対地上施設特効
	double damage = base_attack;
	if (target_kammusu.AnyOf(ShipClass::AF)) {
		bool has_aaa = false;
		auto wg_count = 0;
		for (auto &it_w : hunter_kammusu.GetWeapon()) {
			if (it_w.AnyOf(WeaponClass::AAA)) has_aaa = true;
			if (it_w.AnyOf(L"WG42"s)) ++wg_count;
		}
		if (has_aaa) damage *= 2.5;
		static const double wg_plus[] = { 0, 75, 109, 142, 162 };
		damage += wg_plus[wg_count];
	}
	// キャップ前補正
	if (battle_phase != kBattlePhaseAir) {
		if (battle_phase != kBattlePhaseNight) {
			// 交戦形態補正
			switch (battle_position) {
			case kBattlePositionSame:
				damage *= 1.0;
				break;
			case kBattlePositionReverse:
				damage *= 0.8;
				break;
			case kBattlePositionGoodT:
				damage *= 1.2;
				break;
			case kBattlePositionBadT:
				damage *= 0.6;
				break;
			}
			// 攻撃側陣形補正
			if (friend_side.FleetSize() == 1) {
				// 通常艦隊
				switch (friend_side.GetFormation()) {
				case kFormationTrail:		//単縦陣
					damage *= (is_target_submarine) ? 0.6 : 1.0;
					break;
				case kFormationSubTrail:	//複縦陣
					damage *= (is_target_submarine) ? 0.8 : 0.8;
					break;
				case kFormationCircle:		//輪形陣
					damage *= (is_target_submarine) ? 1.2 : 0.7;
					break;
				case kFormationEchelon:		//梯形陣
					damage *= (is_target_submarine) ? 1.0 : 0.6;
					break;
				case kFormationAbreast:		//単横陣
					damage *= (is_target_submarine) ? 1.3 : 0.6;
					break;
				}
			}
			else {
				//連合艦隊
				switch (friend_side.GetFormation()) {
				case kFormationTrail:		//第四警戒航行序列(戦闘隊形)＝単縦陣
					damage *= (is_target_submarine) ? 0.7 : 1.1;
					break;
				case kFormationSubTrail:	//第二警戒航行序列(前方警戒)＝複縦陣
					damage *= (is_target_submarine) ? 1.1 : 1.0;
					break;
				case kFormationCircle:		//第三警戒航行序列(輪形陣)＝輪形陣
					damage *= (is_target_submarine) ? 1.0 : 0.7;
					break;
				case kFormationAbreast:		//第一警戒航行序列(対潜警戒)＝単横陣
					damage *= (is_target_submarine) ? 1.3 : 0.8;
					break;
				default:	//「梯形陣」は読み込み時に排除している
					break;
				}
			}
		}
		else {
			// 夜戦特殊攻撃補正
			if (is_special_attack) damage *= multiple;
		}
		// 損傷状態補正
		switch (hunter_kammusu.Status()) {
		case kStatusMiddleDamage:
			damage *= (battle_phase == kBattlePhaseFirstTorpedo) ? 0.8 : 0.7;
			break;
		case kStatusHeavyDamage:
			damage *= (battle_phase == kBattlePhaseFirstTorpedo) ? 0.0 : 0.4;
			break;
		default:
			break;
		}
		// 対潜シナジー補正
		if(hunter_kammusu.HasAntiSubSynergy()) damage *= 1.15;
	}
	// 軽巡軽量砲補正
	damage += hunter_kammusu.FitGunAttackPlus();
	// キャップ計算
	if (is_target_submarine) {
		damage = LimitCap(damage, 100);
	}
	else {
		if (battle_phase == kBattlePhaseNight) {
			damage = LimitCap(damage, 300);
		}
		else {
			damage = LimitCap(damage, 150);
		}
	}
	// キャップ後補正
	damage = int(damage);	//※この切り捨ては仕様です
	{
		// 徹甲弾補正
		if (target_kammusu.IsSpecialEffectAP()) {
			damage *= int(hunter_kammusu.SpecialEffectApPlus());	//※この切り捨ては仕様です
		}
		// クリティカル補正
		double prob_per;
		switch (battle_phase) {
		case kBattlePhaseAir:
			prob_per = 0.025;
			break;
		case kBattlePhaseNight:
			prob_per = 0.3;		//この値は試験実装
			break;
		default:
			prob_per = 0.25 * hit_prob / (hit_prob + 1) + 0.0125;
		}
		prob_per += hunter_kammusu.CL2ProbPlus();	//熟練艦載機によるCL2率上昇(試験実装)
		if (rand.RandBool(prob_per)) {
			damage *= 1.5 * hunter_kammusu.CL2AttackPlus();	//熟練艦載機によるダメージ補正
		}
		damage = int(damage);	//※この切り捨ては仕様です
		// 触接補正
		damage *= all_attack_plus[turn_player];
		// 弾着観測射撃補正
		if (battle_phase == kBattlePhaseGun && is_special_attack) damage *= multiple;
	}
	// 装甲乱数の分だけダメージを減少させる
	damage -= 0.7 * target_kammusu.AllDefense() + 0.6 * rand.RandInt(target_kammusu.AllDefense());
	// 弾薬量補正
	if (hunter_kammusu.GetAmmo() < 50) {
		damage *= 2.0 * hunter_kammusu.GetAmmo() / 100;
	}
	// 弾着観測射撃時、カットインが入ると命中率が劇的に上昇する(試験実装)
	if (battle_phase == kBattlePhaseGun && is_special_attack) {
		if (hit_prob < 0.9) hit_prob = 0.9;
	}
	// 弾着観測射撃および夜間特殊攻撃ならば回避してもカスダメ、それ以外では0ダメージ
	if (!rand.RandBool(hit_prob | limit(0.0, 1.0))) {
		if (is_special_attack) damage = 0.0; else return 0;
	}
	// 夜戦における対潜攻撃は常にカスダメ(ただし開幕夜戦および連合艦隊では無視される)
	if (friend_side.GetFleetType() == FleetType::Normal && simulate_mode_ != kSimulateModeN && is_target_submarine && battle_phase == kBattlePhaseNight) damage = 0.0;
	// カスダメの際は相手残り耐久の6～14％を与える
	if (damage < 1.0) {
		damage = 0.06 * target_kammusu.GetHP() + 0.08 * rand.RandInt(target_kammusu.GetHP());
	}
	return int(damage);
}
int Simulator::CalcDamage(
	const BattlePhase &battle_phase, const size_t turn_player, const KammusuIndex &friend_index, KammusuIndex &enemy_index,
	const int &base_attack, const bool &is_special_attack, const double &multiple) {
	return CalcDamage(battle_phase, turn_player, friend_index, enemy_index, base_attack, std::get<magnification>(air_war_result_), battle_position_, is_special_attack, multiple);
	}

// 「かばい」を確率的に発生させる
void Simulator::ProtectOracle(const size_t defense_side, KammusuIndex &defense_index) const {
	// 旗艦ではない場合、かばいは発生しない
	if (defense_index.fleet_i != 0) return;
	// 陸上型をかばう艦などいない
	if (fleet_[defense_side].GetUnit()[defense_index.fleet_no][0].AnyOf(ShipClass::AF)) return;
	// 水上艦は水上艦、潜水艦は潜水艦しかかばえないのでリストを作成する
	const auto &attendants = fleet_[defense_side].GetUnit()[defense_index.fleet_no];
	auto is_submarine = attendants[0].IsSubmarine();
	std::array<size_t, kMaxUnitSize> block_list;
	size_t block_list_size = 0;
	for (size_t ui = 1; ui < attendants.size(); ++ui) {
		if (attendants[ui].IsSubmarine() == is_submarine && attendants[ui].Status() < kStatusLightDamage) {
			block_list[block_list_size] = ui;
			++block_list_size;
		}
	}
	if (block_list_size == 0) return;
	// かばいは確率的に発生し、どの艦がかばうかも確率的に決まる
	if (rand.RandBool(0.4)) {	//とりあえず4割に設定している
		defense_index.fleet_i = rand.select_random_in_range(block_list, block_list_size);
	}
	return;
}

//命中率を計算する
double Simulator::CalcHitProb(
	const Formation friend_formation, const Formation enemy_formation, const Kammusu &hunter_kammusu,
	const Kammusu &target_kammusu, const BattlePhase battle_phase, const size_t turn_player, const size_t index) const noexcept {
	double hit_prob = 1.0;
	switch (battle_phase) {
	case kBattlePhaseAir:
	case kBattlePhaseGun:
	case kBattlePhaseNight:
		{
			/* 昼砲撃戦命中率 */
			// 回避側
			//まずは回避値を出す
			double evade_sum = target_kammusu.AllEvade();
			if (enemy_formation == kFormationEchelon || enemy_formation == kFormationAbreast) evade_sum *= 1.2;
			if (target_kammusu.Mood() == kMoodHappy) evade_sum *= 1.8;
			//そこから回避項を求める
			double evade_value;
			if (evade_sum <= 40) {
				evade_value = 0.03 + evade_sum / 80;
			}
			else {
				evade_value = 0.03 + evade_sum / (evade_sum + 40);
			}
			// 命中側
			//練度による命中率補正
			double hit_value = 1.0 + sqrt(hunter_kammusu.GetLevel() - 1) / 50;
			//装備による明示的な命中率補正
			hit_value += hunter_kammusu.AllHit() / 100;
			//赤疲労状態だと命中率が激減する
			if (hunter_kammusu.Mood() == kMoodRed) hit_value /= 1.9;
			//命中率の運補正
			hit_value += hunter_kammusu.GetLuck() * 0.0015;
			//フィット砲補正
			hit_value += hunter_kammusu.FitGunHitPlus();
			//命中率の陣形補正
			switch (friend_formation) {
			case kFormationSubTrail:
				if (enemy_formation != kFormationAbreast) hit_value += 0.2;
				break;
			case kFormationEchelon:
				if (enemy_formation != kFormationSubTrail) hit_value += 0.2;
				break;
			case kFormationAbreast:
				if (enemy_formation != kFormationTrail) hit_value += 0.2;
				break;
			default:
				break;
			}
			//引き算により命中率を決定する(上限あり)
			hit_prob = hit_value - evade_value;
			// 残燃料による回避補正
			hit_prob *= 3.73127 - 3.53991 * std::min(target_kammusu.GetFuel(), 80) / 100;
			hit_prob = std::min(hit_prob, 0.97);
		}
	break;
	case kBattlePhaseFirstTorpedo:
	case kBattlePhaseTorpedo:
		{
			// 雷撃戦命中率
			//命中側
			double hit_value = 0.91587;	//命中項
			double T = 1.0;
			switch (battle_position_) {
			case kBattlePositionSame:
				T = 1.0;
				break;
			case kBattlePositionReverse:
				T = 0.8242;
				break;
			case kBattlePositionGoodT:
				T = 1.191;
				break;
			case kBattlePositionBadT:
				T = 0.6046;
				break;
			}
			hit_value += 0.02188 * sqrt(hunter_kammusu.GetLevel() - 1);
			hit_value += T * int(0.001426 * hunter_kammusu.AllTorpedo(false) + 0.000836 * hunter_kammusu.GetTorpedo());
			hit_value += 0.01009 * hunter_kammusu.AllHit();
			hit_value += hunter_kammusu.SumWeapons([](const auto& it_w) {
				return it_w.AnyOf(WeaponClass::Torpedo) ? 0.02104 * sqrt(it_w.GetLevel()) : 0;
			});
			hit_value += 0.001482 * hunter_kammusu.GetLuck();
			//回避側
			double a;
			switch (enemy_formation) {
			case kFormationTrail:
				a = 38.63;
				break;
			case kFormationSubTrail:
				a = 38.4;
				break;
			case kFormationCircle:
				a = 33.69;
				break;
			default:
				a = 38.63;
				break;
			}
			double evade_sum = target_kammusu.AllEvade();
			double evade_value;		//回避項
			if (evade_sum < a) {
				evade_value = evade_sum / (2.0 * a);
			}
			else {
				evade_value = evade_sum / (evade_sum + a);
			}
			//引き算により命中率を決定する(上限あり)
			hit_prob = hit_value - evade_value;
			// 残燃料による回避補正
			hit_prob *= 3.73127 - 3.53991 * std::min(target_kammusu.GetFuel(), 80) / 100;
			hit_prob = std::min(hit_prob, 0.9698);
		}
	break;
	}
	// 連合艦隊における命中率補正を試験的に実装
	if (battle_phase != kBattlePhaseNight) {
		switch (fleet_[kFriendSide].GetFleetType()) {
		case FleetType::CombinedAir:
		case FleetType::CombinedDrum:
			hit_prob /= 2.0;
			if (turn_player == kFriendSide && index == 0) hit_prob += 0.2;
			hit_prob = hit_prob | limit(0.3, 0.97);
			break;
		case FleetType::CombinedGun:
			hit_prob /= 2.0;
			if (turn_player == kFriendSide && index == 1) hit_prob += 0.2;
			hit_prob = hit_prob | limit(0.3, 0.97);
			break;
		default:
			break;
		}
	}
	return hit_prob;
}

// 戦闘終了を判断する
bool Simulator::IsBattleTerminate() const noexcept {
	for (auto &it_u : fleet_[kEnemySide].GetUnit()) {
		for (auto &it_k : it_u) {
			if (it_k.Status() != kStatusLost) return false;
		}
	}
	return true;
}

// 昼戦での攻撃種別を判断する
DayFireType Simulator::JudgeDayFireType(const size_t turn_player, const KammusuIndex &attack_index, const KammusuIndex &defense_index) const noexcept {
	auto other_side = kBattleSize - turn_player - 1;
	// 敵が潜水艦なら対潜攻撃
	if (fleet_[other_side].GetUnit()[defense_index.fleet_no][defense_index.fleet_i].IsSubmarine()) return kDayFireChage;
	// 自身が空母系統なら空撃
	auto &hunter_kammusu = fleet_[turn_player].GetUnit()[attack_index.fleet_no][attack_index.fleet_i];
	if (hunter_kammusu.AnyOf(ShipClass::CV | ShipClass::ACV | ShipClass::CVL)) return kDayFireAir;
	if(hunter_kammusu.AnyOf(ShipClass::AO) && hunter_kammusu.IsFireGunPlane()) return kDayFireAir;
	// それ以外は全て砲撃
	return kDayFireGun;
}

// 昼戦での特殊攻撃を判断する
tuple<bool, double> Simulator::JudgeDaySpecialAttack(const size_t turn_player, const KammusuIndex &attack_index) const {
	// 主砲・副砲・徹甲弾・電探・偵察機の数を数える
	// (水上戦闘機は偵察機に含めない)
	const auto &hunter_kammusu = fleet_[turn_player].GetUnit()[attack_index.fleet_no][attack_index.fleet_i];
	size_t sum_gun = 0, sum_subgun = 0, sum_ap = 0, sum_radar = 0, sum_ws = 0;
	for (auto &it_w : hunter_kammusu.GetWeapon()) {
		switch (it_w.GetWeaponClass()) {
		case WeaponClass::Gun:
			++sum_gun;
			break;
		case WeaponClass::SubGun:
			++sum_subgun;
			break;
		case WeaponClass::AP:
			++sum_ap;
			break;
		case WeaponClass::SmallR:
		case WeaponClass::LargeR:
			++sum_radar;
			break;
		case WeaponClass::WB:
		case WeaponClass::WS:
		case WeaponClass::WSN:
			++sum_ws;
		default:
			break;
		}
	}
	// 発動可能な攻撃方法を判断する
	if(sum_ws == 0) return tuple<bool, double>(false, 1.0);
	const bool cutin_gg_flg = (sum_gun == 2 && sum_subgun == 0 && sum_ap == 1 && sum_radar == 0);
	const bool cutin_ga_flg = (sum_gun == 1 && sum_subgun == 1 && sum_ap == 1 && sum_radar == 0);
	const bool cutin_gr_flg = (sum_gun == 1 && sum_subgun == 1 && sum_ap == 0 && sum_radar == 1);
	const bool cutin_gs_flg = (sum_gun >= 1 && sum_subgun >= 1);
	const bool double_flg = (sum_gun >= 2);
	/* 発動確率は「艦隊の索敵が高い」「複合する」場合に高くなる
	 * また、連撃とカットインが複合した場合は1：2ぐらいの割合で後者が優先されるらしい
	 * ……どう実装しろっていうのさ日向？ 結構適当に実装するけどいいの？ */
	if (cutin_gg_flg || cutin_ga_flg || cutin_gr_flg || cutin_gs_flg || double_flg) {
		double oracle = 0.0;
		oracle += (cutin_gg_flg ? 0.2 : 0.0);
		oracle += (cutin_ga_flg ? 0.2 : 0.0);
		oracle += (cutin_gr_flg ? 0.2 : 0.0);
		oracle += (cutin_gs_flg ? 0.2 : 0.0);
		oracle += (double_flg ? 0.2 : 0.0);
		oracle += 0.06 * fleet_[turn_player].SearchValue();
		oracle += (attack_index.fleet_i == 0 ? 0.1 : 0.0);
		oracle += (std::get<condition>(air_war_result_) == kAirWarStatusBest ? 0.1 : 0.0);
		if (rand.RandBool(oracle | limit(0.0, 0.99))) {
			std::array<size_t, 9> roulette = {};
			size_t roulette_size = 0;
			if (cutin_gg_flg) { roulette[roulette_size++] = 0; roulette[roulette_size++] = 0; }
			if (cutin_ga_flg) { roulette[roulette_size++] = 1; roulette[roulette_size++] = 1; }
			if (cutin_gr_flg) { roulette[roulette_size++] = 2; roulette[roulette_size++] = 2; }
			if (cutin_gs_flg) { roulette[roulette_size++] = 3; roulette[roulette_size++] = 3; }
			if (double_flg) { roulette[roulette_size++] = 4; }
			assert(roulette_size <= roulette.size());
			static constexpr std::bitset<5> dflg = 1;
			static constexpr std::array<double, 5> mlt = { { 1.5, 1.3, 1.2, 1.1, 1.2 } };
			const size_t rand_pos = rand.select_random_in_range(roulette, roulette_size);
			return tuple<bool, double>(dflg[rand_pos], mlt[rand_pos]);//2つの配列は対応している・・・？
		}
	}
	return tuple<bool, double>(false, 1.0);
}

// 夜戦での攻撃種別を判断する
NightFireType Simulator::JudgeNightFireType(const size_t turn_player, const KammusuIndex &defense_index) const noexcept {
	auto other_side = kBattleSize - turn_player - 1;
	// 敵が潜水艦なら対潜攻撃
	if (fleet_[other_side].GetUnit()[defense_index.fleet_no][defense_index.fleet_i].IsSubmarine()) return kNightFireChage;
	// それ以外は全て砲撃
	return kNightFireGun;
}

// 夜戦での特殊攻撃を判断する
tuple<bool, double> Simulator::JudgeNightSpecialAttack(const size_t turn_player, const KammusuIndex &attack_index, const bool af_flg) const {
	// 主砲・副砲・魚雷の数を数える
	const auto &hunter_kammusu = fleet_[turn_player].GetUnit()[attack_index.fleet_no][attack_index.fleet_i];
	size_t sum_gun = 0, sum_subgun = 0, sum_torpedo = 0;
	for (auto &it_w : hunter_kammusu.GetWeapon()) {
		switch (it_w.GetWeaponClass()) {
		case WeaponClass::Gun:
			++sum_gun;
			break;
		case WeaponClass::SubGun:
			++sum_subgun;
			break;
		case WeaponClass::Torpedo:
			// 恐ろしいことに、敵が地上型なら魚雷を持ってない扱いにされる！
			if(!af_flg) ++sum_torpedo;
			break;
		default:
			break;
		}
	}
	// 発動可能な攻撃方法を判断する
	size_t attack_type = 0;
	if (sum_torpedo >= 2) {
		attack_type = 1;	//魚雷カットイン
	}
	else if (sum_gun >= 3){
		attack_type = 2;	//主砲カットイン
	}
	else if (sum_gun >= 2 && sum_subgun >= 1) {
		attack_type = 3;	//主砲カットイン
	}
	else if ((sum_gun == 2 && sum_subgun == 0 && sum_torpedo == 1)
		|| (sum_gun == 1 && sum_torpedo == 1)) {
		attack_type = 4;	//主魚カットイン
	}
	else if ((sum_gun == 2 && sum_subgun == 0 && sum_torpedo == 0)
		|| (sum_gun == 1 && sum_subgun >= 1 && sum_torpedo == 1)
		|| (sum_gun == 0 && sum_subgun >= 2)) {
		attack_type = 5;	//連撃
	}
	// 発動確率は、連撃が99％、他はキャップに従って計算される
	if (attack_type == 5) {
		if (rand.RandBool(0.99)) {
			return tuple<bool, double>(true, 1.2);
		}
		else {
			return tuple<bool, double>(false, 1.0);
		}
	}
	if (attack_type == 0) return tuple<bool, double>(false, 1.0);
	double oracle = 0.0;
	// 熟練見張員補正
	auto has_ssp = [&hunter_kammusu]() -> bool {
		for (auto &it_w : hunter_kammusu.GetWeapon())
			if (it_w.AnyOf(WeaponClass::SSP)) return true;
		return false; };
	// 運による発動率上昇は、運キャップによる上限がある
	switch (attack_type) {
	case 1:
		oracle += sqrt(70.0 * std::max(hunter_kammusu.GetLuck() + (has_ssp() ? 5 : 0), 60)) / 100;
		break;
	case 2:
		oracle += sqrt(50.0 * std::max(hunter_kammusu.GetLuck() + (has_ssp() ? 5 : 0), 55)) / 100;
		break;
	case 3:
		oracle += sqrt(50.0 * std::max(hunter_kammusu.GetLuck() + (has_ssp() ? 5 : 0), 55)) / 100;
		break;
	case 4:
		oracle += sqrt(70.0 * std::max(hunter_kammusu.GetLuck() + (has_ssp() ? 5 : 0), 75)) / 100;
		break;
	default:
		break;
	}
	// 配置補正
	if (attack_index.fleet_i == 0) oracle += 0.125;
	// 損傷補正
	if (hunter_kammusu.Status() == kStatusMiddleDamage) oracle += 0.2;
	// 探照灯・照明弾補正
	auto other_side = kBattleSize - turn_player - 1;
	if (fleet_[turn_player].HasLights()) oracle += 0.05;
	if (fleet_[other_side].HasLights()) oracle -= 0.10;
	// 乱数によって決定される
	oracle = oracle | limit(0.0, 0.99);
	if (rand.RandBool(oracle)) {
		static const bool dflg[] = { false, true, false, false, true };
		static const double mlt[] = { 1.0, 1.5, 2.0, 1.75, 1.3 };
		return tuple<bool, double>(dflg[attack_type], mlt[attack_type]);
	}
	else {
		return tuple<bool, double>(false, 1.0);
	}
}
