#pragma once

#include "picojson.h"
#include "weapon.hpp"
#include "kammusu.hpp"
#include "other.hpp"
#include "char_convert.hpp"
#include "simulator.hpp"
#include "random.hpp"
// 艦隊の形式
enum FleetType {kFleetTypeNormal = 1, kFleetTypeCombined};
const wstring kFleetTypeStr[] = {L"通常艦隊", L"連合艦隊"};

class Fleet {
	Formation formation_;			//陣形
	vector<vector<Kammusu>> unit_;	//艦娘・深海棲艦
	int level_;						//司令部レベル
	FleetType fleet_type_;			//艦隊の形式
	SharedRand rand_;
	void LoadJson(std::istream &file, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, char_cvt::char_enc fileenc);
public:
	// コンストラクタ
	Fleet() { formation_ = kFormationTrail; level_ = 120; fleet_type_ = kFleetTypeNormal; }
	Fleet(const string &file_name, const Formation &formation, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, const SharedRand& rand = {}, char_cvt::char_enc fileenc = char_cvt::char_enc::utf8);
	Fleet(std::istream &file, const Formation &formation, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, const SharedRand& rand = {}, char_cvt::char_enc fileenc = char_cvt::char_enc::utf8);
	// setter
	void SetRandGenerator(const SharedRand& rand);
	// getter
	auto GetUnit() const { return unit_; }
	FleetType GetFleetType() noexcept{ return fleet_type_; }
	// その他
	size_t FleetSize() const noexcept { return unit_.size(); }	//「艦隊数」(通常艦隊だと1、連合艦隊だと2)
	size_t UnitSize(const size_t fi) const noexcept { return unit_[fi].size(); }	//「艦隊」における艦数
	void Put() const;					//中身を表示する
	double SearchValue() const noexcept;			//索敵値を計算する
	int AntiAirScore() const noexcept;			//制空値を計算する
	double TrailerAircraftProb(const AirWarStatus&) const;	//触接開始率を計算する
	int AntiAirBonus() const;			//艦隊対空ボーナス値を計算する
	Kammusu& RandomKammusu();			//生存艦から艦娘をランダムに指定する
	bool HasAir() const noexcept;		//艦載機をいずれかの艦が保有していた場合はtrue
	bool HasAirFight() const noexcept;	//航空戦に参加する艦載機をいずれかの艦が保有していた場合はtrue
	bool HasAirTrailer() const noexcept;	//触接に参加する艦載機をいずれかの艦が保有していた場合はtrue
	friend std::ostream& operator<<(std::ostream& os, const Fleet& conf);
	friend std::wostream& operator<<(std::wostream& os, const Fleet& conf);
};
std::ostream& operator<<(std::ostream& os, const Fleet& conf);
std::wostream& operator<<(std::wostream& os, const Fleet& conf);
