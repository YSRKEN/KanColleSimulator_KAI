#include "base.hpp"
#include "kammusu.hpp"
#include "other.hpp"
#include "char_convert.hpp"
#include "fleet.hpp"
#include "simulator.hpp"
// コンストラクタ
Kammusu::Kammusu() 
	:	Kammusu(-1, L"なし", kShipClassDD, 0, 0, 0, 0, 0, 0, kSpeedNone, kRangeNone,
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
	this->Reset();
}

Kammusu::Kammusu(
	const DependOnLv info, const int id, wstring name, const ShipClass shipclass, 
	const int luck, const Speed speed, const Range range, const int slots, vector<int> max_airs, 
	vector<int> first_weapons, const bool kammusu_flg, const SharedRand& rand
)
	: Kammusu(
		id, move(name), shipclass, info.max_hp, info.defense, info.attack, info.torpedo, info.anti_air, luck, speed, range, slots, 
		move(max_airs), info.evade, info.anti_sub, info.search, move(first_weapons), kammusu_flg, info.level, rand
	)
{}

void Kammusu::SetRandGenerator(const SharedRand & rand) {
	this->rand_ = rand;
}

// 中身を表示する
void Kammusu::Put() const {
	cout << *this;
}

// 簡易的な名称を返す
wstring Kammusu::GetNameLv() const {
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
		weapons_[i] = weapon_db.Get(first_weapons_[i], std::nothrow);
	}
	return *this;
}

// 対空カットインの種類を判別する
int Kammusu::AacType() const noexcept {
	// 各種兵装の数を数える
	//高角砲、高角砲+高射装置、高射装置、対空機銃(三式弾以外)、集中配備の数
	size_t sum_hag = 0, sum_hagX = 0, sum_aad = 0, sum_aag = 0, sum_aagX = 0;
	// 大口径主砲・水上電探・対空電探・三式弾の数
	size_t sum_gunL = 0, sum_radarW = 0, sum_radarA = 0, sum_three = 0;
	for (auto &it_w : weapons_) {
		switch (it_w.GetWeaponClass()) {
		case kWeaponClassGun:
			if (it_w.IsHAG()) {
				if (it_w.Include(L"高射装置") || it_w.GetName() == L"90mm単装高角砲") {
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
		case kWeaponClassAAD:
			++sum_aad;
			break;
		case kWeaponClassAAG:
			if (it_w.Include(L"集中")) {
				++sum_aagX;
			}
			else {
				++sum_aag;
			}
			break;
		case kWeaponClassAAA:
			++sum_three;
			break;
		case kWeaponClassSmallR:
		case kWeaponClassLargeR:
			if (it_w.Include(L"対空")) {
				++sum_radarA;
			}
			else {
				++sum_radarW;
			}
		default:
			break;
		}
	}
	// まず、固有カットインを判定する
	if (Include(L"秋月") || Include(L"照月") || Include(L"初月")) {
		/* 秋月型……ご存知防空駆逐艦。対空カットイン無しでも圧倒的な対空値により艦載機を殲滅する。
		* 二次創作界隈ではまさma氏が有名であるが、秋月型がこれ以上増えると投稿時のタイトルが長くなりすぎることから
		* 嬉しい悲鳴を上げていたとか。なお史実上では後9隻居るが、有名なのは涼月などだろう……  */
		if (sum_hag + sum_hagX >= 2 && sum_radarW + sum_radarA >= 1) return 1;
		if (sum_hag + sum_hagX >= 1 && sum_radarW + sum_radarA >= 1) return 2;
		if (sum_hag + sum_hagX >= 2) return 3;
	}
	if (name_ == L"摩耶改二") {
		/* 摩耶改二……麻耶ではない。対空兵装により「洋上の対空要塞」(by 青島文化教材社)となったため、
		* 重巡にしては驚異的な対空値を誇る。ついでに服装もかなりプリティーに進化した(妹の鳥海も同様) */
		if (sum_hag + sum_hagX >= 1 && sum_aagX >= 1 && sum_radarA >= 1) return 10;
		if (sum_hag + sum_hagX >= 1 && sum_aagX >= 1) return 11;
	}
	if (name_ == L"五十鈴改二") {
		/* 五十鈴改二…… 名前通りLv50からの改装である。防空巡洋艦になった史実から、射程が短となり、
		* 防空力が大幅にアップした。しかし搭載数0で火力面で使いづらくなった上、対潜は装備対潜のウェイトが高いため
		* 彼女を最適解に出来る状況は限られている。また、改二なのに金レアで固有カットインがゴミクズ「だった」ことから、
		* しばしば不遇改二の代表例として挙げられていた。逆に言えば、新人向けに便利とも言えるが…… */
		if (sum_hag + sum_hagX >= 1 && sum_aag + sum_aagX >= 1 && sum_radarA >= 1) return 14;
		if (sum_hag + sum_hagX >= 1 && sum_aag + sum_aagX >= 1) return 15;
	}
	if (name_ == L"霞改二乙") {
		/* 霞改二乙…… Lv88という驚異的な練度を要求するだけあり、内蔵されたギミックは特殊である。
		* まず霞改二でも積めた大発に加え、大型電探も装備可能になった(代償に艦隊司令部施設が積めなくなった)。
		* また、対空値も上昇し、固有カットインも実装された。ポスト秋月型＋アルファとも言えるだろう。
		* なお紐が霞改二と違い赤色であるが、どちらにせよランドｓゲフンゲフン */
		if (sum_hag + sum_hagX >= 1 && sum_aag + sum_aagX >= 1 && sum_radarA >= 1) return 16;
		if (sum_hag + sum_hagX >= 1 && sum_aag + sum_aagX >= 1) return 17;
	}
	if (name_ == L"皐月改二") {
		/* 皐月改二…… うるう年の2/29に実装された、皐月改二における固有の対空カットイン。
		 * この調子では改二が出るたびに新型カットインが出るのではないかと一部で危惧されている。*/
		if (sum_aagX >= 1) return 18;
	}
	// 次に一般カットインを判定する
	if (sum_gunL >= 1 && sum_three >= 1 && sum_hagX + sum_aad >= 1 && sum_radarA >= 1) return 4;
	if (sum_hagX >= 2 && sum_radarA >= 1) return 5;
	if (sum_gunL >= 1 && sum_three >= 1 && sum_hagX + sum_aad >= 1) return 6;
	if (sum_hag + sum_hagX >= 1 && sum_aad >= 1 && sum_radarA >= 1) return 7;
	if (sum_hagX >= 1 && sum_radarA >= 1) return 8;
	if (sum_hag + sum_hagX >= 1 && sum_aad >= 1) return 9;
	if (sum_aagX >= 1 && sum_aag >= 1 && sum_radarA >= 1) return 12;
	return 0;
}

// 対空カットインの発動確率を計算する
double Kammusu::AacProb(const int &aac_type) const noexcept {
	// 色々とお察しください
	/* 艦娘       位置   素対空値   装備対空値   種類   装備                                         結果      ％      備考
	 * 秋月       僚艦   116        6            1      12.7高単、12.7高単、22号改四                 72/100    72.0%
	 * 秋月       僚艦   116        24           1      秋月砲★9、秋月砲★9、13号改                 164/206   79.6%   (16-991)
	 * 秋月       僚艦   116        17           3      10cm、10cm、94式                             121/202   59.9%
	 * 秋月       僚艦   116        23           3      秋月砲★9、秋月砲★9、94式★6                33/56     58.9%   (16-813)
	 * 榛名       旗艦   92         18           4      ダズル、三式弾、14号、91式                   33/50     66.0%   (16-691)
	 * 大淀       僚艦   74         28           5      秋月砲、秋月砲、14号、観測機                 67/100    67.0%
	 * 大淀       僚艦   74         29           5      秋月砲、秋月砲、14号、94式                   65/100    65.0%
	 * 大淀       僚艦   74         32           5      秋月砲、秋月砲、14号、14号                   60/100    60.0%
	 * 榛名       旗艦   92         12           6      ダズル、三式弾、22号、91式                   23/50     46.0%
	 * 大淀       僚艦   74         29           7      浦風砲、浦風砲、14号、94式                   48/100    48.0%
	 * 摩耶       僚艦   89         14           7      8cm、91式、14号                              89/200    44.5%
	 * 雪風       旗艦   59         14           8      秋月砲★9、13号改、魚雷                      144/279   51.6%   (17-10)
	 * 能代       僚艦   72         8            8      秋月砲★4、FuMO                              48/100    48.0%
	 * 時雨       旗艦   72         14           8      秋月砲★9、13号改、魚雷                      58/100    58.0%   (16-785)
	 * 時雨       旗艦   72         16           8      秋月砲、13号改、主砲                         26/50     52.0%
	 * 大淀       僚艦   74         25           8      秋月砲、10cm、14号、観測機                   52/100    52.0%   (16-600)
	 * 陽炎       僚艦   49         17           9      10cm、10cm、94式                             103/300   34.3%   (16-702,17-44)
	 * 能代       僚艦   72         6            9      8cm、91式                                    44/100    44.0%   (16-701)
	 * 能代       僚艦   72         8            9      8cm、94式★2                                 49/100    49.0%   (16-660)
	 * 潮         僚艦   74         17           9      10cm、10cm、94式                             131/300   43.7%   (17-44)
	 * 摩耶       僚艦   89         8            9      8cm、91式                                    83/200    41.5%
	 * 摩耶改二   僚艦   106        27           10     2号砲★9、90mm高★10、集中機銃、Fumo         108/206   52.4%   [22]
	 * 摩耶改二   僚艦   106        22           11     2号砲★9、90mm高★10、集中機銃、観測機>>     103/204   50.5%
	 */
	// とりあえず装備対空を計算する
	int weapon_anti_air = 0;
	for (auto &it_w : weapons_) {
		weapon_anti_air += it_w.GetAntiAir();
	}
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
		case kWeaponClassAAG:
			aaa *= 6.0;
			break;
		case kWeaponClassAAD:
			aaa *= 4.0;
			break;
		case kWeaponClassLargeR:
		case kWeaponClassSmallR:
			aaa *= 3.0;
			break;
		case kWeaponClassPF:
		case kWeaponClassAAA:
			aaa *= 0.0;
			break;
		case kWeaponClassGun:
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
	int evade_sum = evade_;
	for (auto &it_w : weapons_) {
		evade_sum += it_w.GetEvade();
	}
	return evade_sum;
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
	int hit_sum = 0;
	for (auto &it_w : weapons_) {
		hit_sum += it_w.GetHit();
	}
	return hit_sum;
}

// フィット砲補正
double Kammusu::FitGunHitPlus() const noexcept {
	double hit_plus = 0.0;
	// 通常命中率低下は赤疲労検証での減少率÷2ぐらいでちょうどいいのでは？
	const double fit[] = { 0.0, 0.01365, 0.0315, 0.0261, 0.0319 };
	const double unfit_small[] = { 0.0, -0.00845, -0.04, -0.0422, -0.04255 };
	const double unfit_large[] = { 0.0, -0.05375, -0.06365, -0.08415, -0.09585 };
	// 数を数えておく
	int sum_356 = 0, sum_38 = 0, sum_381 = 0, sum_41 = 0, sum_46 = 0, sum_46X = 0;
	for (auto &it_w : weapons_) {
		if (it_w.Include(L"35.6cm")) ++sum_356;
		if (it_w.Include(L"38cm")) ++sum_38;
		if (it_w.Include(L"381mm")) ++sum_381;
		if (it_w.Include(L"41cm")) ++sum_41;
		if (it_w.Include(L"46cm")) {
			if (it_w.Include(L"試製")) ++sum_46X; else ++sum_46;
		}
	}
	// 種類により減衰量を決定する
	//伊勢型および扶桑型
	if (Include(L"伊勢") || Include(L"日向") || Include(L"扶桑") || Include(L"山城")) {
			if(Include(L"改")) {
				// 航戦
				hit_plus = fit[sum_41] + unfit_large[sum_46] + unfit_large[sum_46X];
			}
			else {
				// 戦艦
				hit_plus = fit[sum_356] + fit[sum_38] + fit[sum_381] + unfit_large[sum_46] + unfit_small[sum_46X];
				if (Include(L"扶桑") || Include(L"山城")) {
					hit_plus += fit[sum_41];
				}
			}
	}
	//金剛型およびビスマルク
	if (Include(L"金剛") || Include(L"比叡") || Include(L"榛名") || Include(L"霧島") || Include(L"Bismarck")) {
		hit_plus = fit[sum_356] + fit[sum_38] + unfit_small[sum_41] + unfit_large[sum_46] + unfit_small[sum_46X];
		if (Include(L"Bismarck")) {
			hit_plus += unfit_small[sum_381];
		}
	}
	//イタリア艦
	if (Include(L"Littorio") || Include(L"Italia") || Include(L"Roma")) {
		hit_plus = fit[sum_356] + fit[sum_381] + unfit_small[sum_41] + unfit_large[sum_46] + unfit_large[sum_46X];
	}
	//長門型
	if (Include(L"長門") || Include(L"陸奥")) {
		hit_plus = fit[sum_356] + fit[sum_381] + fit[sum_41] + unfit_small[sum_46] + unfit_small[sum_46X];
	}
	//大和型
	if (Include(L"大和") || Include(L"武蔵")) {
		hit_plus = fit[sum_41];
	}
	return hit_plus;
}

// 総雷装を返す
// (level_flgがtrueの場合、装備改修による威力向上も考慮する)
int Kammusu::AllTorpedo(const bool &level_flg) const noexcept {
	double torpedo_sum = torpedo_;
	for (auto &it_w : weapons_) {
		torpedo_sum += it_w.GetTorpedo();
		if (level_flg) {
			switch (it_w.GetAntiAir()) {
			case kWeaponClassTorpedo:
			case kWeaponClassAAG:
				torpedo_sum += 1.2 * sqrt(it_w.GetLevel());
			default:
				break;
			}
		}
	}
	return int(torpedo_sum);
}

// 軽巡軽量砲補正
double Kammusu::FitGunAttackPlus() const noexcept {
	switch (ship_class_) {
	case kShipClassCL:
	case kShipClassCLT:
	case kShipClassCP:
	{
		int light_gun_single = 0, light_gun_double = 0;
		for (auto &it_w : weapons_) {
			auto &name = it_w.GetName();
			if (name == L"14cm単装砲"
				|| name == L"15.2cm単装砲") ++light_gun_single;
			if (name == L"14cm連装砲"
				|| name == L"15.2cm連装砲"
				|| name == L"15.2cm連装砲改") ++light_gun_double;
		}
		return sqrt(light_gun_single) + 2.0 * sqrt(light_gun_double);
	}
		break;
	default:
		return 0.0;
	}
}

// 徹甲弾補正
double Kammusu::SpecialEffectApPlus() const noexcept {
	bool has_gun = false, has_ap = false, has_subgun = false, has_radar = false;
	for (auto &it_w : weapons_) {
		switch (it_w.GetWeaponClass()) {
		case kWeaponClassGun:
			has_gun = true;
			break;
		case kWeaponClassAP:
			has_ap = true;
			break;
		case kWeaponClassSubGun:
			has_subgun = true;
			break;
		case kWeaponClassSmallR:
		case kWeaponClassLargeR:
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
	double cl_prob_plus = 0.0;
	for (auto &it_w : weapons_) {
		switch (it_w.GetWeaponClass()) {
		case kWeaponClassPA:
		case kWeaponClassPB:
		case kWeaponClassPBF:
		case kWeaponClassWB:
			cl_prob_plus += 0.05 * it_w.GetLevel() / 7;
			break;
		default:
			break;
		}
	}
	return cl_prob_plus;
}

// 熟練艦載機によるダメージ補正
double Kammusu::CL2AttackPlus() const noexcept {
	double cl_attack_plus = 0.0;
	for (auto wi = 0; wi < slots_; ++wi) {
		auto &it_w = weapons_[wi];
		switch (it_w.GetWeaponClass()) {
		case kWeaponClassPA:
		case kWeaponClassPB:
		case kWeaponClassPBF:
		case kWeaponClassWB:
			cl_attack_plus += (wi == 0 ? 0.2 : 0.1) * it_w.GetLevel() / 7;
			break;
		default:
			break;
		}
	}
	return cl_attack_plus;
}

// 総装甲を返す
int Kammusu::AllDefense() const noexcept {
	int defense_sum = defense_;
	for (auto &it_w : weapons_) {
		defense_sum += it_w.GetDefense();
	}
	return defense_sum;
}

// ダメージを与える
void Kammusu::MinusHP(const int &damage, const bool &stopper_flg) {
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

// 艦載機を保有していた場合はtrue
bool Kammusu::HasAir() const noexcept {
	for (auto i = 0; i < slots_; ++i) {
		if (weapons_[i].IsAir() && airs_[i] > 0) return true;
	}
	return false;
}

// 航空戦に参加する艦載機を保有していた場合はtrue
bool Kammusu::HasAirFight() const noexcept {
	for (auto i = 0; i < slots_; ++i) {
		if (weapons_[i].IsAirFight() && airs_[i] > 0) return true;
	}
	return false;
}

// 触接に参加する艦載機を保有していた場合はtrue
bool Kammusu::HasAirTrailer() const noexcept {
	for (auto i = 0; i < slots_; ++i) {
		if (weapons_[i].IsAirTrailer() && airs_[i] > 0) return true;
	}
	return false;
}

// 艦爆を保有していた場合はtrue
bool Kammusu::HasAirBomb() const noexcept {
	for (auto i = 0; i < slots_; ++i) {
		auto weapon_class = weapons_[i].GetWeaponClass();
		if ((weapon_class == kWeaponClassPB || weapon_class == kWeaponClassPBF) && airs_[i] > 0) return true;
	}
	return false;
}

// 潜水艦系ならtrue
bool Kammusu::IsSubmarine() const noexcept {
	if (ship_class_ == kShipClassSS || ship_class_ == kShipClassSSV) return true;
	return false;
}

// 名前に特定の文字が含まれていればtrue
bool Kammusu::Include(const wstring &wstr) const noexcept {
	return (name_.find(wstr) != wstring::npos);
}

// 対潜シナジーを持っていたらtrue
bool Kammusu::HasAntiSubSynergy() const noexcept {
	bool has_dp = false, has_sonar = false;
	for (auto &it_w : weapons_) {
		switch (it_w.GetWeaponClass()) {
		case kWeaponClassDP:
			has_dp = true;
			break;
		case kWeaponClassSonar:
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
	switch (ship_class_) {
	case kShipClassCA:
	case kShipClassCAV:
	case kShipClassBB:
	case kShipClassBBV:
	case kShipClassCV:
	case kShipClassAF:
	case kShipClassACV:
		return true;
	default:
		return false;
	}
}

// 彩雲を保有していた場合はtrue
bool Kammusu::HasAirPss() const noexcept {
	for (auto &it_w : weapons_) {
		if (it_w.GetWeaponClass() == kWeaponClassPSS) return true;
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
			switch (ship_class_) {
			case kShipClassCL:
			case kShipClassSS:
			case kShipClassSSV:
			case kShipClassCLT:
			case kShipClassAV:
				for (auto &it_w : weapons_) {
					if (it_w.GetWeaponClass() == kWeaponClassSpecialSS) return true;
				}
			default:
				break;
			}
			// Lv10以上の潜水艦系は甲標的無しでも飛ばせる
			if (IsSubmarine() && level_ >= 10) return true;
		}
		else {
			// elite以上の潜水艦なら開幕魚雷を撃てる(ただし潜水棲姫は除く。なんでや！)
			if (IsSubmarine() && (Include(L"elite") || Include(L"flagship"))) return true;
			// エリレ級と水母棲姫と駆逐水鬼(甲作戦最終形態,艦船ID=649)は無条件で撃てる
			if (name_ == L"戦艦レ級elite" || name_ == L"水母棲姫" || id_ == 649) return true;
		}
		return false;
		break;
	case kTorpedoSecond:	//雷撃戦
		// 中破以上だと不可
		if (Status() >= kStatusMiddleDamage) return false;
		// 素雷装が0なら不可
		if (torpedo_ == 0) return false;
		// 秋津洲および未改造の千歳型は不可
		if (Include(L"秋津洲") || name_ == L"千歳" || name_ == L"千代田") return false;
		return true;
		break;
	default:
		return false;
	}
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
		os << char_cvt::utf_16_to_shift_jis(conf.weapons_[i].GetName()) << "(" << conf.airs_[i] << ")";
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
		os << conf.weapons_[i].GetName() << L"(" << conf.airs_[i] << ")";
	}
	os
		<< endl
		<< L"　cond値：" << conf.cond_ << L"　残弾薬(％)：" << conf.ammo_ << L"　残燃料(％)" << conf.fuel_ << endl;
	return os;
}
