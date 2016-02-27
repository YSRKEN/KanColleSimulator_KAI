#pragma once

#include "weapon.hpp"

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
const string kShipClassStr[] = { "", "魚雷艇", "駆逐艦", "軽巡洋艦", "重雷装巡洋艦",
"重巡洋艦", "航空巡洋艦", "軽空母", "巡洋戦艦", "戦艦", "航空戦艦", "正規空母",
"陸上型", "潜水艦", "潜水空母", "輸送艦", "水上機母艦", "揚陸艦", "装甲空母",
"工作艦", "潜水母艦", "練習巡洋艦", "給油艦"
};

// 速力
enum Speed { kSpeedNone, kSpeedLow, kSpeedHigh };
const string kSpeedStr[] = { "無", "低速", "高速" };

// 艦娘クラス
class Kammusu {
	// 変更しないもの
	int id_;						//艦船ID
	string name_;					//艦名
	ShipClass shipclass_;			//艦種
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
		const int, const string, const ShipClass, const int, const int, const int, const int,
		const int, const int, const Speed, const Range, const int, const vector<int>, const int,
		const int, const int, const vector<int>, const bool, const int);
	// getter
	int MaxHP() const { return max_hp_; }
	int Luck() const { return luck_; }
	int Evade() const { return evade_; }
	int AntiSub() const { return anti_sub_; }
	int Search() const { return search_; }
	// setter
	void SetMaxHP(const int max_hp) { max_hp_ = max_hp; }
	void SetLuck(const int luck) { luck_ = luck; }
	void SetEvade(const int evade) { evade_ = evade; }
	void SetAntiSub(const int anti_sub) { anti_sub_ = anti_sub; }
	void SetSearch(const int search) { search_ = search; }
	void SetLevel(const int level) { level_ = level; }
	// その他
	void Put();						// 中身を表示する
	Kammusu Reset();				// 変更可な部分をリセットする
	Kammusu Reset(const WeaponDB&);	// 変更可な部分をリセットする(初期装備)
};

// 文字列を速力に変換する
Speed ToSpeed(const string&);
