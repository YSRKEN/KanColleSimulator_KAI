#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "base.hpp"
#include "kammusu.hpp"
#include "other.hpp"
#include "char_convert.hpp"
#include "fleet.hpp"
#include "simulator.hpp"
#include <algorithm>
using namespace std::string_literals;

static std::pair<int, int> parse(const std::wstring& str) {
	auto pos = str.find(L'.');
	if (pos == std::wstring::npos) {
		auto i = std::stoi(str);
		return{ i, i };
	}
	if (pos == 0 || pos + 1 == str.size())
		throw std::invalid_argument("empty element.");
	if (str.find(L'.', pos + 1) != std::wstring::npos)
		throw std::invalid_argument("multiple delimiters found.");
	return{ std::stoi(str.substr(0, pos)), std::stoi(str.substr(pos + 1)) };
}

static std::vector<int> split(const std::wstring& str) {
	std::vector<int> result;
	result.reserve(4);
	std::wstringstream ss{ str };
	std::wstring line;
	while (std::getline(ss, line, L'.'))
		result.push_back(std::stoi(line));
	return result;
}

static std::pair<const Kammusu, const Kammusu> parse(int id, const wchar_t* name_str, int shipclass_num, const wchar_t* max_hp_str, const wchar_t* defense_str, const wchar_t* attack_str,
	const wchar_t* torpedo_str, const wchar_t* anti_air_str, const wchar_t* luck_str, int speed_num, int range_num, int slots, const wchar_t* max_airs_str, const wchar_t* evade_str,
	const wchar_t* anti_sub_str, const wchar_t* search_str, const wchar_t* first_weapons_str, const wchar_t* kammusu_flg_str)
{
	using std::get;
	std::wstring name{ name_str };
	auto shipclass = static_cast<ShipClass>(1 << (shipclass_num - 1));
	auto max_hp = parse(max_hp_str);
	auto defense = parse(defense_str);
	auto attack = parse(attack_str);
	auto torpedo = parse(torpedo_str);
	auto anti_air = parse(anti_air_str);
	auto luck = get<0>(parse(luck_str));
	auto speed = speed_num == 10 ? kSpeedHigh : speed_num == 5 ? kSpeedLow : kSpeedNone;
	auto range = static_cast<Range>(range_num);
	auto max_airs = split(max_airs_str);
	auto evade = parse(evade_str);
	auto anti_sub = parse(anti_sub_str);
	auto search = parse(search_str);
	auto first_weapons = split(first_weapons_str);
	auto kammusu_flg = std::stoi(kammusu_flg_str) != 0;
	return{	// TODO: Lv99の艦娘はなんでLv1をセットするんだろう？
		{ id, name, shipclass, get<0>(max_hp), get<0>(defense), get<0>(attack), get<0>(torpedo), get<0>(anti_air), luck, speed, range, slots, max_airs, get<0>(evade), get<0>(anti_sub), get<0>(search), first_weapons, kammusu_flg, 0 },
		{ id, name, shipclass, get<1>(max_hp), get<1>(defense), get<1>(attack), get<1>(torpedo), get<1>(anti_air), luck, speed, range, slots, max_airs, get<1>(evade), get<1>(anti_sub), get<1>(search), first_weapons, kammusu_flg, 1 }
	};
}

const std::unordered_map<int, std::pair<const Kammusu, const Kammusu>> Kammusu::db_ = [] {
	std::unordered_map<int, std::pair<const Kammusu, const Kammusu>> db;
#define SHIP(PREFIX, ID, NAME, SHIPCLASS, MAX_HP, DEFENSE, ATTACK, TORPEDO, ANTI_AIR, LUCK, SPEED, RANGE, SLOTS, MAX_AIRS, EVADE, ANTI_SUB, SEARCH, FIRST_WEAPONS, KAMMUSU_FLG, POSTFIX)								\
	if (std::wstring(L#MAX_HP L#DEFENSE L#ATTACK L#TORPEDO L#ANTI_AIR L#LUCK L#MAX_AIRS L#EVADE L#ANTI_SUB L#SEARCH L#FIRST_WEAPONS L#KAMMUSU_FLG).find(L"null") == std::wstring::npos)									\
		db.emplace(ID, parse(ID, L#NAME, SHIPCLASS, L#MAX_HP, L#DEFENSE, L#ATTACK, L#TORPEDO, L#ANTI_AIR, L#LUCK, SPEED, RANGE, SLOTS, L#MAX_AIRS, L#EVADE, L#ANTI_SUB, L#SEARCH, L#FIRST_WEAPONS, L#KAMMUSU_FLG));
#include "ships.csv"
#undef SHIP
	return db;
}();

Kammusu Kammusu::Get(int id, int level) {
	auto& pair = db_.at(id);
	auto& lv1 = pair.first;
	auto& lv99 = pair.second;
	auto result = lv99;
	result.SetEvade(lv1.GetEvade() + (lv99.GetEvade() - lv1.GetEvade()) * level / 99);
	result.SetAntiSub(lv1.GetAntiSub() + (lv99.GetAntiSub() - lv1.GetAntiSub()) * level / 99);
	result.SetSearch(lv1.GetSearch() + (lv99.GetSearch() - lv1.GetSearch()) * level / 99);
	result.SetLevel(level);
	auto max_hp = lv1.GetMaxHP();
	if (100 <= level) {
		// ケッコンによる耐久上昇はややこしい
		max_hp += max_hp < 10 ? 3 : max_hp < 30 ? 4 : max_hp < 40 ? 5 : max_hp < 50 ? 6 : max_hp < 70 ? 7 : max_hp < 90 ? 8 : 9;
		max_hp = std::min(max_hp, lv99.GetMaxHP());
		// TODO: ケッコンによる運上昇は+3～+6までランダムなのでとりあえず+4とした
		result.SetLuck(result.GetLuck() + 4);
	}
	result.SetMaxHP(max_hp);
	return result;
}

// コンストラクタ
Kammusu::Kammusu() 
	:	Kammusu(-1, L"なし", SC("駆逐艦"), 0, 0, 0, 0, 0, 0, kSpeedNone, kRangeNone,
		0, { 0, 0, 0, 0, 0 }, 0, 0, 0, { -1, -1, -1, -1, -1 }, true, 1) 
{}

Kammusu::Kammusu(
	const int id, wstring name, const ShipClass shipclass, const int max_hp, const int defense,
	const int attack, const int torpedo, const int anti_air, const int luck, const Speed speed,
	const Range range, const int slots, vector<int> max_airs, const int evade, const int anti_sub,
	const int search, vector<int> first_weapons, const bool kammusu_flg, const int level, const SharedRand& rand) :
	id_(id), name_(move(name)), ship_class_(shipclass), max_hp_(max_hp), defense_(defense), attack_(attack),
	torpedo_(torpedo), anti_air_(anti_air), luck_(luck), speed_(speed), range_(range), slots_(slots),
	max_airs_(move(max_airs)), evade_(evade), anti_sub_(anti_sub), search_(search), first_weapons_(move(first_weapons)),
	kammusu_flg_(kammusu_flg), level_(level) , rand_(rand)
{
	this->Reset(false);
}

void Kammusu::SetRandGenerator(const SharedRand & rand) {
	this->rand_ = rand;
}

// getter
int Kammusu::GetID() const noexcept { return id_; }
wstring Kammusu::GetName() const { return name_; }
ShipClass Kammusu::GetShipClass() const noexcept { return ship_class_; }
int Kammusu::GetMaxHP() const noexcept { return max_hp_; }
int Kammusu::GetTorpedo() const noexcept { return torpedo_; }
int Kammusu::GetLuck() const noexcept { return luck_; }
size_t Kammusu::GetSlots() const noexcept { return slots_; }
int Kammusu::GetEvade() const noexcept { return evade_; }
int Kammusu::GetAntiSub() const noexcept { return anti_sub_; }
int Kammusu::GetSearch() const noexcept { return search_; }
bool Kammusu::IsKammusu() const noexcept { return kammusu_flg_; }
int Kammusu::GetLevel() const noexcept { return level_; }
int Kammusu::GetHP() const noexcept { return hp_; }
vector<Weapon>& Kammusu::GetWeapon() noexcept { return weapons_; }
const vector<Weapon>& Kammusu::GetWeapon() const noexcept { return weapons_; }
int Kammusu::GetAmmo() const noexcept { return ammo_; }
int Kammusu::GetFuel() const noexcept { return fuel_; }
// setter
void Kammusu::SetMaxHP(const int max_hp) noexcept { max_hp_ = max_hp; }
void Kammusu::SetLuck(const int luck) noexcept { luck_ = luck; }
void Kammusu::SetEvade(const int evade) noexcept { evade_ = evade; }
void Kammusu::SetAntiSub(const int anti_sub) noexcept { anti_sub_ = anti_sub; }
void Kammusu::SetSearch(const int search) noexcept { search_ = search; }
void Kammusu::SetLevel(const int level) noexcept { level_ = level; }
void Kammusu::SetHP(const int hp) noexcept { hp_ = hp; }
void Kammusu::SetWeapon(const size_t index, const Weapon & weapon) { weapons_[index] = weapon; weapons_[index].SetAir(max_airs_[index]); }
void Kammusu::SetCond(const int cond) noexcept { cond_ = cond; }

// 中身を表示する
void Kammusu::Put() const {
	cout << *this;
}

// 簡易的な名称を返す
wstring Kammusu::GetNameLv() const {
	return name_ + L"(Lv" + std::to_wstring(level_) + L")";
}

// 変更可な部分をリセットする
Kammusu Kammusu::Reset(bool load_first_weapons) {
	hp_ = max_hp_;
	weapons_ = std::vector<Weapon>(slots_, Weapon());
	cond_ = 49;
	ammo_ = 100;
	fuel_ = 100;
	if(load_first_weapons)
		for (size_t i = 0; i < slots_; ++i) {
			weapons_[i] = Weapon::Get(first_weapons_[i]);
			weapons_[i].SetAir(max_airs_[i]);
		}
	return *this;
}

// 対空カットインの種類を判別する
void Kammusu::AacType_() noexcept {
	// 各種兵装の数を数える
	//高角砲、高角砲+高射装置、高射装置、対空機銃(三式弾以外)、集中配備の数
	size_t sum_hag = 0, sum_hagX = 0, sum_aad = 0, sum_aag = 0, sum_aagX = 0;
	// 大口径主砲・水上電探・対空電探・三式弾の数
	size_t sum_gunL = 0, sum_radarW = 0, sum_radarA = 0, sum_three = 0;
	for (auto &it_w : weapons_) {
		switch (it_w.GetWeaponClass()) {
		case WC("主砲"):
			if (it_w.IsHAG()) {
				if (it_w.AnyOf(WID("10cm連装高角砲+高射装置"), WID("12.7cm高角砲+高射装置"), WID("90mm単装高角砲"))) {
					++sum_hagX;
				}
				else {
					++sum_hag;
				}
			}
			else if (it_w.GetRange() >= kRangeLong) {
				++sum_gunL;
			}
			break;
		case WC("高射装置"):
			++sum_aad;
			break;
		case WC("対空機銃"):
			if (it_w.AnyOf(WID("25mm三連装機銃 集中配備"))) {
				++sum_aagX;
			}
			else {
				++sum_aag;
			}
			break;
		case WC("対空強化弾"):
			++sum_three;
			break;
		case WC("小型電探"):
		case WC("大型電探"):
			if (it_w.AnyOf(WID("13号対空電探"), WID("21号対空電探"), WID("14号対空電探"), WID("21号対空電探改"), WID("13号対空電探改"), WID("対空レーダ― Mark.I"), WID("対空レーダ― Mark.II"), WID("深海対空レーダ―"))) {
				++sum_radarA;
			}
			else {
				++sum_radarW;
			}
		default:
			break;
		}
	}
	// 特定のパターンは先に関数化しておく
	//高角砲を持っていたらtrue
	auto has_high_angle_gun = [sum_hag, sum_hagX]() -> bool { return (sum_hag + sum_hagX >= 1); };
	//対空機銃を持っていたらtrue
	auto has_anti_aircraft_machine_gun = [sum_aag, sum_aagX]() -> bool { return (sum_aag + sum_aagX >= 1); };
	//高射装置を持っていたらtrue
	auto has_anti_aircraft_director = [sum_hagX, sum_aad]() -> bool { return (sum_hagX + sum_aad >= 1); };
	//電探を持っていたらtrue
	auto has_radar = [sum_radarW, sum_radarA]() -> bool { return (sum_radarW + sum_radarA >= 1); };
	// まず、固有カットインを判定する
	if (AnyOf(SID("秋月"), SID("秋月改"), SID("照月"), SID("照月改"), SID("初月"), SID("初月改"))) {
		/* 秋月型……ご存知防空駆逐艦。対空カットイン無しでも圧倒的な対空値により艦載機を殲滅する。
		* 二次創作界隈ではまさma氏が有名であるが、秋月型がこれ以上増えると投稿時のタイトルが長くなりすぎることから
		* 嬉しい悲鳴を上げていたとか。なお史実上では後9隻居るが、有名なのは涼月などだろう……  */
		if (sum_hag + sum_hagX >= 2 && has_radar()) aac_type_ = 1;
		if (has_high_angle_gun() && has_radar()) aac_type_ = 2;
		if (sum_hag + sum_hagX >= 2) aac_type_ = 3;
	}
	else if (AnyOf(SID("摩耶改二"))) {
		/* 摩耶改二……麻耶ではない。対空兵装により「洋上の対空要塞」(by 青島文化教材社)となったため、
		* 重巡にしては驚異的な対空値を誇る。ついでに服装もかなりプリティーに進化した(妹の鳥海も同様) */
		if (has_high_angle_gun() && sum_aagX >= 1 && sum_radarA >= 1) aac_type_ = 10;
		if (has_high_angle_gun() && sum_aagX >= 1) aac_type_ = 11;
	}
	else if (AnyOf(SID("五十鈴改二"))) {
		/* 五十鈴改二…… 名前通りLv50からの改装である。防空巡洋艦になった史実から、射程が短となり、
		* 防空力が大幅にアップした。しかし搭載数0で火力面で使いづらくなった上、対潜は装備対潜のウェイトが高いため
		* 彼女を最適解に出来る状況は限られている。また、改二なのに金レアで固有カットインがゴミクズ「だった」ことから、
		* しばしば不遇改二の代表例として挙げられていた。逆に言えば、新人向けに便利とも言えるが…… */
		if (has_high_angle_gun() && has_anti_aircraft_machine_gun() && sum_radarA >= 1) aac_type_ = 14;
		if (has_high_angle_gun() && has_anti_aircraft_machine_gun()) aac_type_ = 15;
	}
	else if (AnyOf(SID("霞改二乙"))) {
		/* 霞改二乙…… Lv88という驚異的な練度を要求するだけあり、内蔵されたギミックは特殊である。
		* まず霞改二でも積めた大発に加え、大型電探も装備可能になった(代償に艦隊司令部施設が積めなくなった)。
		* また、対空値も上昇し、固有カットインも実装された。ポスト秋月型＋アルファとも言えるだろう。
		* なお紐が霞改二と違い赤色であるが、どちらにせよランドｓゲフンゲフン */
		if (has_high_angle_gun() && has_anti_aircraft_machine_gun() && sum_radarA >= 1) aac_type_ = 16;
		if (has_high_angle_gun() && has_anti_aircraft_machine_gun()) aac_type_ = 17;
	}
	else if (AnyOf(SID("皐月改二"))) {
		/* 皐月改二…… うるう年の2/29に実装された、皐月改二における固有の対空カットイン。
		 * この調子では改二が出るたびに新型カットインが出るのではないかと一部で危惧されている。*/
		if (sum_aagX >= 1) aac_type_ = 18;
	}
	// 次に一般カットインを判定する
	if (sum_gunL >= 1 && sum_three >= 1 && has_anti_aircraft_director() && sum_radarA >= 1) aac_type_ = 4;
	if (sum_hagX >= 2 && sum_radarA >= 1) aac_type_ = 5;
	if (sum_gunL >= 1 && sum_three >= 1 && has_anti_aircraft_director()) aac_type_ = 6;
	if (has_high_angle_gun() && sum_aad >= 1 && sum_radarA >= 1) aac_type_ = 7;
	if (sum_hagX >= 1 && sum_radarA >= 1) aac_type_ = 8;
	if (has_high_angle_gun() && sum_aad >= 1) aac_type_ = 9;
	if (sum_aagX >= 1 && sum_aag >= 1 && sum_radarA >= 1) aac_type_ = 12;
	aac_type_ = 0;
}
int Kammusu::AacType() const noexcept {return aac_type_;}

// 対空カットインの発動確率を計算する
double Kammusu::AacProb(const int &aac_type) const noexcept {
	// 色々とお察しください
	/*
	| 艦娘     | 位置 | 素対空値 | 装備対空値 | 種類 | 装備                                     | 結果    | ％    | 備考            |
	|----------|------|----------|------------|------|------------------------------------------|---------|-------|-----------------|
	| 秋月     | 僚艦 | 116      | 6          | 1    | 12.7高単、12.7高単、22号改四             | 72/100  | 72.0% |                 |
	| 秋月     | 僚艦 | 116      | 24         | 1    | 秋月砲★9、秋月砲★9、13号改             | 164/206 | 79.6% | (16-991)        |
	| 秋月     | 僚艦 | 116      | 17         | 3    | 10cm、10cm、94式                         | 121/202 | 59.9% |                 |
	| 秋月     | 僚艦 | 116      | 23         | 3    | 秋月砲★9、秋月砲★9、94式★6            | 33/56   | 58.9% | (16-813)        |
	| 榛名     | 旗艦 | 92       | 18         | 4    | ダズル、三式弾、14号、91式               | 33/50   | 66.0% | (16-691)        |
	| 大淀     | 僚艦 | 74       | 28         | 5    | 秋月砲、秋月砲、14号、観測機             | 67/100  | 67.0% |                 |
	| 大淀     | 僚艦 | 74       | 29         | 5    | 秋月砲、秋月砲、14号、94式               | 65/100  | 65.0% |                 |
	| 大淀     | 僚艦 | 74       | 32         | 5    | 秋月砲、秋月砲、14号、14号               | 60/100  | 60.0% |                 |
	| 榛名     | 旗艦 | 92       | 12         | 6    | ダズル、三式弾、22号、91式               | 23/50   | 46.0% |                 |
	| 大淀     | 僚艦 | 74       | 29         | 7    | 浦風砲、浦風砲、14号、94式               | 48/100  | 48.0% |                 |
	| 摩耶     | 僚艦 | 89       | 14         | 7    | 8cm、91式、14号                          | 89/200  | 44.5% |                 |
	| 雪風     | 旗艦 | 59       | 14         | 8    | 秋月砲★9、13号改、魚雷                  | 144/279 | 51.6% | (17-10)         |
	| 能代     | 僚艦 | 72       | 8          | 8    | 秋月砲★4、FuMO                          | 48/100  | 48.0% |                 |
	| 時雨     | 旗艦 | 72       | 14         | 8    | 秋月砲★9、13号改、魚雷                  | 58/100  | 58.0% | (16-785)        |
	| 時雨     | 旗艦 | 72       | 16         | 8    | 秋月砲、13号改、主砲                     | 26/50   | 52.0% |                 |
	| 大淀     | 僚艦 | 74       | 25         | 8    | 秋月砲、10cm、14号、観測機               | 52/100  | 52.0% | (16-600)        |
	| 陽炎     | 僚艦 | 49       | 17         | 9    | 10cm、10cm、94式                         | 103/300 | 34.3% | (16-702, 17-44) |
	| 能代     | 僚艦 | 72       | 6          | 9    | 8cm、91式                                | 44/100  | 44.0% | (16-701)        |
	| 能代     | 僚艦 | 72       | 8          | 9    | 8cm、94式★2                             | 49/100  | 49.0% | (16-660)        |
	| 潮       | 僚艦 | 74       | 17         | 9    | 10cm、10cm、94式                         | 131/300 | 43.7% | (17-44)         |
	| 摩耶     | 僚艦 | 89       | 8          | 9    | 8cm、91式                                | 83/200  | 41.5% |                 |
	| 摩耶改二 | 僚艦 | 106      | 27         | 10   | 2号砲★9、90mm高★10、集中機銃、Fumo     | 108/206 | 52.4% | [22]            |
	| 摩耶改二 | 僚艦 | 106      | 22         | 11   | 2号砲★9、90mm高★10、集中機銃、観測機>> | 103/204 | 50.5% |                 |
	 */
	// とりあえず装備対空を計算する
	int weapon_anti_air = SumWeapons(&Weapon::GetAntiAir);
	//とりあえず種別によって場合分け
	switch (aac_type) {
	case 1:
		return 0.004222 * weapon_anti_air + 0.694667;
	case 3:
		return 0.594;
	case 4:
		return 0.66;
	case 5:
		return 0.64;
	case 6:
		return 0.46;
	case 7:
		return 0.0040032362459547 * anti_air_ + 0.006336569579288 * weapon_anti_air;
	case 8:
		return 0.485254 + 0.000239 * anti_air_ + 0.001380 * weapon_anti_air;
	case 9:
		return -0.122712 + 0.00376 * anti_air_ + 0.025 * weapon_anti_air;
	case 10:
		return 0.524;
	case 11:
		return 0.505;
	default:
		return 0.540826087;	//分からないのでとりあえず平均値とした
	}
}

// 加重対空値を計算する
double Kammusu::AllAntiAir() const noexcept {
	double aaa_sum = anti_air_;
	for (auto &it_w : weapons_) {
		double aaa;
		if (kammusu_flg_) {
			aaa = it_w.GetAntiAir() + 0.7 * sqrt(it_w.GetLevel());
		}
		else {
			aaa = int(2.0 * sqrt(it_w.GetAntiAir()));
		}
		switch (it_w.GetWeaponClass()) {
		case WC("対空機銃"):
			aaa *= 6.0;
			break;
		case WC("高射装置"):
			aaa *= 4.0;
			break;
		case WC("大型電探"):
		case WC("小型電探"):
			aaa *= 3.0;
			break;
		case WC("艦上戦闘機"):
		case WC("対空強化弾"):
			aaa *= 0.0;
			break;
		case WC("主砲"):
			if (it_w.IsHAG()) aaa *= 4.0; else aaa *= 0.0;
			break;
		default:
			break;
		}
		aaa_sum += aaa;
	}
	return 0.0;
}

// ステータスを返す
Status Kammusu::Status() const noexcept {
	if (hp_ == max_hp_) return kStatusNoDamage;
	if (hp_ == 0) return kStatusLost;
	if (hp_ * 4 > max_hp_ * 3) return kStatusVeryLightDamage;
	if (hp_ * 4 > max_hp_ * 2) return kStatusLightDamage;
	if (hp_ * 4 > max_hp_ * 1) return kStatusMiddleDamage;
	return kStatusHeavyDamage;
}

// 総回避を返す
int Kammusu::AllEvade() const noexcept {
	return evade_ + SumWeapons(&Weapon::GetEvade);
}

// 疲労度を返す
// 厳密にはcond値50以上がキラキラ状態だが、
// 回避率向上はcond値53以上じゃないと起こらないのでそちらに合わせている
Mood Kammusu::Mood() const noexcept {
	if (cond_ < 20) {
		return kMoodRed;
	}
	else if (cond_ < 30) {
		return kMoodOrange;
	}
	else if (cond_ < 53) {
		return kMoodNormal;
	}
	else {
		return kMoodHappy;
	}
}

// 総命中を返す
int Kammusu::AllHit() const noexcept {
	return SumWeapons(&Weapon::GetHit);
}

// フィット砲補正
double Kammusu::FitGunHitPlus() const noexcept {
	if (!AnyOf(SC("戦艦") | SC("航空戦艦"))) return 0.0;
	// 通常命中率低下は赤疲労検証での減少率÷2ぐらいでちょうどいいのでは？
	const double fit[] = { 0.0, 0.01365, 0.0315, 0.0261, 0.0319 };
	const double unfit_small[] = { 0.0, -0.00845, -0.04, -0.0422, -0.04255 };
	const double unfit_large[] = { 0.0, -0.05375, -0.06365, -0.08415, -0.09585 };
	// 数を数えておく
	int sum_356 = 0, sum_38 = 0, sum_381 = 0, sum_41 = 0, sum_46 = 0, sum_46X = 0;
	for (auto &it_w : weapons_) {
		if (it_w.AnyOf(WID("35.6cm連装砲"), WID("試製35.6cm三連装砲"), WID("35.6cm連装砲(ダズル迷彩)")))
			++sum_356;
		else if (it_w.AnyOf(WID("38cm連装砲"), WID("38cm連装砲改")))
			++sum_38;
		else if (it_w.AnyOf(WID("381mm/50 三連装砲"), WID("381mm/50 三連装砲改")))
			++sum_381;
		else if (it_w.AnyOf(WID("41cm連装砲"), WID("試製41cm三連装砲")))
			++sum_41;
		else if (it_w.AnyOf(WID("46cm三連装砲")))
			++sum_46;
		else if (it_w.AnyOf(WID("試製46cm連装砲")))
			++sum_46X;
	}
	// 種類により減衰量を決定する
	if (AnyOf(SID("扶桑"), SID("山城"), SID("伊勢"), SID("日向")))                              return                                                     fit[sum_41]         + unfit_large[sum_46] + unfit_large[sum_46X];		// 伊勢・扶桑型戦艦
	if (AnyOf(SID("伊勢改"), SID("日向改"), SID("扶桑改"), SID("山城改")))                       return fit[sum_356] + fit[sum_38] + fit[sum_381]                               + unfit_large[sum_46] + unfit_small[sum_46X];		// 伊勢型・扶桑型航戦
	if (AnyOf(SID("扶桑改二"), SID("山城改二")))                                                return fit[sum_356] + fit[sum_38] + fit[sum_381]         + fit[sum_41]         + unfit_large[sum_46] + unfit_small[sum_46X];		// 扶桑型航戦改二
	if (AnyOf(SID("金剛"), SID("榛名"), SID("霧島"), SID("比叡"),
		SID("金剛改"), SID("比叡改"), SID("榛名改"), SID("霧島改"),
		SID("金剛改二"), SID("比叡改二"), SID("榛名改二"), SID("霧島改二")))                     return fit[sum_356] + fit[sum_38]                        + unfit_small[sum_41] + unfit_large[sum_46] + unfit_small[sum_46X];		// 金剛型改二
	if (AnyOf(SID("Bismarck"), SID("Bismarck改"), SID("Bismarck zwei"), SID("Bismarck drei"))) return fit[sum_356] + fit[sum_38] - unfit_small[sum_381] + unfit_small[sum_41] + unfit_large[sum_46] + unfit_small[sum_46X];		// Bismarck型
	if (AnyOf(SID("Littorio"), SID("Roma"), SID("Italia"), SID("Roma改")))                     return fit[sum_356]               + fit[sum_381]         + unfit_small[sum_41] + unfit_large[sum_46] + unfit_large[sum_46X];		// イタリア艦
	if (AnyOf(SID("長門"), SID("陸奥"), SID("長門改"), SID("陸奥改")))                          return fit[sum_356]               + fit[sum_381]         + fit[sum_41]         + unfit_small[sum_46] + unfit_small[sum_46X];		// 長門型
	if (AnyOf(SID("大和"), SID("大和改"), SID("武蔵"), SID("武蔵改")))                          return                                                     fit[sum_41];															// 大和型
	return 0.0;
}

// 総雷装を返す
// (level_flgがtrueの場合、装備改修による威力向上も考慮する)
int Kammusu::AllTorpedo(const bool &level_flg) const noexcept {
	double torpedo_sum = torpedo_ + SumWeapons([=](const auto& it_w) {
		return it_w.GetTorpedo() + (level_flg && it_w.AnyOf(WC("魚雷") | WC("対空機銃")) ? 1.2 * sqrt(it_w.GetLevel()) : 0);
	});
	return int(torpedo_sum);
}

// 軽巡軽量砲補正
double Kammusu::FitGunAttackPlus() const noexcept {
	if (AnyOf(SC("軽巡洋艦") | SC("重雷装巡洋艦") | SC("練習巡洋艦"))) {
		int light_gun_single = 0, light_gun_double = 0;
		for (auto &it_w : weapons_) {
			if (it_w.AnyOf(WID("14cm単装砲"), WID("15.2cm単装砲")))
				++light_gun_single;
			else if (it_w.AnyOf(WID("14cm連装砲"), WID("15.2cm連装砲"), WID("15.2cm連装砲改")))
				++light_gun_double;
		}
		return sqrt(light_gun_single) + 2.0 * sqrt(light_gun_double);
	}
		return 0.0;
	}

// 徹甲弾補正
double Kammusu::SpecialEffectApPlus() const noexcept {
	bool has_gun = false, has_ap = false, has_subgun = false, has_radar = false;
	for (auto &it_w : weapons_) {
		switch (it_w.GetWeaponClass()) {
		case WC("主砲"):
			has_gun = true;
			break;
		case WC("対艦強化弾"):
			has_ap = true;
			break;
		case WC("副砲"):
			has_subgun = true;
			break;
		case WC("小型電探"):
		case WC("大型電探"):
			has_radar = true;
			break;
		default:
			break;
		}
	}
	if (has_gun && has_ap) {
		if (has_subgun) {
			return 1.15;
		}
		else if (has_radar) {
			return 1.1;
		}
		else {
			return 1.08;
		}
	}
	else {
		return 1.0;
	}
}

// 熟練艦載機によるCL2率上昇
double Kammusu::CL2ProbPlus() const noexcept {
	return SumWeapons([](const auto& it_w) {
		return it_w.AnyOf(WeaponClass::AirBomb) ? 0.05 * it_w.GetLevel() / 7 : 0;
	});
}

// 熟練艦載機によるダメージ補正
double Kammusu::CL2AttackPlus() const noexcept {
	double cl_attack_plus = 0.0;
	for (size_t wi = 0; wi < slots_; ++wi) {
		auto &it_w = weapons_[wi];
		if (it_w.AnyOf(WeaponClass::AirBomb))
			cl_attack_plus += (wi == 0 ? 0.2 : 0.1) * it_w.GetLevel() / 7;
	}
	return cl_attack_plus;
}

// 総装甲を返す
int Kammusu::AllDefense() const noexcept {
	return defense_ + SumWeapons(&Weapon::GetDefense);
}

// 射程を返す
Range Kammusu::MaxRange() const noexcept {
	auto range = range_;
	for (auto &it_w : weapons_) {
		range = std::max(range, it_w.GetRange());
	}
	return range;
}

// 昼戦火力を返す
int Kammusu::DayAttack(const DayFireType fire_type, const bool af_flg, const FleetType fleet_type, const size_t index) const noexcept {
	double base_attack = 0.0;
	switch (fire_type) {
	case kDayFireGun:	//砲撃
		base_attack += attack_;
		for (auto &it_w : weapons_) {
			base_attack += it_w.GetAttack();
			switch (it_w.GetWeaponClass()) {
			case WC("主砲"):
				base_attack += (it_w.GetRange() >= kRangeLong ? 1.5 : 1.0) * sqrt(it_w.GetLevel());
				break;
			case WC("副砲"):
			case WC("対空機銃"):
			case WC("高射装置"):
			case WC("探照灯"):
				base_attack += sqrt(it_w.GetLevel());
			case WC("ソナー"):
			case WC("爆雷"):
				base_attack += 0.75 * sqrt(it_w.GetLevel());
			default:
				break;
			}
		}
		switch (fleet_type) {
		case FleetType::Normal:
			base_attack += 5.0;
			break;
		case FleetType::CombinedAir:
			if (index == 0) base_attack += 7.0; else base_attack += 15.0;
			break;
		case FleetType::CombinedGun:
			if (index == 0) base_attack += 15.0; else base_attack += 0.0;
			break;
		case FleetType::CombinedDrum:
			if (index == 0) base_attack += 0.0; else base_attack += 15.0;
			break;
		}
		break;
	case kDayFireAir:	//空撃
		{
			// 総雷装・総爆撃・装備改修補正をカウントする
			int all_torpedo = 0, all_bomb = 0;	//総雷装・総爆撃
			double gamma = 0.0;					//装備改修補正
			for (auto &it_w : weapons_) {
				all_torpedo += it_w.GetTorpedo();
				all_bomb += it_w.GetBomb();
				if (it_w.AnyOf(WC("副砲"))) gamma += sqrt(it_w.GetLevel());
			}
			// 陸上型相手だと雷装値が無効になる
			if (af_flg) all_torpedo = 0;
			// 連合艦隊における補正
			int offset = 0;
			switch (fleet_type) {
			case FleetType::CombinedAir:
				if (index == 0) offset = 2; else offset = 10;
				break;
			case FleetType::CombinedGun:
				if (index == 0) offset = 10; else offset = -5;
				break;
			case FleetType::CombinedDrum:
				if (index == 0) offset = -5; else offset = 10;
				break;
			default:
				break;
			}
			// ※キャストだらけですがあくまでも仕様です
			base_attack = int(1.5 * (attack_ + all_torpedo + int(1.3 * all_bomb) + offset + gamma)) + 55;
		}
		break;
	case kDayFireChage:	//爆雷攻撃
		int base_sub = anti_sub_;
		bool air_flg = false;
		for (auto &it_w : weapons_) {
			switch (it_w.GetWeaponClass()) {
			case WC("爆雷"):
			case WC("ソナー"):
				base_attack += it_w.GetAntiSub() * 1.5;
				base_attack += sqrt(it_w.GetLevel());
				break;
			case WC("艦上攻撃機"):
			case WC("オートジャイロ"):
			case WC("対潜哨戒機"):
				base_attack += it_w.GetAntiSub() * 1.5;
				air_flg = true;
				break;
			default:
				// 小口径主砲・水上偵察機・小型電探の対潜値は無視していい
				break;
			}
		}
		base_attack += sqrt(base_sub) * 2 + (air_flg ? 8 : 13);
		break;
	}
	return int(base_attack);
}

// 夜戦火力を返す
int Kammusu::NightAttack(const NightFireType fire_type, const bool af_flg) const noexcept {
	double base_attack = 0.0;
	switch (fire_type) {
	case kNightFireGun:	//砲撃
		base_attack += attack_;
		if (!af_flg) base_attack += torpedo_;
		for (auto &it_w : weapons_) {
			base_attack += it_w.GetAttack();
			if (!af_flg) base_attack += it_w.GetTorpedo();
			switch (it_w.GetWeaponClass()) {
			case WC("主砲"):
				base_attack += (it_w.GetRange() >= kRangeLong ? 1.5 : 1.0) * sqrt(it_w.GetLevel());
				break;
			case WC("副砲"):
			case WC("対空機銃"):
			case WC("高射装置"):
			case WC("探照灯"):
				base_attack += sqrt(it_w.GetLevel());
			case WC("ソナー"):
			case WC("爆雷"):
				base_attack += 0.75 * sqrt(it_w.GetLevel());
			default:
				break;
			}
		}
		break;
	case kNightFireChage:	//爆雷攻撃
		base_attack += SumWeapons([](const auto& it_w) {
			// 夜戦での対潜は、航空対潜ではありえないので除外
			// 小口径主砲・水上偵察機・小型電探の対潜値は無視していい
			return it_w.AnyOf(WC("爆雷") | WC("ソナー")) ? it_w.GetAntiSub() * 1.5 + sqrt(it_w.GetLevel()) : 0;
		});
		base_attack += sqrt(anti_sub_) * 2 + 13;
		break;
	}
	return int(base_attack);
}

// ダメージを与える
void Kammusu::MinusHP(const int &damage, const bool &stopper_flg) {
	if (hp_ <= 0) return;
	if (hp_ > damage) {
		// 残り耐久＞ダメージなら普通に減算
		hp_ -= damage;
	}
	else if (stopper_flg) {
		// そうでない場合、撃沈ストッパーが効いている状況下では割合ダメージに変換される
		hp_ -= int(0.5 * hp_ + 0.3 * rand_.RandInt(hp_));
		if (hp_ <= 0) hp_ = 1;
	}
	else {
		// 撃沈ストッパーが存在しない場合はそのまま沈む
		hp_ = 0;
	}
}

//弾薬・燃料を減少させる
void Kammusu::ConsumeMaterial(const bool night_flg, const bool combined_ss_flg) noexcept {
	if (!combined_ss_flg) {
		ammo_ = (ammo_ - (night_flg ? 30 : 20)) | limit(0, 100);
	}
	fuel_ = (fuel_ - 20) | limit(0, 100);
}

// cond値を変化させる
void Kammusu::ChangeCond(const int cond_change) noexcept {
	cond_ = (cond_ + cond_change) | limit(0, 100);
}

bool Kammusu::HasWeaponClass(const WeaponClass& wc) const noexcept {
	for (const auto& it_w : weapons_) {
		if (it_w.AnyOf(wc) && it_w.GetAir() > 0) return true;
	}
	return false;
}

// 艦載機を保有していた場合はtrue
bool Kammusu::HasAir() const noexcept {
	return HasWeaponClass(WeaponClass::Air);
}

// 航空戦に参加する艦載機を保有していた場合はtrue
bool Kammusu::HasAirFight() const noexcept {
	return HasWeaponClass(WeaponClass::AirFight);
}

// 触接に参加する艦載機を保有していた場合はtrue
bool Kammusu::HasAirTrailer() const noexcept {
	return HasWeaponClass(WeaponClass::AirTrailer);
}

// 艦爆を保有していた場合はtrue
bool Kammusu::HasAirBomb() const noexcept {
	return HasWeaponClass(WC("艦上爆撃機") | WC("艦上爆撃機(爆戦)"));
}

// 昼戦に参加可能な場合はtrue
bool Kammusu::HasAirAttack() const noexcept {
	return HasWeaponClass(WeaponClass::AirBomb);
}

// 潜水艦系ならtrue
bool Kammusu::IsSubmarine() const noexcept {
	return AnyOf(SC("潜水艦") | SC("潜水空母"));
}

// 対潜シナジーを持っていたらtrue
bool Kammusu::HasAntiSubSynergy() const noexcept {
	bool has_dp = false, has_sonar = false;
	for (auto &it_w : weapons_) {
		switch (it_w.GetWeaponClass()) {
		case WC("爆雷"):
			has_dp = true;
			break;
		case WC("ソナー"):
			has_sonar = true;
			break;
		default:
			break;
		}
	}
	return (has_dp && has_sonar);
}

// 徹甲弾補正を食らう側ならtrue
bool Kammusu::IsSpecialEffectAP() const noexcept {
	return AnyOf(SC("重巡洋艦") | SC("航空巡洋艦") | SC("戦艦") | SC("航空戦艦") | SC("正規空母") | SC("陸上型") | SC("装甲空母"));
}

// 彩雲を保有していた場合はtrue
bool Kammusu::HasAirPss() const noexcept {
	for (auto &it_w : weapons_) {
		if (it_w.AnyOf(WC("艦上偵察機(彩雲)"))) return true;
	}
	return false;
}

// 魚雷を発射できればtrue
bool Kammusu::IsFireTorpedo(const TorpedoTurn &torpedo_turn) const noexcept {
	switch (torpedo_turn) {
	case kTorpedoFirst:	//開幕魚雷
		// 鬱陶しいことに艦娘と深海棲艦とでは判定条件が異なるので分けて処理する
		if (kammusu_flg_) {
			// 甲標的を積んだ軽巡(事実上阿武隈改二のみ)・潜水系・雷巡・水母は飛ばせる
			// (冷静に考えると、甲標的さえ載れば飛ばせるはず……)
			/*switch (ship_class_) {
			case SC("軽巡洋艦"):
			case SC("潜水艦"):
			case SC("潜水空母"):
			case SC("重雷装巡洋艦"):
			case SC("水上機母艦"):*/
				for (auto &it_w : weapons_) {
					if (it_w.AnyOf(WC("特殊潜航艇"))) return true;
				}
			/*default:
				break;
			}*/
			// Lv10以上の潜水艦系は甲標的無しでも飛ばせる
			if (IsSubmarine() && level_ >= 10) return true;
		}
		else {
			// elite以上の潜水艦なら開幕魚雷を撃てる(ただし潜水棲姫は除く。なんでや！)
			if (IsSubmarine() && AnyOf(SID("潜水カ級elite"), SID("潜水ヨ級elite"), SID("潜水カ級flagship"), SID("潜水ヨ級flagship"), SID("潜水ソ級elite"), SID("潜水ソ級flagship"))) return true;
			// エリレ級と水母棲姫と駆逐水鬼(甲作戦最終形態,艦船ID=649)と
			// 重巡棲姫(最終形態,艦船ID=660,662,664)は無条件で撃てる
			if (AnyOf(SID("戦艦レ級elite"), L"水母棲姫"s, ShipId::ID649, ShipId::ID660, ShipId::ID662, ShipId::ID664)) return true;
		}
		return false;
		break;
	case kTorpedoSecond:	//雷撃戦
		// 中破以上だと不可
		if (Status() >= kStatusMiddleDamage) return false;
		// 素雷装が0なら不可
		if (torpedo_ == 0) return false;
		// 秋津洲および未改造の千歳型は不可
		if (AnyOf(SID("秋津洲"), SID("秋津洲改"), SID("千歳"), SID("千代田"))) return false;
		return true;
		break;
	default:
		return false;
	}
}

// 砲撃戦で行動可能な艦ならtrue
bool Kammusu::IsMoveGun() const noexcept {
	// 撃沈していたら当然行動できない
	if (Status() == kStatusLost) return false;
	// 潜水艦系も砲撃フェイズでは行動できない
	if (IsSubmarine()) return false;
	// 艦載機が切れた空母も砲撃フェイズでは行動できない
	if (AnyOf(SC("軽空母") | SC("正規空母") | SC("装甲空母")))
		return HasAirAttack();
	return true;
}

// 砲撃戦で攻撃可能な艦ならtrue
bool Kammusu::IsFireGun() const noexcept {
	// 撃沈していたら当然攻撃できない
	if (Status() == kStatusLost) return false;
	// 潜水艦系も砲撃フェイズでは攻撃できない
	if (IsSubmarine()) return false;
	// 艦載機が切れた空母も砲撃フェイズでは攻撃できない
	// また、中破した空母系・大破した装甲空母も攻撃できない
	if (AnyOf(SC("軽空母") | SC("正規空母") | SC("陸上型")))
		return (HasAirAttack() && Status() != kStatusMiddleDamage);
	if (AnyOf(SC("装甲空母")))
		return (HasAirAttack() && Status() != kStatusHeavyDamage);
	return true;
}

// 昼戦で対潜可能な艦ならtrue
bool Kammusu::IsAntiSubDay() const noexcept {
		// 空母型対潜攻撃
	if (AnyOf(SC("軽空母") | SC("陸上型")))
		return IsAntiSubDayPlane();
		// 航戦型対潜攻撃
	if (AnyOf(SC("航空戦艦") | SC("水上機母艦") | SC("航空巡洋艦")))
		return IsAntiSubDayWater();
		// 水雷型対潜攻撃
	if (AnyOf(SC("軽巡洋艦") | SC("重雷装巡洋艦") | SC("駆逐艦") | SC("練習巡洋艦")))
		return anti_sub_ > 0;
		// 上記3種類が合わさった速吸改は頭おかしい(褒め言葉)
	if (AnyOf(SC("給油艦")))
		return (IsAntiSubDayPlane() || IsAntiSubDayWater() || (anti_sub_ > 0));
	return false;
}

// 空撃可能ならtrue
bool Kammusu::IsFireGunPlane() const noexcept {
	return IsAntiSubDayPlane();
}

bool Kammusu::IsAntiSubDayPlane() const noexcept {
	return HasWeaponClass(WC("艦上爆撃機(爆戦)") | WC("艦上爆撃機") | WC("艦上攻撃機"));
}

bool Kammusu::IsAntiSubDayWater() const noexcept {
	return HasWeaponClass(WC("水上爆撃機") | WC("対潜哨戒機") | WC("オートジャイロ"));
}

// 夜戦で攻撃可能な艦ならtrue
bool Kammusu::IsFireNight() const noexcept {
	// 大破していたら攻撃不可
	if (Status() >= kStatusHeavyDamage) return false;
	// 空母系は一部を覗いて攻撃不可
	if (AnyOf(SC("正規空母") | SC("軽空母") | SC("装甲空母"))) {
		if (kammusu_flg_) {
			if (AnyOf(SID("Graf Zeppelin改"), SID("Graf Zeppelin"))) return true;
			return false;
		}
		else {
			if (AnyOf(L"空母ヲ級flagship"s, SID("軽母ヌ級flagship"), L"空母ヲ級改flagship"s)) return true;
			if (AnyOf(SID("軽母ヌ級"), SID("空母ヲ級"), SID("軽母ヌ級elite"), SID("空母ヲ級elite"))) return false;
			return true;
		}
	}
	// それ以外は攻撃可能
	return true;
}

// 夜戦で対潜可能な艦ならtrue
bool Kammusu::IsAntiSubNight() const noexcept {
	if (AnyOf(SC("軽巡洋艦") | SC("重雷装巡洋艦") | SC("駆逐艦") | SC("練習巡洋艦") | SC("給油艦"))) {
		return anti_sub_ > 0;
	}
	return false;
}

// 探照灯や照明弾を保有していた場合はtrue
bool Kammusu::HasLights() const noexcept {
	for (auto &it_w : weapons_) {
		if (it_w.AnyOf(WC("探照灯") | WC("照明弾"))) return true;
	}
	return false;
}

std::ostream & operator<<(std::ostream & os, const Kammusu & conf)
{
	os 
		<< "艦船ID：" << conf.id_ << endl
		<< "　艦名：" << char_cvt::utf_16_to_shift_jis(conf.name_) << "　艦種：" << char_cvt::utf_16_to_shift_jis(to_wstring(conf.ship_class_)) << endl
		<< "　最大耐久：" << conf.max_hp_ << "　装甲：" << conf.defense_ << "　火力：" << conf.attack_ << "　雷撃：" << conf.torpedo_ << endl
		<< "　対空：" << conf.anti_air_ << "　運：" << conf.luck_ << "　速力：" << char_cvt::utf_16_to_shift_jis(kSpeedStr[conf.speed_]) << "　射程：" << char_cvt::utf_16_to_shift_jis(kRangeStr[conf.range_]) << endl
		<< "　スロット数：" << conf.slots_ << "　最大搭載数：";
	for (size_t i = 0; i < conf.slots_; ++i) {
		if (i != 0) os << ",";
		os << conf.max_airs_[i];
	}
	os 
		<< "　回避：" << conf.evade_ << "　対潜：" << conf.anti_sub_ << endl
		<< "　索敵：" << conf.search_ << "　艦娘か？：" << (conf.kammusu_flg_ ? "はい" : "いいえ") << "　レベル：" << conf.level_ << "　現耐久：" << conf.hp_ << endl
		<< "　装備：";
	for (size_t i = 0; i < conf.slots_; ++i) {
		if (i != 0) os << ",";
		os << char_cvt::utf_16_to_shift_jis(conf.weapons_[i].GetName()) << "(" << conf.weapons_[i].GetAir() << ")";
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
		<< L"　艦名：" << conf.name_ << L"　艦種：" << to_wstring(conf.ship_class_) << endl
		<< L"　最大耐久：" << conf.max_hp_ << L"　装甲：" << conf.defense_ << L"　火力：" << conf.attack_ << L"　雷撃：" << conf.torpedo_ << endl
		<< L"　対空：" << conf.anti_air_ << L"　運：" << conf.luck_ << L"　速力：" << kSpeedStr[conf.speed_] << L"　射程：" << kRangeStr[conf.range_] << endl
		<< L"　スロット数：" << conf.slots_ << L"　最大搭載数：";
	for (size_t i = 0; i < conf.slots_; ++i) {
		if (i != 0) os << ",";
		os << conf.max_airs_[i];
	}
	os
		<< L"　回避：" << conf.evade_ << L"　対潜：" << conf.anti_sub_ << endl
		<< L"　索敵：" << conf.search_ << L"　艦娘か？：" << (conf.kammusu_flg_ ? L"はい" : L"いいえ") << L"　レベル：" << conf.level_ << L"　現耐久：" << conf.hp_ << endl
		<< L"　装備：";
	for (size_t i = 0; i < conf.slots_; ++i) {
		if (i != 0) os << ",";
		os << conf.weapons_[i].GetName() << L"(" << conf.weapons_[i].GetAir() << ")";
	}
	os
		<< endl
		<< L"　cond値：" << conf.cond_ << L"　残弾薬(％)：" << conf.ammo_ << L"　残燃料(％)" << conf.fuel_ << endl;
	return os;
}
