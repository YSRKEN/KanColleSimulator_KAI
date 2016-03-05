#pragma once

#include "weapon.hpp"
#include <iostream>
#include <cstdint>
#include "random.hpp"
class WeaponDB;
enum TorpedoTurn : std::uint8_t;

// 艦種(厳密な綴りはShip Classificationsである)
// ただし、浮遊要塞・護衛要塞・泊地棲鬼/姫・南方棲鬼は「重巡洋艦」、
// 南方棲戦鬼は「航空巡洋艦」、装甲空母鬼/姫・戦艦レ級は「航空戦艦」、
// 秋津洲は「水上機母艦」カテゴリに入れている
enum ShipClass {
	kShipClassPT = 1, kShipClassDD, kShipClassCL, kShipClassCLT,				//重雷装巡洋艦まで
	kShipClassCA, kShipClassCAV, kShipClassCVL, kShipClassCC, kShipClassBB,		//戦艦まで
	kShipClassBBV, kShipClassCV, kShipClassAF, kShipClassSS, kShipClassSSV,		//潜水空母まで
	kShipClassLST, kShipClassAV, kShipClassLHA, kShipClassACV, kShipClassAR,	//工作艦まで
	kShipClassAS, kShipClassCP, kShipClassAO									//給油艦まで
};
const wstring kShipClassStr[] = { L"", L"魚雷艇", L"駆逐艦", L"軽巡洋艦", L"重雷装巡洋艦",
L"重巡洋艦", L"航空巡洋艦", L"軽空母", L"巡洋戦艦", L"戦艦", L"航空戦艦", L"正規空母",
L"陸上型", L"潜水艦", L"潜水空母", L"輸送艦", L"水上機母艦", L"揚陸艦", L"装甲空母",
L"工作艦", L"潜水母艦", L"練習巡洋艦", L"給油艦"
};

// 速力
enum Speed { kSpeedNone, kSpeedLow, kSpeedHigh };
const wstring kSpeedStr[] = { L"無", L"低速", L"高速" };

// ステータス(無傷・小破・中破・大破・撃沈)
enum Status {kStatusNoDamage, kStatusVeryLightDamage, kStatusLightDamage, kStatusMiddleDamage, kStatusHeavyDamage, kStatusLost};

// 艦娘の疲労度(キラキラ状態・通常状態・橙疲労状態・赤疲労状態)
enum Mood { kMoodHappy, kMoodNormal, kMoodOrange, kMoodRed };

// 昼戦における攻撃種別(砲撃・空撃・爆雷攻撃)
enum DayFireType { kDayFireGun, kDayFireAir, kDayFireChage };

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
	SharedRand rand_;
	// 変更するもの
	int hp_;					//現耐久
	vector<int> airs_;			//現搭載数
	vector<Weapon> weapons_;	//現装備
	int cond_;					//cond値
	int ammo_;					//残弾薬割合
	int fuel_;					//残燃料割合
	// 内部用メンバ関数
	bool IsAntiSubDayPlane() const noexcept;	//対潜判定(空母用)
	bool IsAntiSubDayWater() const noexcept;	//対潜判定(航戦用)
	bool HasWeaponClass(const WeaponClass& wc) const noexcept;	// 指定の艦載機を保有していた場合はtrue
public:
	struct DependOnLv;
	// コンストラクタ
	Kammusu();
	Kammusu(
		const int id, wstring name, const ShipClass shipclass, const int max_hp, const int defense,
		const int attack, const int torpedo, const int anti_air, const int luck, const Speed speed,
		const Range range, const int slots, vector<int> max_airs, const int evade, const int anti_sub,
		const int search, vector<int> first_weapons, const bool kammusu_flg, const int level, const SharedRand& rand = {}
	);
	Kammusu(
		const DependOnLv info, const int id, wstring name, const ShipClass shipclass, 
		const int luck, const Speed speed, const Range range, const int slots, vector<int> max_airs, 
		vector<int> first_weapons, const bool kammusu_flg, const SharedRand& rand = {}
	);
	void SetRandGenerator(const SharedRand& rand);
	// getter
	int GetID() const noexcept;
	wstring GetName() const;
	ShipClass GetShipClass() const noexcept;
	int GetMaxHP() const noexcept;
	int GetTorpedo() const noexcept;
	int GetLuck() const noexcept;
	int GetSlots() const noexcept;
	int GetEvade() const noexcept;
	int GetAntiSub() const noexcept;
	int GetSearch() const noexcept;
	bool IsKammusu() const noexcept;
	int GetLevel() const noexcept;
	int GetHP() const noexcept;
	vector<int>& GetAir() noexcept;
	const vector<int>& GetAir() const noexcept;
	vector<Weapon>& GetWeapon() noexcept;
	const vector<Weapon>& GetWeapon() const noexcept;
	int GetAmmo() const noexcept;
	// setter
	void SetMaxHP(const int max_hp) noexcept;
	void SetLuck(const int luck) noexcept;
	void SetEvade(const int evade) noexcept;
	void SetAntiSub(const int anti_sub) noexcept;
	void SetSearch(const int search) noexcept;
	void SetLevel(const int level) noexcept;
	void SetHP(const int hp) noexcept;
	void SetWeapon(const int index, const Weapon &weapon);
	void SetCond(const int cond) noexcept;
	// その他
	void Put() const;						//中身を表示する
	void ChangeCond(const int ) noexcept;	//cond値を変化させる
	wstring GetNameLv() const;		//簡易的な名称を返す
	Kammusu Reset();				//変更可な部分をリセットする
	Kammusu Reset(const WeaponDB&);	//変更可な部分をリセットする(初期装備)
	int AacType() const noexcept;					//対空カットインの種類を判別する
	double AacProb(const int&) const noexcept;		//対空カットインの発動確率を計算する
	double AllAntiAir() const noexcept;				//加重対空値を計算する
	Status Status() const noexcept;					//ステータスを返す
	int AllEvade() const noexcept;					//総回避を返す
	Mood Mood() const noexcept;						//疲労度を返す
	int AllHit() const noexcept;					//総命中を返す
	double FitGunHitPlus() const noexcept;			//フィット砲補正
	int AllTorpedo(const bool&) const noexcept;		//総雷装を返す
	double FitGunAttackPlus() const noexcept;		//軽巡軽量砲補正
	double SpecialEffectApPlus() const noexcept;	//徹甲弾補正
	double CL2ProbPlus() const noexcept;			//熟練艦載機によるCL2率上昇
	double CL2AttackPlus() const noexcept;			//熟練艦載機によるダメージ補正
	int AllDefense() const noexcept;				//総装甲を返す
	Range MaxRange() const noexcept;				//射程を返す
	int DayAttack(const DayFireType, const bool) const noexcept;	//昼戦火力を返す
	void MinusHP(const int&, const bool&);			//ダメージを与える、ctorもしくはSetRandGenerator経由で乱数エンジンを渡している必要がある
	void ConsumeMaterial() noexcept;				//弾薬・燃料を減少させる
	bool HasAir() const noexcept;					//艦載機を保有していた場合はtrue
	bool HasAirFight() const noexcept;				//航空戦に参加する艦載機を保有していた場合はtrue
	bool HasAirTrailer() const noexcept;			//触接に参加する艦載機を保有していた場合はtrue
	bool HasAirBomb() const noexcept;				//艦爆を保有していた場合はtrue
	bool HasAirAttack() const noexcept;				// 昼戦に参加可能な場合はtrue
	bool IsSubmarine() const noexcept;				//潜水艦系ならtrue
	bool Include(const wstring& wstr) const noexcept;	//名前に特定の文字が含まれていればtrue
	bool Include(const wchar_t* wstr) const noexcept;	//名前に特定の文字が含まれていればtrue
	bool IncludeAnyOf(std::initializer_list<const wchar_t*> strings)  const;//いずれか一つが含まれていたらtrue
	bool HasAntiSubSynergy() const noexcept;				//対潜シナジーを持っていたらtrue
	bool IsSpecialEffectAP() const noexcept;				//徹甲弾補正を食らう側ならtrue
	bool HasAirPss() const noexcept;						//彩雲を保有していた場合はtrue
	bool IsFireTorpedo(const TorpedoTurn&) const noexcept;	//魚雷を発射できればtrue
	bool IsMoveGun() const noexcept;						//砲撃戦で行動可能な艦ならtrue
	bool IsFireGun() const noexcept;						//砲撃戦で攻撃可能な艦ならtrue
	bool IsAntiSubDay() const noexcept;						//昼戦で対潜可能な艦ならtrue
	bool IsFireGunPlane() const noexcept;					//空撃可能ならtrue
	friend std::ostream& operator<<(std::ostream& os, const Kammusu& conf);
	friend std::wostream& operator<<(std::wostream& os, const Kammusu& conf);
};
std::ostream& operator<<(std::ostream& os, const Kammusu& conf);
std::wostream& operator<<(std::wostream& os, const Kammusu& conf);

struct Kammusu::DependOnLv {
	int max_hp;					//最大耐久
	int defense;					//装甲
	int attack;					//火力
	int torpedo;					//雷撃
	int anti_air;					//対空
	int evade;						//回避
	int anti_sub;					//対潜
	int search;					//索敵
	int level;						//レベル(練度)
};
namespace detail {
	struct ToSpeed_helper {};
	// 文字列を速力に変換する
	inline Speed operator| (const string& str, ToSpeed_helper) {
		switch (str | to_i()) {
		case 10: return kSpeedHigh;
		case 5:  return kSpeedLow;
		default: return kSpeedNone;
		}
	}
}
constexpr detail::ToSpeed_helper ToSpeed() { return{}; }
