#include "base.hpp"
#include "kammusu.hpp"
#include "other.hpp"
#include "char_convert.hpp"
// コンストラクタ
Kammusu::Kammusu() 
	:	Kammusu(-1, L"なし", kShipClassDD, 0, 0, 0, 0, 0, 0, kSpeedNone, kRangeNone,
		0, { 0, 0, 0, 0, 0 }, 0, 0, 0, { -1, -1, -1, -1, -1 }, true, 1) 
{}

Kammusu::Kammusu(
	const int id, wstring name, const ShipClass shipclass, const int max_hp, const int defense,
	const int attack, const int torpedo, const int anti_air, const int luck, const Speed speed,
	const Range range, const int slots, vector<int> max_airs, const int evade, const int anti_sub,
	const int search, vector<int> first_weapons, const bool kammusu_flg, const int level) :
	id_(id), name_(move(name)), ship_class_(shipclass), max_hp_(max_hp), defense_(defense), attack_(attack),
	torpedo_(torpedo), anti_air_(anti_air), luck_(luck), speed_(speed), range_(range), slots_(slots),
	max_airs_(move(max_airs)), evade_(evade), anti_sub_(anti_sub), search_(search), first_weapons_(move(first_weapons)),
	kammusu_flg_(kammusu_flg), level_(level) {}

// 中身を表示する
void Kammusu::Put() const {
	cout << *this;
}

// 簡易的な名称を返す
wstring Kammusu::GetName() const {
	return name_ + L"(Lv" + std::to_wstring(level_) + L")";
}

// 変更可な部分をリセットする(装備なし)
Kammusu Kammusu::Reset() {
	hp_ = max_hp_;
	airs_ = max_airs_;
	weapons_.resize(slots_, Weapon());
	cond_ = 49;
	ammo_ = 100;
	fuel_ = 100;
	return *this;
}
// 変更可な部分をリセットする(初期装備)
Kammusu Kammusu::Reset(const WeaponDB &weapon_db) {
	this->Reset();
	for (auto i = 0; i < slots_; ++i) {
		weapons_[i] = weapon_db.Get(first_weapons_[i]);
	}
	return *this;
}

// 艦載機を保有していた場合はtrue
bool Kammusu::HasAir() const {
	for (auto i = 0; i < slots_; ++i) {
		if (weapons_[i].IsAir() && airs_[i] > 0) return true;
	}
	return false;
}

// 航空戦に参加する艦載機を保有していた場合はtrue
bool Kammusu::HasAirFight() const {
	for (auto i = 0; i < slots_; ++i) {
		if (weapons_[i].IsAirFight() && airs_[i] > 0) return true;
	}
	return false;
}


// 触接に参加する艦載機を保有していた場合はtrue
bool Kammusu::HasAirTrailer() const {
	for (auto i = 0; i < slots_; ++i) {
		if (weapons_[i].IsAirTrailer() && airs_[i] > 0) return true;
	}
	return false;
}

std::ostream & operator<<(std::ostream & os, const Kammusu & conf)
{
	os 
		<< "艦船ID：" << conf.id_ << endl
		<< "　艦名：" << char_cvt::utf_16_to_shift_jis(conf.name_) << "　艦種：" << char_cvt::utf_16_to_shift_jis(kShipClassStr[conf.ship_class_]) << endl
		<< "　最大耐久：" << conf.max_hp_ << "　装甲：" << conf.defense_ << "　火力：" << conf.attack_ << "　雷撃：" << conf.torpedo_ << endl
		<< "　対空：" << conf.anti_air_ << "　運：" << conf.luck_ << "　速力：" << char_cvt::utf_16_to_shift_jis(kSpeedStr[conf.speed_]) << "　射程：" << char_cvt::utf_16_to_shift_jis(kRangeStr[conf.range_]) << endl
		<< "　スロット数：" << conf.slots_ << "　最大搭載数：";
	for (auto i = 0; i < conf.slots_; ++i) {
		if (i != 0) os << ",";
		os << conf.max_airs_[i];
	}
	os 
		<< "　回避：" << conf.evade_ << "　対潜：" << conf.anti_sub_ << endl
		<< "　索敵：" << conf.search_ << "　艦娘か？：" << (conf.kammusu_flg_ ? "はい" : "いいえ") << "　レベル：" << conf.level_ << "　現耐久：" << conf.hp_ << endl
		<< "　装備：";
	for (auto i = 0; i < conf.slots_; ++i) {
		if (i != 0) os << ",";
		os << char_cvt::utf_16_to_shift_jis(conf.weapons_[i].Name()) << "(" << conf.airs_[i] << ")";
	}
	os 
		<< endl
		<< "　cond値：" << conf.cond_ << "　残弾薬(％)：" << conf.ammo_ << "　残燃料(％)" << conf.fuel_ << endl;
	return os;
}

std::wostream & operator<<(std::wostream & os, const Kammusu & conf)
{
	os
		<< L"艦船ID：" << conf.id_ << endl
		<< L"　艦名：" << conf.name_ << L"　艦種：" << kShipClassStr[conf.ship_class_] << endl
		<< L"　最大耐久：" << conf.max_hp_ << L"　装甲：" << conf.defense_ << L"　火力：" << conf.attack_ << L"　雷撃：" << conf.torpedo_ << endl
		<< L"　対空：" << conf.anti_air_ << L"　運：" << conf.luck_ << L"　速力：" << kSpeedStr[conf.speed_] << L"　射程：" << kRangeStr[conf.range_] << endl
		<< L"　スロット数：" << conf.slots_ << L"　最大搭載数：";
	for (auto i = 0; i < conf.slots_; ++i) {
		if (i != 0) os << ",";
		os << conf.max_airs_[i];
	}
	os
		<< L"　回避：" << conf.evade_ << L"　対潜：" << conf.anti_sub_ << endl
		<< L"　索敵：" << conf.search_ << L"　艦娘か？：" << (conf.kammusu_flg_ ? L"はい" : L"いいえ") << L"　レベル：" << conf.level_ << L"　現耐久：" << conf.hp_ << endl
		<< L"　装備：";
	for (auto i = 0; i < conf.slots_; ++i) {
		if (i != 0) os << ",";
		os << conf.weapons_[i].Name() << L"(" << conf.airs_[i] << ")";
	}
	os
		<< endl
		<< L"　cond値：" << conf.cond_ << L"　残弾薬(％)：" << conf.ammo_ << L"　残燃料(％)" << conf.fuel_ << endl;
	return os;
}

// 文字列を速力に変換する
Speed ToSpeed(const string &str) {
	if (str == "10") return kSpeedHigh;
	if (str == "5") return kSpeedLow;
	return kSpeedNone;
}
