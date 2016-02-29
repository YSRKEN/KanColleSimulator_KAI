#pragma once

// 制空状態(制空権確保・航空優勢・制空拮抗・航空劣勢・制空権喪失)
enum AirWarStatus { kAirWarStatusBest, kAirWarStatusGood, kAirWarStatusNormal, kAirWarStatusBad, kAirWarStatusWorst};

class Fleet;
#include "result.hpp"
#include "random.hpp"
class Simulator {
	vector<Fleet> fleet_;	//シミュレーションに使用する艦隊
	Result result_;			//シミュレーション結果を保存するクラス
	SharedRand rand;
	// 各フェーズ
	bitset<kBattleSize> SearchPhase();
	tuple<AirWarStatus, vector<double>> AirWarPhase(const bitset<kBattleSize>&);
	// 計算用メソッド(内部)
	AirWarStatus JudgeAirWarStatus(const bitset<kBattleSize>&, const vector<int>&);	//制空状態を判断する
public:
	// コンストラクタ
	Simulator(){}
	Simulator(const vector<Fleet> &fleet, const unsigned int seed)
		: fleet_(fleet), rand(seed)
	{}
	SharedRand GetGenerator() noexcept { return this->rand; }
	// 計算用メソッド
	Result Calc();
};
