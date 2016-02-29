#include "base.hpp"
#include "weapon.hpp"
#include "char_convert.hpp"
// コンストラクタ
Weapon::Weapon() : Weapon(-1, L"なし", kWeaponClassOther, 0, 0, 0, 0, 0, 0, 0, 0, 0, kRangeNone, 0, 0){}
Weapon::Weapon(
	const int id, wstring name, const WeaponClass weapon_class, const int defense,
	const int attack, const int torpedo, const int bomb, const int anti_air, const int anti_sub,
	const int hit, const int evade, const int search, const Range range, const int level, const int level_detail) noexcept :
	id_(id), name_(move(name)), weapon_class_(weapon_class), defense_(defense), attack_(attack),
	torpedo_(torpedo), bomb_(bomb), anti_air_(anti_air), anti_sub_(anti_sub), hit_(hit),
	evade_(evade), search_(search), wrange_(range), level_(level), level_detail_(level_detail) {}

// 中身を表示する
void Weapon::Put() const {
	cout << *this;
}

//制空値を計算する
int Weapon::AntiAirScore(const int &airs) const noexcept {
	static const double kBonusPF[] = { 0,0,2,5,9,14,14,22 }, kBonusWB[] = { 0,0,1,1,1,3,3,6 };
	double anti_air_score = anti_air_ * sqrt(airs) + sqrt(1.0 * level_detail_ / 10);
	if (weapon_class_ == kWeaponClassPF) {
		anti_air_score += kBonusPF[level_];
	}
	else if (weapon_class_ == kWeaponClassWB) {
		anti_air_score += kBonusWB[level_];
	}
	return int(anti_air_score);
}

// (熟練度が存在する)艦載機ならtrue
bool Weapon::IsAir() const noexcept {
	switch (weapon_class_) {
	case kWeaponClassPF:
	case kWeaponClassPBF:
	case kWeaponClassPB:
	case kWeaponClassWB:
	case kWeaponClassPA:
	case kWeaponClassPS:
	case kWeaponClassPSS:
	case kWeaponClassDaiteiChan:
	case kWeaponClassWS:
	case kWeaponClassWSN:
		return true;
	default:
		return false;
	}
}

// 航空戦に参加する艦載機ならtrue
bool Weapon::IsAirFight() const noexcept {
	switch (weapon_class_) {
	case kWeaponClassPF:
	case kWeaponClassPBF:
	case kWeaponClassPB:
	case kWeaponClassWB:
	case kWeaponClassPA:
		return true;
	default:
		return false;
	}
}

// 触接に参加する艦載機ならtrue
bool Weapon::IsAirTrailer() const noexcept {
	switch (weapon_class_) {
	case kWeaponClassPF:
	case kWeaponClassPA:
	case kWeaponClassPS:
	case kWeaponClassPSS:
	case kWeaponClassDaiteiChan:
	case kWeaponClassWS:
	case kWeaponClassWSN:
		return true;
	default:
		return false;
	}
}

// 開幕爆撃に参加する艦載機ならtrue
bool Weapon::IsAirBomb() const noexcept {
	switch (weapon_class_) {
	case kWeaponClassPBF:
	case kWeaponClassPB:
	case kWeaponClassWB:
	case kWeaponClassPA:
		return true;
	default:
		return false;
	}
}

std::ostream & operator<<(std::ostream & os, const Weapon & conf)
{
	os
		<< "装備ID：" << conf.id_ << endl
		<< "　装備名：" << char_cvt::utf_16_to_shift_jis(conf.name_) << "　種別：" << conf.weapon_class_ << endl
		<< "　装甲：" << conf.defense_ << "　火力：" << conf.attack_ << "　雷撃：" << conf.torpedo_ << "　爆装：" << conf.bomb_ << endl
		<< "　対空：" << conf.anti_air_ << "　対潜：" << conf.anti_sub_ << "　命中：" << conf.hit_ << "　回避：" << conf.evade_ << endl
		<< "　索敵：" << conf.search_ << "　射程：" << char_cvt::utf_16_to_shift_jis(kRangeStr[conf.wrange_]) << "　改修/熟練：" << conf.level_ << endl;
	return os;
}

std::wostream & operator<<(std::wostream & os, const Weapon & conf)
{
	os
		<< L"装備ID：" << conf.id_ << endl
		<< L"　装備名：" << conf.name_ << L"　種別：" << conf.weapon_class_ << endl
		<< L"　装甲：" << conf.defense_ << L"　火力：" << conf.attack_ << L"　雷撃：" << conf.torpedo_ << L"　爆装：" << conf.bomb_ << endl
		<< L"　対空：" << conf.anti_air_ << L"　対潜：" << conf.anti_sub_ << L"　命中：" << conf.hit_ << L"　回避：" << conf.evade_ << endl
		<< L"　索敵：" << conf.search_ << L"　射程：" << kRangeStr[conf.wrange_] << L"　改修/熟練：" << conf.level_ << endl;
	return os;
}

// 文字列を種別に変換する
WeaponClass ToWC(const string str) {
	for (auto i = 0u; i < kWeaponClassStr.size(); ++i) {
		if (str == kWeaponClassStr[i]) return static_cast<WeaponClass>(i);
	}
	return kWeaponClassOther;
}

// 外部熟練度(Simple)を内部熟練度(Detail)に変換する
int ConvertStoD(const int &level) {
	return limit(level * 15 - 5, 0, 100);
}

// 内部熟練度を外部熟練度に変換する
int ConvertDtoS(const int &level_detail) {
	return limit((level_detail + 5) / 15, 0, 7);
}
