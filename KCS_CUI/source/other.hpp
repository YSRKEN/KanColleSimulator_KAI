#pragma once

#include "weapon.hpp"
#include "kammusu.hpp"

// 装備DB
class WeaponDB {
	unordered_map<int, Weapon> hash_;
public:
	// コンストラクタ
	WeaponDB();
	// ハッシュからデータを読みだす
	Weapon Get(const int) const;
};

// 艦娘DB
class KammusuDB {
	unordered_map<int, Kammusu> hash_lv1_;
	unordered_map<int, Kammusu> hash_lv99_;
public:
	// コンストラクタ
	KammusuDB();
	// ハッシュからデータを読みだす
	Kammusu Get(const int, const int) const;
};

// 文字列をデリミタで区切り分割する
vector<string> Split(const string, const char);

// 文字列配列を数字配列に変換する
vector<int> ToInt(const vector<string>);

// 配列をハッシュに変換する
template<typename T>
unordered_map<T, size_t> ToHash(const vector<T>&);
