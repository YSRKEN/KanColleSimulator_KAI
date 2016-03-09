#include "base.hpp"
#include "weapon.hpp"
#include "char_convert.hpp"
using namespace std::string_literals;
// コンストラクタ
Weapon::Weapon() noexcept : Weapon(-1, {}, WeaponClass::Other, 0, 0, 0, 0, 0, 0, 0, 0, 0, kRangeNone, 0, 0, 0) {}
Weapon::Weapon(
	const int id, wstring name, const WeaponClass weapon_class, const int defense,
	const int attack, const int torpedo, const int bomb, const int anti_air, const int anti_sub,
	const int hit, const int evade, const int search, const Range range, const int level, const int level_detail, const int air) noexcept :
	id_(id), name_(move(name)), weapon_class_(weapon_class), defense_(defense), attack_(attack),
	torpedo_(torpedo), bomb_(bomb), anti_air_(anti_air), anti_sub_(anti_sub), hit_(hit),
	evade_(evade), search_(search), range_(range), level_(level), level_detail_(level_detail), air_(air) {}

// getter
int Weapon::GetID() const noexcept { return id_; }
const std::wstring & Weapon::GetName() const noexcept { return name_; }
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

//制空値を計算する
int Weapon::AntiAirScore(const int &airs) const noexcept {
	static const double kBonusPF[] = { 0,0,2,5,9,14,14,22 }, kBonusWB[] = { 0,0,1,1,1,3,3,6 };
	double anti_air_score = anti_air_ * sqrt(airs) + sqrt(1.0 * level_detail_ / 10);
	if (AnyOf(WeaponClass::PF)) {
		anti_air_score += kBonusPF[level_];
	}
	else if (AnyOf(WeaponClass::WB)) {
		anti_air_score += kBonusWB[level_];
	}
	return int(anti_air_score);
}

// 高角砲ならtrue
bool Weapon::IsHAG() const noexcept {
	return (name_.find(L"高角砲") != wstring::npos);
//	return AnyOf(L"10cm連装高角砲"s, L"12.7cm連装高角砲"s, L"12.7cm単装高角砲"s, L"8cm高角砲"s, L"10cm連装高角砲(砲架)"s, L"12.7cm連装高角砲(後期型)"s, L"10cm連装高角砲+高射装置"s, L"12.7cm高角砲+高射装置"s, L"90mm単装高角砲"s, L"3inch単装高角砲"s);
}

std::ostream & operator<<(std::ostream & os, const Weapon & conf)
{
	os
		<< "装備ID：" << conf.id_ << endl
		<< "　装備名：" << ((conf.name_.empty()) ? "なし" : char_cvt::utf_16_to_shift_jis(conf.name_)) << "　種別：" << conf.weapon_class_ << endl
		<< "　装甲：" << conf.defense_ << "　火力：" << conf.attack_ << "　雷撃：" << conf.torpedo_ << "　爆装：" << conf.bomb_ << endl
		<< "　対空：" << conf.anti_air_ << "　対潜：" << conf.anti_sub_ << "　命中：" << conf.hit_ << "　回避：" << conf.evade_ << endl
		<< "　索敵：" << conf.search_ << "　射程：" << char_cvt::utf_16_to_shift_jis(kRangeStr[conf.range_]) << "　改修/熟練：" << conf.level_ << endl;
	return os;
}

std::wostream & operator<<(std::wostream & os, const Weapon & conf)
{
	os
		<< L"装備ID：" << conf.id_ << endl
		<< L"　装備名：" << ((conf.name_.empty()) ? L"なし" : conf.name_) << L"　種別：" << conf.weapon_class_ << endl
		<< L"　装甲：" << conf.defense_ << L"　火力：" << conf.attack_ << L"　雷撃：" << conf.torpedo_ << L"　爆装：" << conf.bomb_ << endl
		<< L"　対空：" << conf.anti_air_ << L"　対潜：" << conf.anti_sub_ << L"　命中：" << conf.hit_ << L"　回避：" << conf.evade_ << endl
		<< L"　索敵：" << conf.search_ << L"　射程：" << kRangeStr[conf.range_] << L"　改修/熟練：" << conf.level_ << endl;
	return os;
}

// 文字列を種別に変換する
WeaponClass ToWC(const string str) {
	struct {
		const char* name;
		WeaponClass value;
	} map[] = {
		{ "主砲", WeaponClass::Gun },
		{ "対艦強化弾", WeaponClass::AP },
		{ "副砲", WeaponClass::SubGun },
		{ "魚雷", WeaponClass::Torpedo },
		{ "特殊潜航艇", WeaponClass::SpecialSS },
		{ "艦上戦闘機", WeaponClass::PF },
		{ "艦上爆撃機", WeaponClass::PB },
		{ "艦上爆撃機(爆戦)", WeaponClass::PBF },
		{ "水上爆撃機", WeaponClass::WB },
		{ "艦上攻撃機", WeaponClass::PA },
		{ "艦上偵察機", WeaponClass::PS },
		{ "艦上偵察機(彩雲)", WeaponClass::PSS },
		{ "大型飛行艇", WeaponClass::DaiteiChan },
		{ "水上偵察機", WeaponClass::WS },
		{ "水上偵察機(夜偵)", WeaponClass::WSN },
		{ "対潜哨戒機", WeaponClass::ASPP },
		{ "オートジャイロ", WeaponClass::AJ },
		{ "小型電探", WeaponClass::SmallR },
		{ "大型電探", WeaponClass::LargeR },
		{ "対空機銃", WeaponClass::AAG },
		{ "対空強化弾", WeaponClass::AAA },
		{ "高射装置", WeaponClass::AAD },
		{ "爆雷", WeaponClass::DP },
		{ "ソナー", WeaponClass::Sonar },
		{ "応急修理要員", WeaponClass::DC },
		{ "探照灯", WeaponClass::SL },
		{ "照明弾", WeaponClass::LB },
		{ "艦隊司令部施設", WeaponClass::HQ },
		{ "水上艦要員", WeaponClass::SSP },
		{ "戦闘糧食", WeaponClass::CR },
		{ "洋上補給", WeaponClass::OS },
		{ "その他", WeaponClass::Other },
	};
	for (const auto& item : map)
		if (str == item.name)
			return item.value;
	return WeaponClass::Other;
}

// 外部熟練度(Simple)を内部熟練度(Detail)に変換する
int ConvertStoD(const int &level) {
	return limit(level * 15 - 5, 0, 100);
}

// 内部熟練度を外部熟練度に変換する
int ConvertDtoS(const int &level_detail) {
	return limit((level_detail + 5) / 15, 0, 7);
}
