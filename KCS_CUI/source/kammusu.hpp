﻿#pragma once

#include "weapon.hpp"
#include <iostream>
class WeaponDB;

// 艦種(厳密な綴りはShip Classificationsである)
// ただし、浮遊要塞・護衛要塞・泊地棲鬼/姫・南方棲鬼は「重巡洋艦」、
// 南方棲戦鬼は「航空巡洋艦」、装甲空母鬼/姫・戦艦レ級は「航空戦艦」、
// 秋津洲は「水上機母艦」カテゴリに入れている
enum ShipClass {
	kShipClassPT = 1, kShipClassDD, kShipClassCL, kShipClassCLT,
	kShipClassCA, kShipClassCAV, kShipClassCVL, kShipClassCC, kShipClassBB,
	kShipClassBBV, kShipClassCV, kShipClassAF, kShipClassSS, kShipClassSSV,
	kShipClassLST, kShipClassAV, kShipClassLHA, kShipClassACV, kShipClassAR,
	kShipClassAS, kShipClassCP, kShipClassAO
};
const wstring kShipClassStr[] = { L"", L"魚雷艇", L"駆逐艦", L"軽巡洋艦", L"重雷装巡洋艦",
L"重巡洋艦", L"航空巡洋艦", L"軽空母", L"巡洋戦艦", L"戦艦", L"航空戦艦", L"正規空母",
L"陸上型", L"潜水艦", L"潜水空母", L"輸送艦", L"水上機母艦", L"揚陸艦", L"装甲空母",
L"工作艦", L"潜水母艦", L"練習巡洋艦", L"給油艦"
};

// 速力
enum Speed { kSpeedNone, kSpeedLow, kSpeedHigh };
const wstring kSpeedStr[] = { L"無", L"低速", L"高速" };

// 艦娘クラス
class Kammusu {
	// 変更しないもの
	int id_;						//艦船ID
	wstring name_;					//艦名
	ShipClass ship_class_;			//艦種
	int max_hp_;					//最大耐久
	int defense_;					//装甲
	int attack_;					//火力
	int torpedo_;					//雷撃
	int anti_air_;					//対空
	int luck_;						//運
	Speed speed_;					//速力
	Range range_;					//射程
	int slots_;						//スロット数
	vector<int> max_airs_;			//最大搭載数
	int evade_;						//回避
	int anti_sub_;					//対潜
	int search_;					//索敵
	vector<int> first_weapons_;		//初期装備
	bool kammusu_flg_;				//艦娘フラグ
	int level_;						//レベル(練度)
	// 変更するもの
	int hp_;					//現耐久
	vector<int> airs_;			//現搭載数
	vector<Weapon> weapons_;	//現装備
	int cond_;					//cond値
	int ammo_;					//残弾薬割合
	int fuel_;					//残燃料割合
public:
	// コンストラクタ
	Kammusu();
	Kammusu(
		const int id, wstring name, const ShipClass shipclass, const int max_hp, const int defense,
		const int attack, const int torpedo, const int anti_air, const int luck, const Speed speed,
		const Range range, const int slots, vector<int> max_airs, const int evade, const int anti_sub,
		const int search, vector<int> first_weapons, const bool kammusu_flg, const int level
	);
	// getter
	int GetMaxHP() const { return max_hp_; }
	int GetLuck() const { return luck_; }
	int GetSlots() const { return slots_; }
	int GetEvade() const { return evade_; }
	int GetAntiSub() const { return anti_sub_; }
	int GetSearch() const { return search_; }
	int GetHP() const { return hp_; }
	vector<Weapon> GetWeapon() const { return weapons_; }
	// setter
	void SetMaxHP(const int max_hp) { max_hp_ = max_hp; }
	void SetLuck(const int luck) { luck_ = luck; }
	void SetEvade(const int evade) { evade_ = evade; }
	void SetAntiSub(const int anti_sub) { anti_sub_ = anti_sub; }
	void SetSearch(const int search) { search_ = search; }
	void SetLevel(const int level) { level_ = level; }
	void SetWeapon(const int index, const Weapon &weapon) { weapons_[index] = weapon; }
	void SetCond(const int cond) { cond_ = cond; }
	// その他
	void Put() const;				// 中身を表示する
	wstring GetName() const;		// 簡易的な名称を返す
	Kammusu Reset();				// 変更可な部分をリセットする
	Kammusu Reset(const WeaponDB&);	// 変更可な部分をリセットする(初期装備)
	bool HasAir() const;			// 艦載機を保有していた場合はtrue
	friend std::ostream& operator<<(std::ostream& os, const Kammusu& conf);
	friend std::wostream& operator<<(std::wostream& os, const Kammusu& conf);
};
std::ostream& operator<<(std::ostream& os, const Kammusu& conf);
std::wostream& operator<<(std::wostream& os, const Kammusu& conf);

// 文字列を速力に変換する
Speed ToSpeed(const string&);
