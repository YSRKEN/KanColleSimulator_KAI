#include "base.hpp"
#include "kammusu.hpp"
#include "other.hpp"

// コンストラクタ
Kammusu::Kammusu() :
	id_(-1), name_("なし"), shipclass_(kShipClassDD), max_hp_(0), defense_(0), attack_(0),
	torpedo_(0), anti_air_(0), luck_(0), speed_(kSpeedNone), range_(kRangeNone), slots_(0),
	max_airs_({ 0, 0, 0, 0, 0 }), evade_(0), anti_sub_(0), search_(0), first_weapons_({ -1, -1, -1, -1, -1 }),
	kammusu_flg_(true), level_(1) {}

Kammusu::Kammusu(
	const int id, const string name, const ShipClass shipclass, const int max_hp, const int defense,
	const int attack, const int torpedo, const int anti_air, const int luck, const Speed speed,
	const Range range, const int slots, const vector<int> max_airs, const int evade, const int anti_sub,
	const int search, const vector<int> first_weapons, const bool kammusu_flg, const int level) :
	id_(id), name_(name), shipclass_(shipclass), max_hp_(max_hp), defense_(defense), attack_(attack),
	torpedo_(torpedo), anti_air_(anti_air), luck_(luck), speed_(speed), range_(range), slots_(slots),
	max_airs_(max_airs), evade_(evade), anti_sub_(anti_sub), search_(search), first_weapons_(first_weapons),
	kammusu_flg_(kammusu_flg), level_(level) {}

// 中身を表示する
void Kammusu::Put() {
	cout << "艦船ID：" << id_ << "\n";
	cout << "　艦名：" << name_ << "　艦種：" << kShipClassStr[shipclass_] << "\n";
	cout << "　最大耐久：" << max_hp_ << "　装甲：" << defense_ << "　火力：" << attack_ << "　雷撃：" << torpedo_ << "\n";
	cout << "　対空：" << anti_air_ << "　運：" << luck_ << "　速力：" << kSpeedStr[speed_] << "　射程：" << kRangeStr[range_] << "\n";
	cout << "　スロット数：" << slots_ << "　最大搭載数：";
	for (auto i = 0; i < slots_; ++i) {
		if (i != 0) cout << ",";
		cout << max_airs_[i];
	}
	cout << "　回避：" << evade_ << "　対潜：" << anti_sub_ << "\n";
	cout << "　索敵：" << search_ << "　艦娘か？：" << (kammusu_flg_ ? "はい" : "いいえ") << "　レベル：" << level_ << "　現耐久：" << hp_ << "\n";
	cout << "　装備：";
	for (auto i = 0; i < slots_; ++i) {
		if (i != 0) cout << ",";
		cout << weapons_[i].Name() << "(" << airs_[i] << ")";
	}
	cout << "\n";
	cout << "　cond値：" << cond_ << "　残弾薬(％)：" << ammo_ << "　残燃料(％)" << fuel_ << "\n";
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

// 文字列を速力に変換する
Speed ToSpeed(const string &str) {
	if (str == "10") return kSpeedHigh;
	if (str == "5") return kSpeedLow;
	return kSpeedNone;
}
