#pragma once

#include "picojson.h"
#include "weapon.hpp"
#include "kammusu.hpp"
#include "other.hpp"

using picojson::object;
using picojson::value;

// 艦隊の形式
enum FleetType {kFleetTypeNormal = 1, kFleetTypeCombined};
const string kFleetTypeStr[] = {"通常艦隊", "連合艦隊"};

class Fleet {
	Formation formation_;			//陣形
	vector<vector<Kammusu>> unit_;	//艦娘・深海棲艦
	int level_;						//司令部レベル
	FleetType fleet_type_;			//艦隊の形式
public:
	// コンストラクタ
	Fleet() { formation_ = kFormationTrail; level_ = 120; fleet_type_ = kFleetTypeNormal; }
	Fleet(const string&, const Formation&, const WeaponDB&, const KammusuDB&);
	// getter
	FleetType GetFleetType() { return fleet_type_; }
	auto GetUnit(const size_t fi, const size_t ui) const { return unit_[fi][ui]; }
	// その他
	size_t FleetSize() const { return unit_.size(); }	//「艦隊数」(通常艦隊だと1、連合艦隊だと2)
	size_t UnitSize(const size_t fi) const { return unit_[fi].size(); }	//「艦隊」における艦数
	void Put() const;	// 中身を表示する
};
