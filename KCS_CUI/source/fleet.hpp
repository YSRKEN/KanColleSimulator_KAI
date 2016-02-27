#pragma once

#include "picojson.h"
#include "weapon.hpp"
#include "kammusu.hpp"
#include "other.hpp"

class Fleet {
	Formation formation_;
	vector<Kammusu> unit_;
public:
	// コンストラクタ
	Fleet() { formation_ = kFormationTrail; }
	Fleet(const string&, const Formation&, const WeaponDB&, const KammusuDB&);
	// 中身を表示する
	void Put() const;
};
