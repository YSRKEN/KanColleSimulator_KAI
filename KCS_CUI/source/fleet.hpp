#pragma once

#include "picojson.h"
#include "weapon.hpp"
#include "kammusu.hpp"
#include "other.hpp"
#include <iostream>
#include "char_convert.hpp"

// 艦隊の形式
enum FleetType {kFleetTypeNormal = 1, kFleetTypeCombined};
const wstring kFleetTypeStr[] = {L"通常艦隊", L"連合艦隊"};

class Fleet {
	Formation formation_;			//陣形
	vector<vector<Kammusu>> unit_;	//艦娘・深海棲艦
	int level_;						//司令部レベル
	FleetType fleet_type_;			//艦隊の形式
	void LoadJson(std::istream &file, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, char_cvt::char_enc fileenc);
public:
	// コンストラクタ
	Fleet() { formation_ = kFormationTrail; level_ = 120; fleet_type_ = kFleetTypeNormal; }
	Fleet(const string &file_name, const Formation &formation, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, char_cvt::char_enc fileenc = char_cvt::char_enc::utf8);
	Fleet(std::istream &file, const Formation &formation, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, char_cvt::char_enc fileenc = char_cvt::char_enc::utf8);
	void Put() const;			// 中身を表示する
	double SearchValue() const;	//索敵値を計算する
	// getter
	auto GetUnit(const size_t fi, const size_t ui) const { return unit_[fi][ui]; }
	FleetType GetFleetType() { return fleet_type_; }
	// その他
	size_t FleetSize() const { return unit_.size(); }	//「艦隊数」(通常艦隊だと1、連合艦隊だと2)
	size_t UnitSize(const size_t fi) const { return unit_[fi].size(); }	//「艦隊」における艦数
	friend std::ostream& operator<<(std::ostream& os, const Fleet& conf);
	friend std::wostream& operator<<(std::wostream& os, const Fleet& conf);
};
std::ostream& operator<<(std::ostream& os, const Fleet& conf);
std::wostream& operator<<(std::wostream& os, const Fleet& conf);
