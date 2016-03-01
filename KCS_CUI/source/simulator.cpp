#include "base.hpp"
#include "config.hpp"
#include "fleet.hpp"
#include "simulator.hpp"

Simulator::Simulator(const vector<Fleet>& fleet, const unsigned int seed)
	: fleet_(fleet), rand(seed)
{
	for (auto& f : this->fleet_) f.SetRandGenerator(rand);
}

// 計算用メソッド
Result Simulator::Calc() {
	result_ = Result();
#ifdef KCS_DEBUG_MODE
	for (auto bi = 0; bi < kBattleSize; ++bi) {
		for (auto fi = 0u; fi < fleet_[bi].FleetSize(); ++fi) {
			for (auto ui = 0u; ui < fleet_[bi].UnitSize(fi); ++ui) {
				result_.SetHP(bi, fi, ui, fleet_[bi].GetUnit()[fi][ui].GetHP());
			}
		}
	}
	cout << result_.Put() << endl;
#endif

	// 索敵フェイズ
	auto search_result = SearchPhase();
#ifdef KCS_DEBUG_MODE
	cout << "索敵結果・索敵値・艦載機を持っているか：\n";
	for (auto i = 0; i < kBattleSize; ++i) {
		cout << search_result[i] << " " << fleet_[i].SearchValue() << " " << fleet_[i].HasAir() << endl;
	}
	cout << endl;
#endif
	// 航空戦フェイズ
	auto air_war_result = AirWarPhase(search_result);
#ifdef KCS_DEBUG_MODE
	cout << "制空状態・自艦隊倍率・敵艦隊倍率：\n";
	cout << get<0>(air_war_result) << " " << get<1>(air_war_result)[0] << " " << get<1>(air_war_result)[1] << "\n" << endl;
#endif

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
				result_.SetHP(bi, fi, ui, fleet_[bi].GetUnit()[fi][ui].GetHP());
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
	//制空値を計算する
	vector<int> anti_air_score(2);
	for (auto i = 0; i < kBattleSize; ++i) {
		anti_air_score[i] = fleet_[i].AntiAirScore();
	}
#ifdef KCS_DEBUG_MODE
	cout << "制空値：" << anti_air_score[0] << " " << anti_air_score[1] << "\n" << endl;
#endif
	//制空状態を判断する
	auto air_war_status = JudgeAirWarStatus(search_result, anti_air_score);

	// 触接判定
	// TODO:simulatorクラス内のrand.RandReal関数を別の関数に「渡す」には？
	vector<double> all_attack_plus(2, 1.0);
	for (auto i = 0; i < kBattleSize; ++i) {
		// 触接発生条件
		if (!search_result[i]) continue;
		if((i == kFriendSide && air_war_status == kAirWarStatusWorst)
		|| (i == kEnemySide && air_war_status == kAirWarStatusBest)) continue;
		if (!fleet_[i].HasAirTrailer()) continue;
		// 触接の開始率を計算する
		auto trailer_aircraft_prob = fleet_[i].TrailerAircraftProb(air_war_status);
		if (trailer_aircraft_prob < rand.RandReal()) continue;	//触接は確率的に開始される
		// 触接の選択率を計算する
		all_attack_plus[i] = fleet_[i].TrailerAircraftPlus();
	}

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
	for (auto i = 0; i < kBattleSize; ++i) {
		for (auto &it_u : fleet_[i].GetUnit()) {
			for (auto &it_k : it_u) {
				for (auto wi = 0; wi < it_k.GetSlots(); ++wi) {
					auto &it_w = it_k.GetWeapon()[wi];
					if (!it_w.IsAirFight()) continue;
					it_k.GetAir()[wi] -= int(it_k.GetAir()[wi] * killed_airs_per[i]);
				}
			}
		}
	}

	// 対空砲火
	for (auto i = 0; i < kBattleSize; ++i) {
		// 艦隊対空ボーナス値を決定
		int anti_air_bonus = fleet_[i].AntiAirBonus();
		// 対空カットイン判定を行う
		int aac_type = fleet_[i].AacType();
		// 迎撃！
		//加重対空値
		//                       0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
		const int aac_bonus_add1[] = { 0,7,6,4,6,4,4,3,4,2,8, 6, 3, 0, 4, 3, 4, 2, 2};
		auto other_side = kBattleSize - i - 1;	//自分にとっての敵、敵にとっての自分
		for (auto &it_k : fleet_[other_side].GetUnit()[0]) {
			for (auto wi = 0; wi < it_k.GetSlots(); ++wi) {
				auto &it_w = it_k.GetWeapon()[wi];
				if (!it_w.IsAirFight()) continue;
				Kammusu &intercept_kammusu = fleet_[i].GetUnit()[0][fleet_[i].RandomKammusu()];	//迎撃艦
				auto all_anti_air = intercept_kammusu.AllAntiAir();									//加重対空値
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
					/* 種類／対空 11  45  48  69.4 75  91  102 108 126 142
						* 第9種(+2)      +1  +2    +2          +3          +4
						* 第7種(+3)                       +3   +3  +4  +5  +5
						* 第8種(+4)                +3 +3       +4
						* 第4種(+6)                            +5
						* 第1種(+7)  +0
						*/
					if (aac_type > 0) {
						killed_airs += int(-1.1376 + 0.2341 * aac_bonus_add1[aac_type] + 0.0392 * all_anti_air + 0.5);
					}
				}
				//割合撃墜
				if (rand.RandBool()) killed_airs += int(int(0.9 * all_anti_air) * it_k.GetAir()[wi] / 360);
				//対空カットイン成功時の固定ボーナス
				killed_airs += aac_bonus_add1[aac_type];
				//艦娘限定ボーナス
				if (intercept_kammusu.IsKammusu()) killed_airs += 1;
				//撃墜処理
				if (it_k.GetAir()[wi] > killed_airs) {
					it_k.GetAir()[wi] -= killed_airs;
				}
				else {
					it_k.GetAir()[wi] = 0;
				}
			}
		}
	}

	// 開幕爆撃
	//ダメージ計算
	vector<vector<int>> all_damage(kBattleSize, vector<int>(kMaxUnitSize, 0));
	for (auto bi = 0; bi < kBattleSize; ++bi) {
		auto other_side = kBattleSize - bi - 1;
		auto &friend_unit = fleet_[bi].GetUnit()[0];
		for (auto ui = 0u; ui < friend_unit.size(); ++ui) {
			auto &friend_kammusu = friend_unit[ui];
			auto &friend_weapon = friend_kammusu.GetWeapon();
			for (auto wi = 0; wi < friend_kammusu.GetSlots(); ++wi) {
				if (friend_kammusu.GetAir()[wi] == 0 || !friend_weapon[wi].IsAirBomb()) continue;
				// 爆撃する対象を決定する
				auto target = fleet_[other_side].RandomKammusuNonSS(friend_kammusu.HasAirBomb());
				// 基礎攻撃力を算出する
				int base_attack;
				switch (friend_weapon[wi].GetWeaponClass()) {
				case kWeaponClassPBF:
				case kWeaponClassPB:
				case kWeaponClassWB:
					// 爆撃は等倍ダメージ
					base_attack = int(friend_weapon[wi].GetBomb() * sqrt(friend_kammusu.GetAir()[wi]) + 25);
					break;
				case kWeaponClassPA:
					// 雷撃は150％か80％かがランダムで決まる
					base_attack = int((rand.RandBool() ? 1.5 : 0.8) * (friend_weapon[wi].GetTorpedo() * sqrt(friend_kammusu.GetAir()[wi]) + 25));
					break;
				default:
					base_attack = 0;
					break;
				}
				// 与えるダメージを計算する
				auto damage = CalcDamage(kBattlePhaseAir, bi, { 0, int(ui) }, { 0, target }, base_attack, all_attack_plus, kBattlePositionSame);
				result_.AddDamage(bi, 0, ui, damage);
				all_damage[other_side][target] += damage;
			}
		}
	}
	// ダメージ処理
	for (auto bi = 0; bi < kBattleSize; ++bi) {
		auto &friend_unit = fleet_[bi].GetUnit()[0];
		for (auto ui = 0u; ui < friend_unit.size(); ++ui) {
			friend_unit[ui].SetRandGenerator(this->rand);
			friend_unit[ui].MinusHP(all_damage[bi][ui], (bi == kFriendSide));
		}
	}

#ifdef KCS_DEBUG_MODE
	cout << "残機：" << endl;
	for (auto i = 0; i < kBattleSize; ++i) {
		for (auto &it_u : fleet_[i].GetUnit()) {
			for (auto &it_k : it_u) {
				for (auto wi = 0; wi < it_k.GetSlots(); ++wi) {
					cout << it_k.GetAir()[wi] << " ";
				}
				cout << endl;
			}
		}
	}
	cout << endl;
	cout << "受けたダメージ：" << endl;
	for (auto bi = 0; bi < kBattleSize; ++bi) {
		auto &friend_unit = fleet_[bi].GetUnit()[0];
		for (auto ui = 0u; ui < friend_unit.size(); ++ui) {
			cout << all_damage[bi][ui] << ",";
		}
		cout << endl;
	}
	cout << endl;
#endif

	return tuple <AirWarStatus, vector<double>>(air_war_status, all_attack_plus) ;
}

//制空状態を判断する
AirWarStatus Simulator::JudgeAirWarStatus(const bitset<kBattleSize> &search_result, const vector<int> &anti_air_score) {
	// どちらも航空戦に参加する艦載機を持っていなかった場合は航空均衡
	if (!fleet_[kFriendSide].HasAirFight() && !fleet_[kEnemySide].HasAirFight()) {
		return kAirWarStatusNormal;
	}
	// 敵が制空値0か索敵失敗していた場合は制空権確保
	else if (anti_air_score[kEnemySide] == 0 || !search_result[kEnemySide]) {
		return kAirWarStatusBest;
	}
	// 味方が索敵失敗していた場合は制空権喪失
	else if (!search_result[kFriendSide]) {
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
	const BattlePhase &battle_phase, const int &turn_player, const vector<int> &friend_index, const vector<int> &enemy_index,
	const int &base_attack, const vector<double> &all_attack_plus, const BattlePosition &battle_position){
	return this->rand.RandInt(0, base_attack);	//仮置きのメソッド
}
