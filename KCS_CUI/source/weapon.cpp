#include "base.hpp"
#include "weapon.hpp"

// コンストラクタ
Weapon::Weapon() :
	id_(-1), name_("なし"), weapon_class_(kWeaponClassOther), defense_(0), attack_(0),
	torpedo_(0), bomb_(0), anti_air_(0), anti_sub_(0), hit_(0),
	evade_(0), search_(0), range_(kRangeNone), level_(0) {}
Weapon::Weapon(
	const int id, const string name, const WeaponClass weapon_class, const int defense,
	const int attack, const int torpedo, const int bomb, const int anti_air, const int anti_sub,
	const int hit, const int evade, const int search, const Range range, const int level):
	id_(id), name_(name), weapon_class_(weapon_class), defense_(defense), attack_(attack),
	torpedo_(torpedo), bomb_(bomb), anti_air_(anti_air), anti_sub_(anti_sub), hit_(hit),
	evade_(evade), search_(search), range_(range), level_(level) {}

// 中身を表示する
void Weapon::Put() {
	cout << "装備ID：" << id_ << "\n";
	cout << "　装備名：" << name_ << "　種別：" << weapon_class_ << "\n";
	cout << "　装甲：" << defense_ << "　火力：" << attack_ << "　雷撃：" << torpedo_ << "　爆装：" << bomb_ << "\n";
	cout << "　対空：" << anti_air_ << "　対潜：" << anti_sub_ << "　命中：" << hit_ << "　回避：" << evade_ << "\n";
	cout << "　索敵：" << search_ << "　射程：" << kRangeStr[range_] << "　改修/熟練：" << level_ << "\n";
}

// 文字列を種別に変換する
WeaponClass ToWC(const string str) {
	for (auto i = 0u; i < kWeaponClassStr.size(); ++i) {
		if (str == kWeaponClassStr[i]) return static_cast<WeaponClass>(i);
	}
	return kWeaponClassOther;
}
