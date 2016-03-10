#ifndef KCS_KAI_INC_KAMMUSU_HPP_
#define KCS_KAI_INC_KAMMUSU_HPP_

#include "weapon.hpp"
#include <iostream>
#include <cstdint>
#include <type_traits>
#include "random.hpp"
class WeaponDB;
enum class FleetType : std::uint8_t;
enum TorpedoTurn : std::uint8_t;

// 艦種(厳密な綴りはShip Classificationsである)
// ただし、浮遊要塞・護衛要塞・泊地棲鬼/姫・南方棲鬼は「重巡洋艦」、
// 南方棲戦鬼は「航空巡洋艦」、装甲空母鬼/姫・戦艦レ級は「航空戦艦」、
// 秋津洲は「水上機母艦」カテゴリに入れている
enum class ShipClass {
	PT  = 0x00000001,	// 魚雷艇
	DD  = 0x00000002,	// 駆逐艦
	CL  = 0x00000004,	// 軽巡洋艦
	CLT = 0x00000008,	// 重雷装巡洋艦
	CA  = 0x00000010,	// 重巡洋艦
	CAV = 0x00000020,	// 航空巡洋艦
	CVL = 0x00000040,	// 軽空母
	CC  = 0x00000080,	// 巡洋戦艦
	BB  = 0x00000100,	// 戦艦
	BBV = 0x00000200,	// 航空戦艦
	CV  = 0x00000400,	// 正規空母
	AF  = 0x00000800,	// 陸上型
	SS  = 0x00001000,	// 潜水艦
	SSV = 0x00002000,	// 潜水空母
	LST = 0x00004000,	// 輸送艦
	AV  = 0x00008000,	// 水上機母艦
	LHA = 0x00010000,	// 揚陸艦
	ACV = 0x00020000,	// 装甲空母
	AR  = 0x00040000,	// 工作艦
	AS  = 0x00080000,	// 潜水母艦
	CP  = 0x00100000,	// 練習巡洋艦
	AO  = 0x00200000,	// 給油艦
};
constexpr inline auto operator|(const ShipClass& l, const ShipClass& r) { return static_cast<ShipClass>(static_cast<std::underlying_type_t<ShipClass>>(l) | static_cast<std::underlying_type_t<ShipClass>>(r)); }

namespace detail {
	constexpr std::pair<cstring<wchar_t>, ShipClass> shipClassMap[] = {
		{ L"魚雷艇", ShipClass::PT },
		{ L"駆逐艦", ShipClass::DD },
		{ L"軽巡洋艦", ShipClass::CL },
		{ L"重雷装巡洋艦", ShipClass::CLT },
		{ L"重巡洋艦", ShipClass::CA },
		{ L"航空巡洋艦", ShipClass::CAV },
		{ L"軽空母", ShipClass::CVL },
		{ L"巡洋戦艦", ShipClass::CC },
		{ L"戦艦", ShipClass::BB },
		{ L"航空戦艦", ShipClass::BBV },
		{ L"正規空母", ShipClass::CV },
		{ L"陸上型", ShipClass::AF },
		{ L"潜水艦", ShipClass::SS },
		{ L"潜水空母", ShipClass::SSV },
		{ L"輸送艦", ShipClass::LST },
		{ L"水上機母艦", ShipClass::AV },
		{ L"揚陸艦", ShipClass::LHA },
		{ L"装甲空母",ShipClass::ACV },
		{ L"工作艦", ShipClass::AR },
		{ L"潜水母艦", ShipClass::AS },
		{ L"練習巡洋艦", ShipClass::CP },
		{ L"給油艦", ShipClass::AO },
	};
}
// 文字列から艦種へ変換します。
#define SC(STR) (std::integral_constant<ShipClass, Single(detail::shipClassMap, L##STR##_cs)>{}())

inline std::wstring to_wstring(const ShipClass& sc) {
	const auto itor = std::find_if(std::cbegin(detail::shipClassMap), std::cend(detail::shipClassMap), [sc](const auto& item) { return item.second == sc; });
	if (itor == std::cend(detail::shipClassMap))
		throw std::invalid_argument("bad ShipClass.");
	return itor->first.c_str();
}


// 速力
enum Speed { kSpeedNone, kSpeedLow, kSpeedHigh };
const wstring kSpeedStr[] = { L"無", L"低速", L"高速" };

// ステータス(無傷・小破・中破・大破・撃沈)
enum Status {kStatusNoDamage, kStatusVeryLightDamage, kStatusLightDamage, kStatusMiddleDamage, kStatusHeavyDamage, kStatusLost};

// 艦娘の疲労度(キラキラ状態・通常状態・橙疲労状態・赤疲労状態)
enum Mood { kMoodHappy, kMoodNormal, kMoodOrange, kMoodRed };

// 昼戦における攻撃種別(砲撃・空撃・爆雷攻撃)
enum DayFireType { kDayFireGun, kDayFireAir, kDayFireChage };

// 夜戦における攻撃種別(砲撃・爆雷攻撃)
enum NightFireType { kNightFireGun, kNightFireChage };

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
	size_t slots_;					//スロット数
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
	vector<Weapon> weapons_;	//現装備
	int cond_;					//cond値
	int ammo_;					//残弾薬割合
	int fuel_;					//残燃料割合
	// キャッシュ
	int aac_type_;				//対空カットインの種類
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
	size_t GetSlots() const noexcept;
	int GetEvade() const noexcept;
	int GetAntiSub() const noexcept;
	int GetSearch() const noexcept;
	bool IsKammusu() const noexcept;
	int GetLevel() const noexcept;
	int GetHP() const noexcept;
	vector<Weapon>& GetWeapon() noexcept;
	const vector<Weapon>& GetWeapon() const noexcept;
	int GetAmmo() const noexcept;
	int GetFuel() const noexcept;
	// setter
	void SetMaxHP(const int max_hp) noexcept;
	void SetLuck(const int luck) noexcept;
	void SetEvade(const int evade) noexcept;
	void SetAntiSub(const int anti_sub) noexcept;
	void SetSearch(const int search) noexcept;
	void SetLevel(const int level) noexcept;
	void SetHP(const int hp) noexcept;
	void SetWeapon(const size_t index, const Weapon &weapon);
	void SetCond(const int cond) noexcept;
	// その他
	void Put() const;						//中身を表示する
	void ChangeCond(const int ) noexcept;	//cond値を変化させる
	wstring GetNameLv() const;		//簡易的な名称を返す
	Kammusu Reset();				//変更可な部分をリセットする
	Kammusu Reset(const WeaponDB&);	//変更可な部分をリセットする(初期装備)
	void AacType_() noexcept;						//対空カットインの種類を判別する
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
	int DayAttack(const DayFireType, const bool, const FleetType, const size_t) const noexcept;	//昼戦火力を返す
	int NightAttack(const NightFireType, const bool) const noexcept;	//夜戦火力を返す
	void MinusHP(const int&, const bool&);			//ダメージを与える、ctorもしくはSetRandGenerator経由で乱数エンジンを渡している必要がある
	void ConsumeMaterial(const bool, const bool) noexcept;	//弾薬・燃料を減少させる
	bool HasAir() const noexcept;					//艦載機を保有していた場合はtrue
	bool HasAirFight() const noexcept;				//航空戦に参加する艦載機を保有していた場合はtrue
	bool HasAirTrailer() const noexcept;			//触接に参加する艦載機を保有していた場合はtrue
	bool HasAirBomb() const noexcept;				//艦爆を保有していた場合はtrue
	bool HasAirAttack() const noexcept;				// 昼戦に参加可能な場合はtrue
	bool IsSubmarine() const noexcept;				//潜水艦系ならtrue
	bool HasAntiSubSynergy() const noexcept;				//対潜シナジーを持っていたらtrue
	bool IsSpecialEffectAP() const noexcept;				//徹甲弾補正を食らう側ならtrue
	bool HasAirPss() const noexcept;						//彩雲を保有していた場合はtrue
	bool IsFireTorpedo(const TorpedoTurn&) const noexcept;	//魚雷を発射できればtrue
	bool IsMoveGun() const noexcept;						//砲撃戦で行動可能な艦ならtrue
	bool IsFireGun() const noexcept;						//砲撃戦で攻撃可能な艦ならtrue
	bool IsAntiSubDay() const noexcept;						//昼戦で対潜可能な艦ならtrue
	bool IsFireGunPlane() const noexcept;					//空撃可能ならtrue
	bool IsFireNight() const noexcept;						//夜戦で攻撃可能な艦ならtrue
	bool IsAntiSubNight() const noexcept;					//夜戦で対潜可能な艦ならtrue
	bool HasLights() const noexcept;						//探照灯や照明弾を保有していた場合はtrue
	// 指定のIDか判別する。
	bool AnyOf(const int test) const noexcept { return id_ == test; }
	// 指定の種別か判定する。
	bool AnyOf(const ShipClass& sc) const noexcept { return (static_cast<std::underlying_type_t<ShipClass>>(ship_class_) & static_cast<std::underlying_type_t<ShipClass>>(sc)) != 0; }
	// 指定の名前か判定する。名前は完全一致で比較する。
	template<class String, class = std::enable_if_t<std::is_same<String, std::wstring>::value>>		// 暗黙の型キャストにより非効率とならないようstd::wstringのみを受け付ける。
	bool AnyOf(const String& test) const noexcept { return std::size(name_) == std::size(test) && name_ == test; }	// 長さが一致した場合に限り文字列比較を行う。
	// 引数に指定された条件を満たすか判定する。引数はint型のID、ShipClass型の種別、std::wstring型の名前のいずれでも指定できる。名前は完全一致で比較する。
	template<class Head, class... Rest>
	bool AnyOf(Head head, Rest... rest) const noexcept { return AnyOf(head) || AnyOf(rest...); }
	template<class F>
	auto SumWeapons(F f) const { return std::accumulate(std::cbegin(weapons_), std::cend(weapons_), std::result_of_t<F(const Weapon&)>{}, [&](const auto& sum, const auto& it_w) { return sum + std::invoke(f, it_w); }); }
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

#endif //KCS_KAI_INC_KAMMUSU_HPP_
