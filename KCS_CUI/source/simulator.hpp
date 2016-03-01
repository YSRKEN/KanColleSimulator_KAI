#pragma once

// 制空状態(制空権確保・航空優勢・制空拮抗・航空劣勢・制空権喪失)
enum AirWarStatus { kAirWarStatusBest, kAirWarStatusGood, kAirWarStatusNormal, kAirWarStatusBad, kAirWarStatusWorst};

// 戦闘フェーズ(航空戦・開幕雷撃・砲撃戦・雷撃戦・夜戦)
enum BattlePhase{ kBattlePhaseAir, kBattlePhaseFirstTorpedo, kBattlePhaseGun, kBattlePhaseTorpedo, kBattlePhaseNight };

// 陣形(同航戦・反航戦・丁字有利・丁字不利)
enum BattlePosition{ kBattlePositionSame, kBattlePositionReverse, kBattlePositionGoodT, kBattlePositionBadT };

class Fleet;
#include "result.hpp"

class Simulator {
	vector<Fleet> fleet_;	//シミュレーションに使用する艦隊
	Result result_;			//シミュレーション結果を保存するクラス
	// 乱数用メンバ関数
	std::mt19937 mt;										//
	std::uniform_real_distribution<double> rand;			//
	// 各フェーズ
	bitset<kBattleSize> SearchPhase();
	tuple<AirWarStatus, vector<double>> AirWarPhase(const bitset<kBattleSize>&);
	// 計算用メソッド(内部)
	AirWarStatus JudgeAirWarStatus(const bitset<kBattleSize>&, const vector<int>&);	//制空状態を判断する
	int CalcDamage(
		const BattlePhase&, const int&, const vector<int>&, const vector<int>&,		//与えるダメージ量を計算する
		const int&, const vector<double>&, const BattlePosition&);
public:
	// コンストラクタ
	Simulator(){}
	Simulator(const vector<Fleet> &fleet, const unsigned int &seed) {
		fleet_ = fleet;
		mt = std::mt19937(seed);
		rand = std::uniform_real_distribution<double>(0.0, 1.0);
	}
	// 乱数用メンバ関数
	double RandReal() { return rand(mt); }					//[0, 1)の一様実数乱数を取り出す
	int RandInt(const int n) { return int(rand(mt) * n); }	//[0, n - 1]の一様整数乱数を取り出す
	int RandInt(const int a, const int b) { return int(rand(mt) * (b - a + 1) + a); }	//[a, b]の一様整数乱数を取り出す
	// 計算用メソッド
	Result Calc();
};
