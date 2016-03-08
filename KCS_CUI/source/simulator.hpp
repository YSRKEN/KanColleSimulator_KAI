#ifndef KCS_KAI_SIMULATOR_H_
#define KCS_KAI_SIMULATOR_H_

#include <cstdint>
#include "kammusu.hpp"
#include <array>
// 制空状態(制空権確保・航空優勢・制空拮抗・航空劣勢・制空権喪失)
enum AirWarStatus { kAirWarStatusBest, kAirWarStatusGood, kAirWarStatusNormal, kAirWarStatusBad, kAirWarStatusWorst};

// 戦闘フェーズ(航空戦・開幕雷撃・砲撃戦・雷撃戦・夜戦)
enum BattlePhase{ kBattlePhaseAir, kBattlePhaseFirstTorpedo, kBattlePhaseGun, kBattlePhaseTorpedo, kBattlePhaseNight };

// 陣形(同航戦・反航戦・丁字有利・丁字不利)
enum BattlePosition{ kBattlePositionSame, kBattlePositionReverse, kBattlePositionGoodT, kBattlePositionBadT };

// 戦闘モード(昼戦＋夜戦、昼戦のみ、開幕夜戦)
enum SimulateMode { kSimulateModeDN, kSimulateModeD, kSimulateModeN };

// 雷撃戦の巡目(開幕および雷撃戦)
enum TorpedoTurn : std::uint8_t { kTorpedoFirst, kTorpedoSecond };

// 砲撃戦の巡目(1巡目および2巡目)
enum FireTurn { kFireFirst , kFireSecond };

class Fleet;
#include "result.hpp"
#include "random.hpp"
class Simulator {
	vector<Fleet> fleet_;			//シミュレーションに使用する艦隊
	Result result_;					//シミュレーション結果を保存するクラス
	SharedRand rand;				//シミュレーションに使用する乱数生成器
	SimulateMode simulate_mode_;	//シミュレーションにおける戦闘モード
	bitset<kBattleSize> search_result_;	//索敵結果
	tuple<AirWarStatus, vector<double>> air_war_result_;	//制空状態および触接倍率
	BattlePosition battle_position_;	//陣形
	std::array<std::array<bitset<kMaxUnitSize>, kMaxFleetSize>, kBattleSize> stopper_;	//撃沈ストッパー
	bool is_calculated;
	// 各フェーズ
	void SearchPhase();
	void AirWarPhase();
	void BattlePositionOracle() noexcept;
	void TorpedoPhase(const TorpedoTurn&);
	void FirePhase(const FireTurn&, const size_t &fleet_index = 0);
	void NightPhase();
	// 計算用メソッド(内部)
	//制空状態を判断する
	AirWarStatus JudgeAirWarStatus(const vector<int>&);
	//与えるダメージ量を計算する
	int CalcDamage(
		const BattlePhase&, const size_t, const KammusuIndex&, KammusuIndex&, const int&,
		const vector<double>&, const BattlePosition&, const bool&, const double&);
	int CalcDamage(const BattlePhase&, const size_t, const KammusuIndex&, KammusuIndex&, const int&, const bool&, const double&);
	//「かばい」を確率的に発生させる
	void ProtectOracle(const size_t, KammusuIndex&);
	//命中率を計算する
	double CalcHitProb(const Formation, const Formation, const Kammusu&, const Kammusu&, const BattlePhase, const size_t, const size_t) const noexcept;
	// 戦闘終了を判断する
	bool IsBattleTerminate() const noexcept;
	// 昼戦での攻撃種別を判断する
	DayFireType JudgeDayFireType(const size_t, const KammusuIndex&, const KammusuIndex&) const noexcept;
	// 昼戦での特殊攻撃を判断する
	tuple<bool, double> JudgeDaySpecialAttack(const size_t turn_player, const KammusuIndex &attack_index);
	// 夜戦での攻撃種別を判断する
	NightFireType JudgeNightFireType(const size_t, const KammusuIndex&) const noexcept;
	// 夜戦での特殊攻撃を判断する
	tuple<bool, double> JudgeNightSpecialAttack(const size_t turn_player, const KammusuIndex &attack_index, const bool);
public:
	// コンストラクタ
	Simulator(){}
	Simulator(const vector<Fleet> &fleet, const unsigned int seed, const SimulateMode& simulate_mode);
	SharedRand GetGenerator() noexcept;
	void Flush_Calc_Result(const vector<Fleet>& fleet);
	// 計算用メソッド
	tuple<Result, vector<Fleet>> Calc();
};

#endif //KCS_KAI_SIMULATOR_H_
