#include "base.hpp"
#include "config.hpp"
#include "fleet.hpp"
#include "simulator.hpp"

Simulator::Simulator(const vector<Fleet>& fleet, const unsigned int seed, const SimulateMode& simulate_mode)
	: fleet_(fleet), rand(seed), simulate_mode_(simulate_mode)
{
	for (auto& f : this->fleet_) f.SetRandGenerator(rand);
}

// 計算用メソッド
Result Simulator::Calc() {
	result_ = Result();
#ifdef _DEBUG
	for (auto bi = 0; bi < kBattleSize; ++bi) {
		for (auto fi = 0u; fi < fleet_[bi].FleetSize(); ++fi) {
			for (auto ui = 0u; ui < fleet_[bi].UnitSize(fi); ++ui) {
				result_.SetHP(bi, fi, ui, fleet_[bi].GetUnit()[fi][ui].GetHP());
			}
		}
	}
	cout << result_.Put() << endl;
#endif

	// 今のところ、敵が連合艦隊である場合には対応していない
	if (fleet_[kEnemySide].GetFleetType() != kFleetTypeNormal) return result_;

	// 索敵フェイズ
	SearchPhase();
#ifdef _DEBUG
	cout << "索敵結果・索敵値・艦載機を持っているか：\n";
	for (auto i = 0; i < kBattleSize; ++i) {
		cout << search_result_[i] << " " << fleet_[i].SearchValue() << " " << fleet_[i].HasAir() << endl;
	}
	cout << endl;
#endif
	if (simulate_mode_ != kSimulateModeN) {
		// 航空戦フェイズ
		AirWarPhase();
		if (IsBattleTerminate()) goto Exit;
#ifdef _DEBUG
		cout << "制空状態・自艦隊倍率・敵艦隊倍率：\n";
		cout << get<0>(air_war_result_) << " " << get<1>(air_war_result_)[0] << " " << get<1>(air_war_result_)[1] << "\n" << endl;
#endif

		// 交戦形態の決定
		BattlePositionOracle();
#ifdef _DEBUG
		cout << "交戦形態：" << battle_position_ << "\n" << endl;
#endif

		// 支援艦隊攻撃フェイズ(未実装)

		// 開幕雷撃フェイズ(連合艦隊では第2艦隊のみ)
		TorpedoPhase(kTorpedoFirst);
		if (IsBattleTerminate()) goto Exit;

		// 砲撃戦フェイズ(1巡目)　水上打撃なら第1艦隊・空母機動と輸送護衛では第2艦隊
		FirePhase(kFireFirst);
		if (IsBattleTerminate()) goto Exit;

		// 砲撃戦フェイズ(2巡目)　水上打撃なら第2艦隊・空母機動と輸送護衛では第1艦隊
		FirePhase(kFireSecond);
		if (IsBattleTerminate()) goto Exit;

		// 雷撃フェイズ(連合艦隊では第2艦隊のみ)
		TorpedoPhase(kTorpedoSecond);
		if (IsBattleTerminate()) goto Exit;
	}
	// 夜戦フェイズ(連合艦隊では第2艦隊のみ)
	NightPhase();

	// 結果を出力する
Exit:
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
void Simulator::SearchPhase() {
	// 索敵の成功条件がよく分からないので、とりあえず次のように定めた
	// ・艦載機があれば無条件で成功
	// ・艦載機が存在しない場合、索敵値が0より大なら成功
	search_result_ = bitset<kBattleSize>(kBattleSize);
	for (auto i = 0; i < kBattleSize; ++i) {
		auto search_value = fleet_[i].SearchValue();
		search_result_[i] = (search_value > 0.0 || fleet_[i].HasAir());
	}
}

// 航空戦フェイズ
void Simulator::AirWarPhase() {
	// 制空状態の決定
	//制空値を計算する
	vector<int> anti_air_score(2);
	for (auto i = 0; i < kBattleSize; ++i) {
		anti_air_score[i] = fleet_[i].AntiAirScore();
	}
#ifdef _DEBUG
	cout << "制空値：" << anti_air_score[0] << " " << anti_air_score[1] << "\n" << endl;
#endif
	//制空状態を判断する
	auto air_war_status = JudgeAirWarStatus(anti_air_score);

	// 触接判定
	// TODO:simulatorクラス内のrand.RandReal関数を別の関数に「渡す」には？
	vector<double> all_attack_plus(2, 1.0);
	for (auto i = 0; i < kBattleSize; ++i) {
		// 触接発生条件
		if (!search_result_[i]) continue;
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
				KammusuIndex enemy_index = { 0, target };
				auto damage = CalcDamage(kBattlePhaseAir, bi, { 0, int(ui) }, enemy_index, base_attack, all_attack_plus, kBattlePositionSame, false, 1.0);
				result_.AddDamage(bi, 0, ui, damage);
				all_damage[other_side][enemy_index[1]] += damage;
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

#ifdef _DEBUG
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

	air_war_result_ = tuple<AirWarStatus, vector<double>>(air_war_status, all_attack_plus) ;
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

// 開幕雷撃フェイズ
void Simulator::TorpedoPhase(const TorpedoTurn &torpedo_turn) {
	// ダメージ計算
	vector<vector<int>> all_damage(kBattleSize, vector<int>(kMaxUnitSize, 0));

	// ダメージ処理

}

// 砲撃戦フェイズ
void Simulator::FirePhase(const FireTurn &fire_turn) {

}

// 夜戦フェイズ
void Simulator::NightPhase() {

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
	const BattlePhase &battle_phase, const int &turn_player, const KammusuIndex &friend_index, KammusuIndex &enemy_index,
	const int &base_attack, const vector<double> &all_attack_plus, const BattlePosition &battle_position,
	const bool &is_special_attack, const double &multiple){
	auto other_side = kBattleSize - turn_player - 1;
	// 旗艦相手への攻撃に限り、「かばい」が確率的に発生する
	ProtectOracle(other_side, enemy_index);
	const auto &friend_side = fleet_[turn_player];
	const auto &enemy_side  = fleet_[other_side];
	const auto &hunter_kammusu = friend_side.GetUnit()[friend_index[0]][friend_index[1]];
	const auto &target_kammusu = enemy_side.GetUnit()[enemy_index[0]][enemy_index[1]];
	// 攻撃の命中率を計算する
	double hit_prob = CalcHitProb(friend_side.GetFormation(), enemy_side.GetFormation(), hunter_kammusu, target_kammusu, battle_phase);
	// 対潜攻撃かどうかを判断する
	auto is_target_submarine = target_kammusu.IsSubmarine();
	if (is_target_submarine && battle_phase != kBattlePhaseGun
		&& battle_phase != kBattlePhaseNight) return 0;		//砲撃戦および夜戦以外ではそもそも対潜攻撃を行わない
	// 三式弾・WG42による対地上施設特効
	double damage = base_attack;
	if (target_kammusu.GetShipClass() == kShipClassAF) {
		bool has_aaa = false;
		auto wg_count = 0;
		for (auto &it_w : hunter_kammusu.GetWeapon()) {
			if (it_w.GetWeaponClass() == kWeaponClassAAA) has_aaa = true;
			if (it_w.GetName() == L"WG42") ++wg_count;
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
			switch (friend_side.GetFormation()) {
			case kFormationTrail:
				damage *= (is_target_submarine) ? 0.60 : 1.0;
				break;
			case kFormationSubTrail:
				damage *= (is_target_submarine) ? 0.80 : 0.8;
				break;
			case kFormationCircle:
				damage *= (is_target_submarine) ? 1.2 : 0.7;
				break;
			case kFormationEchelon:
				damage *= (is_target_submarine) ? 1.0 : 0.6;
				break;
			case kFormationAbreast:
				damage *= (is_target_submarine) ? 1.3 : 0.6;
				break;
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
			damage = int(hunter_kammusu.SpecialEffectApPlus());	//※この切り捨ては仕様です
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
	if (!rand.RandBool(hit_prob)) {
		if (is_special_attack) damage = 0.0; else return 0;
	}
	// 夜戦における対潜攻撃は常にカスダメ(ただし開幕夜戦および連合艦隊では無視される)
	if (friend_side.GetFleetType() == kFleetTypeNormal && simulate_mode_ != kSimulateModeN && is_target_submarine && battle_phase == kBattlePhaseNight) damage = 0.0;
	// カスダメの際は相手残り耐久の6～14％を与える
	if (damage < 1.0) {
		damage = 0.06 * target_kammusu.GetHP() + 0.08 * rand.RandInt(target_kammusu.GetHP());
	}
	return int(damage);
}

// 「かばい」を確率的に発生させる
void Simulator::ProtectOracle(const int &defense_side, KammusuIndex &defense_index) {
	// 旗艦ではない場合、かばいは発生しない
	if (defense_index[1] != 0) return;
	// 水上艦は水上艦、潜水艦は潜水艦しかかばえないのでリストを作成する
	auto &attendants = fleet_[defense_side].GetUnit()[defense_index[0]];
	auto is_submarine = attendants[0].IsSubmarine();
	vector<int> block_list;
	for (auto ui = 1u; ui < attendants.size(); ++ui) {
		if (attendants[ui].IsSubmarine() == is_submarine && attendants[ui].Status() < kStatusLightDamage) {
			block_list.push_back(ui);
		}
	}
	if (block_list.size() == 0) return;
	// かばいは確率的に発生し、どの艦がかばうかも確率的に決まる
	if (rand.RandBool(0.4)) {	//とりあえず4割に設定している
		defense_index[1] = block_list[rand.RandInt(block_list.size())];
	}
	return;	//仮置き
}

//命中率を計算する
double Simulator::CalcHitProb(
	const Formation &friend_formation, const Formation &enemy_formation, const Kammusu &hunter_kammusu,
	const Kammusu &target_kammusu, const BattlePhase &battle_phase) const noexcept {
	switch (battle_phase) {
	case kBattlePhaseAir:
	case kBattlePhaseGun:
	case kBattlePhaseNight:
		{
			// 昼砲撃戦命中率
			//回避側
			double evade_sum = target_kammusu.AllEvade();	//回避値
			if (enemy_formation == kFormationEchelon || enemy_formation == kFormationAbreast) evade_sum *= 1.2;
			if (target_kammusu.Mood() == kMoodHappy) evade_sum *= 1.8;
			double evade_value;		//回避項
			if (evade_sum <= 40) {
				evade_value = 0.03 + evade_sum / 80;
			}
			else {
				evade_value = 0.03 + evade_sum / (evade_sum + 40);
			}
			//命中側
			double hit_value = 1.0 + sqrt(hunter_kammusu.GetLevel() - 1) / 50;	//練度による命中率補正
			hit_value += hunter_kammusu.AllHit() / 100;							//装備による明示的な命中率補正
			if (hunter_kammusu.Mood() == kMoodRed) hit_value /= 1.9;	//赤疲労状態だと命中率が激減する
			hit_value += hunter_kammusu.GetLuck() * 0.0015;				//命中率の運補正
			hit_value += hunter_kammusu.FitGunHitPlus();				//フィット砲補正
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
			const double hit_prob = std::min(hit_value - evade_value, 0.97);
			return hit_prob;
		}
	break;
	case kBattlePhaseFirstTorpedo:
	case kBattlePhaseTorpedo:
		{
			// 雷撃戦命中率
			//命中側
			double hit_value = 0.9272;	//命中項
			hit_value += 0.02178 * sqrt(hunter_kammusu.GetLevel() - 1);
			hit_value += 0.001518 * hunter_kammusu.AllTorpedo(false);
			hit_value += 0.000540 * hunter_kammusu.GetTorpedo();
			hit_value += 0.009017 * hunter_kammusu.AllHit();
			auto &hunter_weapon = hunter_kammusu.GetWeapon();
			for (auto i : {0, 1}) if (hunter_weapon[i].GetWeaponClass() == kWeaponClassTorpedo) hit_value += 0.02014 * sqrt(hunter_weapon[i].GetLevel());
			hit_value += 0.001463 * hunter_kammusu.GetLuck();
			//回避側
			double a;
			switch (enemy_formation) {
			case kFormationTrail:
				a = 37.40;
				break;
			case kFormationSubTrail:
				a = 37.16;
				break;
			case kFormationCircle:
				a = 32.77;
				break;
			default:
				a = 37.40;
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
			double hit_prob = hit_value - evade_value;
			if (hit_prob > 0.9691) hit_prob = 0.9691 + sqrt(hit_prob - 0.9691);
			return hit_prob;
		}
	break;
	}
	return 0.0;
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
