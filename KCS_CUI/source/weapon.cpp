#include "base.hpp"
#include "weapon.hpp"
#include "char_convert.hpp"
using namespace std::string_literals;

const std::unordered_map<int, const std::wstring> weaponNameMap{
#define WEAPON(PREFIX, ID, NAME, WEAPON_CLASS, DEFENSE, ATTACK, TORPEDO, BOMB, ANTI_AIR, ANTI_SUB, HIT, EVADE, SEARCH, RANGE, POSTFIX)	\
	{ ID, L ## #NAME },
#include "slotitems.csv"
#undef WEAPON
};

constexpr WeaponClass ToWC(const cstring<wchar_t>& str) noexcept {
	// CSVにはWeaponClassに定義されていないクラスが記載されているため、見つからなかった場合はOtherとしておく。
	return FirstOrDefault(detail::weaponClassMap, str, WeaponClass::Other);
}

const std::unordered_map<int, const Weapon> Weapon::db_{
#define WEAPON(PREFIX, ID, NAME, WEAPON_CLASS, DEFENSE, ATTACK, TORPEDO, BOMB, ANTI_AIR, ANTI_SUB, HIT, EVADE, SEARCH, RANGE, POSTFIX)	\
	{ ID, { ID, weaponNameMap.at(ID), std::integral_constant<WeaponClass, ToWC(L ## WEAPON_CLASS ## _cs)>{}(),							\
			DEFENSE, ATTACK, TORPEDO, BOMB, ANTI_AIR, ANTI_SUB, HIT, EVADE, SEARCH, static_cast<Range>(RANGE), 0, 0, 0 } },
#include "slotitems.csv"
#undef WEAPON
};

// コンストラクタ
Weapon::Weapon() noexcept : Weapon(-1, {}, WC("その他"), 0, 0, 0, 0, 0, 0, 0, 0, 0, kRangeNone, 0, 0, 0) {}
Weapon::Weapon(
	const int id, const std::wstring& name, const WeaponClass weapon_class, const int defense,
	const int attack, const int torpedo, const int bomb, const int anti_air, const int anti_sub,
	const int hit, const int evade, const int search, const Range range, const int level, const int level_detail, const int air) noexcept :
	id_(id), name_(name), weapon_class_(weapon_class), defense_(defense), attack_(attack),
	torpedo_(torpedo), bomb_(bomb), anti_air_(anti_air), anti_sub_(anti_sub), hit_(hit),
	evade_(evade), search_(search), range_(range), level_(level), level_detail_(level_detail), air_(air) {
	AntiAirBonus_();	//キャッシュする
}

// getter
int Weapon::GetID() const noexcept { return id_; }
const std::wstring & Weapon::GetName() const noexcept { return name_.get(); }
WeaponClass Weapon::GetWeaponClass() const noexcept { return weapon_class_; }
int Weapon::GetDefense() const noexcept { return defense_; }
int Weapon::GetAttack() const noexcept { return attack_; }
int Weapon::GetTorpedo() const noexcept { return torpedo_; }
int Weapon::GetBomb() const noexcept { return bomb_; }
int Weapon::GetAntiAir() const noexcept { return anti_air_; }
int Weapon::GetAntiSub() const noexcept { return anti_sub_; }
int Weapon::GetHit() const noexcept { return hit_; }
int Weapon::GetEvade() const noexcept { return evade_; }
int Weapon::GetSearch() const noexcept { return search_; }
Range Weapon::GetRange() const noexcept { return range_; }
int Weapon::GetLevel() const noexcept { return level_; }
int Weapon::GetAir() const noexcept { return air_; }
// setter
void Weapon::SetLevel(const int level) { level_ = level; }
void Weapon::SetLevelDetail(const int level_detail) { level_detail_ = level_detail; }
void Weapon::SetAir(const int air) { air_ = air; }

// 中身を表示する
void Weapon::Put() const {
	cout << *this;
}

// 制空値を計算する
int Weapon::AntiAirScore(const int &airs) const noexcept {
	static const double kBonusPF[] = { 0,0,2,5,9,14,14,22 }, kBonusWB[] = { 0,0,1,1,1,3,3,6 };
	double anti_air_score = anti_air_ * sqrt(airs) + sqrt(1.0 * level_detail_ / 10);
	if (AnyOf(WC("艦上戦闘機"))) {
		anti_air_score += kBonusPF[level_];
	}
	else if (AnyOf(WC("水上爆撃機"))) {
		anti_air_score += kBonusWB[level_];
	}
	return int(anti_air_score);
}

// 艦隊防空ボーナスを計算する
void Weapon::AntiAirBonus_() noexcept {
	auto scale = (IsHAG() || AnyOf(WID("91式高射装置"), WID("94式高射装置")) ? 0.35 : AnyOf(WC("小型電探") | WC("大型電探")) ? 0.4 : AnyOf(WC("対空強化弾")) ? 0.6 : 0.2);
	anti_air_bonus_ = scale * GetAntiAir();
}
double Weapon::AntiAirBonus() const noexcept { return anti_air_bonus_; }

// 高角砲ならtrue
bool Weapon::IsHAG() const noexcept {
//	return (name_.find(L"高角砲") != wstring::npos);
	return AnyOf(WID("10cm連装高角砲"), WID("12.7cm連装高角砲"), WID("12.7cm単装高角砲"), WID("8cm高角砲"), WID("10cm連装高角砲(砲架)"), WID("12.7cm連装高角砲(後期型)"), WID("10cm連装高角砲+高射装置"), WID("12.7cm高角砲+高射装置"), WID("90mm単装高角砲"), WID("3inch単装高角砲"));
}

std::ostream & operator<<(std::ostream & os, const Weapon & conf)
{
	os
		<< "装備ID：" << conf.id_ << endl
		<< "　装備名：" << (conf.id_ == -1 ? "なし" : char_cvt::utf_16_to_shift_jis(conf.GetName())) << "　種別：" << conf.weapon_class_ << endl
		<< "　装甲：" << conf.defense_ << "　火力：" << conf.attack_ << "　雷撃：" << conf.torpedo_ << "　爆装：" << conf.bomb_ << endl
		<< "　対空：" << conf.anti_air_ << "　対潜：" << conf.anti_sub_ << "　命中：" << conf.hit_ << "　回避：" << conf.evade_ << endl
		<< "　索敵：" << conf.search_ << "　射程：" << char_cvt::utf_16_to_shift_jis(kRangeStr[conf.range_]) << "　改修/熟練：" << conf.level_ << endl;
	return os;
}

std::wostream & operator<<(std::wostream & os, const Weapon & conf)
{
	os
		<< L"装備ID：" << conf.id_ << endl
		<< L"　装備名：" << (conf.id_ == -1 ? L"なし" : conf.GetName()) << L"　種別：" << conf.weapon_class_ << endl
		<< L"　装甲：" << conf.defense_ << L"　火力：" << conf.attack_ << L"　雷撃：" << conf.torpedo_ << L"　爆装：" << conf.bomb_ << endl
		<< L"　対空：" << conf.anti_air_ << L"　対潜：" << conf.anti_sub_ << L"　命中：" << conf.hit_ << L"　回避：" << conf.evade_ << endl
		<< L"　索敵：" << conf.search_ << L"　射程：" << kRangeStr[conf.range_] << L"　改修/熟練：" << conf.level_ << endl;
	return os;
}

// 外部熟練度(Simple)を内部熟練度(Detail)に変換する
int ConvertStoD(const int &level) {
	return limit(level * 15 - 5, 0, 100);
}

// 内部熟練度を外部熟練度に変換する
int ConvertDtoS(const int &level_detail) {
	return limit((level_detail + 5) / 15, 0, 7);
}
