#pragma once

#include "weapon.hpp"
#include "kammusu.hpp"

class Fleet;
class Result;

// 装備DB
class WeaponDB {
	unordered_map<int, Weapon> hash_;
public:
	// コンストラクタ
	WeaponDB(const char* csv_name);
	// ハッシュからデータを読みだす
	Weapon Get(const int) const;
	Weapon Get(const int id, std::nothrow_t) const noexcept;
};

// 艦娘DB
class KammusuDB {
	unordered_map<int, Kammusu> hash_lv1_;
	unordered_map<int, Kammusu> hash_lv99_;
public:
	// コンストラクタ
	KammusuDB(const char* csv_name);
	// ハッシュからデータを読みだす
	Kammusu Get(const int, const int) const;
};

// 集計結果保存用クラス
class ResultStat {
	vector<vector<vector<int>>> hp_min_, hp_max_, damage_min_, damage_max_;
	vector<vector<int>>mvp_count_, heavy_damage_count_;
	vector<vector<vector<double>>> hp_ave_, hp_sd_, damage_ave_, damage_sd_;
	int all_count_, reader_killed_count_;
public:
	// コンストラクタ：収集した結果から内部を計算で初期化する
	ResultStat(const vector<Result>&, const vector<vector<Kammusu>>&) noexcept;
	// 結果を標準出力に出力する
	void Put(const vector<Fleet>&) const noexcept;
	// 結果をファイルに出力する
	void Put(const vector<Fleet>&, const string&, const bool&) const;
};

// 文字列をデリミタで区切り分割する
//vector<string> Split(const string&, const char);

// 文字列配列を数字配列に変換する
vector<int> ToInt(const vector<string>&);

//// 配列をハッシュに変換する
//template<typename T>
//unordered_map<T, size_t> ToHash(const vector<T>&);
