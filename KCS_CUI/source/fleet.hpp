﻿#ifndef KCS_KAI_INC_FLEET_HPP_
#define KCS_KAI_INC_FLEET_HPP_

#include "picojson.h"
#include "weapon.hpp"
#include "kammusu.hpp"
#include "other.hpp"
#include "char_convert.hpp"
#include "simulator.hpp"
#include "random.hpp"

// 艦隊の形式
enum class FleetType : std::uint8_t {Normal = 1, CombinedAir, CombinedGun, CombinedDrum};
const wstring kFleetTypeStr[] = {L"通常艦隊", L"空母機動部隊", L"水上打撃部隊", L"輸送護衛部隊" };

// 標的(第一艦隊・第二艦隊・全体)
enum TargetType { kTargetTypeFirst, kTargetTypeSecond, kTargetTypeAll };

class Fleet {
	Formation formation_;			//陣形
	vector<vector<Kammusu>> unit_;	//艦娘・深海棲艦
	int level_;						//司令部レベル
	FleetType fleet_type_;			//艦隊の形式
	SharedRand rand_;
	void LoadJson(std::istream &file, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, char_cvt::char_enc fileenc);
public:
	// コンストラクタ
	Fleet() { formation_ = kFormationTrail; level_ = 120; fleet_type_ = FleetType::Normal; }
	Fleet(const string &file_name, const Formation &formation, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, const SharedRand& rand = {}, char_cvt::char_enc fileenc = char_cvt::char_enc::utf8);
	Fleet(std::istream &file, const Formation &formation, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, const SharedRand& rand = {}, char_cvt::char_enc fileenc = char_cvt::char_enc::utf8);
	// setter
	void SetRandGenerator(const SharedRand& rand);
	// getter
	Formation GetFormation() const noexcept;
	vector<vector<Kammusu>>& GetUnit();
	const vector<vector<Kammusu>>& GetUnit() const;
	FleetType GetFleetType() const noexcept;
	// その他
	vector<Kammusu>& FirstUnit();
	const vector<Kammusu>& FirstUnit() const;
	vector<Kammusu>& SecondUnit();
	const vector<Kammusu>& SecondUnit() const;

	size_t FleetSize() const noexcept;						//「艦隊数」(通常艦隊だと1、連合艦隊だと2)
	size_t UnitSize(const size_t fi) const noexcept;	//「艦隊」における艦数
	size_t FirstIndex() const noexcept;								//第一艦隊のインデックス
	size_t SecondIndex() const noexcept;					//第二艦隊のインデックス
	void Put() const;							//中身を表示する
	void ChangeCond(const SimulateMode, const Result&) noexcept;	//cond値を変更する
	double SearchValue() const noexcept;		//索敵値を計算する
	int AntiAirScore() const noexcept;			//制空値を計算する
	double TrailerAircraftProb(const AirWarStatus&) const;	//触接開始率を計算する
	bool HasAir() const noexcept;			//艦載機をいずれかの艦が保有していた場合はtrue
	bool HasAirFight() const noexcept;		//航空戦に参加する艦載機をいずれかの艦が保有していた場合はtrue
	bool HasAirTrailer() const noexcept;	//触接に参加する艦載機をいずれかの艦が保有していた場合はtrue
	bool HasAirPss() const noexcept;		//彩雲をいずれかの艦が保有していた場合はtrue
	int AntiAirBonus() const;				//艦隊対空ボーナス値を計算する
	friend std::ostream& operator<<(std::ostream& os, const Fleet& conf);
	friend std::wostream& operator<<(std::wostream& os, const Fleet& conf);

	//ctorもしくはSetRandGenerator経由で乱数エンジンを渡している必要がある
	double TrailerAircraftPlus();			//攻撃力補正を計算する
	int AacType();							//発動する対空カットインの種類を判断する
	tuple<bool, size_t> RandomKammusu();					//生存艦から艦娘をランダムに指定する
	tuple<bool, KammusuIndex> RandomKammusuNonSS(const bool, const TargetType, const bool has_sl = false);	//水上の生存艦から艦娘をランダムに指定する
	tuple<bool, KammusuIndex> RandomKammusuSS(const size_t);												//潜水の生存艦から艦娘をランダムに指定する
	bool HasLights() const noexcept;		//探照灯や照明弾をいずれかの艦が保有していた場合はtrue
};
std::ostream& operator<<(std::ostream& os, const Fleet& conf);
std::wostream& operator<<(std::wostream& os, const Fleet& conf);

#endif //KCS_KAI_INC_FLEET_HPP_
