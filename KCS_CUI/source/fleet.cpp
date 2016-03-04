#include "base.hpp"
#include "fleet.hpp"
#include "utf8bomskip.hpp"
namespace detail {
	template<typename T>
	struct to_i_limit_helper {
		constexpr to_i_limit_helper(const T &val_min, const T &val_max) : min(val_min), max(val_max) {}
		const T& min;
		const T& max;
	};
	template<typename T>
	inline T operator|(const std::string& str, const to_i_limit_helper<T>& info) {
		const int val = str | to_i();
		return (val < info.min) ? info.min : (info.max < val) ? info.max : static_cast<T>(val);
	}
}
template<typename T>
inline constexpr detail::to_i_limit_helper<T> to_i_limit(const T &val_min, const T &val_max) noexcept { return{ val_min, val_max }; }
namespace detail {
	template<typename ResultType> struct picojson_object_get_with_limit_or_default {
		std::string key; ResultType min; ResultType max; ResultType Default;
	};
	template<typename ResultType>
	ResultType operator|(const picojson::object& obj, const picojson_object_get_with_limit_or_default<ResultType>& info){//ADLで発見される。処理はここが担う
		auto it = obj.find(info.key);
		if (obj.end() == it) return info.Default;
		return it->second.to_str() | to_i_limit(info.min, info.max);
	}
}
template<typename ResultType>
detail::picojson_object_get_with_limit_or_default<ResultType> GetWithLimitOrDefault(std::string key, ResultType min, ResultType max, ResultType Default) noexcept {
	return{ key, min, max, Default };
}

void Fleet::LoadJson(std::istream & file, const WeaponDB & weapon_db, const KammusuDB & kammusu_db, char_cvt::char_enc fileenc)
{
	using picojson::object;
	ENCODE_THROW_WITH_MESSAGE_IF(fileenc == char_cvt::char_enc::unknown, "unknown char enc type.")//文字コード自動判別なんてやらない
	//本当はobjectの検索keyはfileencによってbyte列が違うので文字列テーブルを作らなければならないが、幸いアルファベットのみなのでその作業をサボることにした
	picojson::value v;
	file >> v;
	// 読み込んだJSONデータを解析する
	auto& o = v.get<object>();
	//司令部レベル
	level_ = o | GetWithLimitOrDefault("lv", 1, 120, 120);
	//艦隊の形式
	fleet_type_ = o | GetWithLimitOrDefault("type", kFleetTypeNormal, kFleetTypeCombinedDrum, kFleetTypeNormal);
	if (fleet_type_ != kFleetTypeNormal && formation_ == kFormationEchelon) {
		// 連合艦隊に梯形陣は存在しないので、とりあえず単横陣(第一警戒航行序列)に変更しておく
		formation_ = kFormationAbreast;
	}
	if (fleet_type_ == kFleetTypeNormal) {
		// 通常艦隊
		unit_.resize(1);
	}
	else {
		// 連合艦隊
		unit_.resize(2);
	}
	//艦娘・深海棲艦
	int fi = 0;	//読み込む際のインデックス
	for (auto &temp_f : o) {
		// 艦隊の形式によって、読まなければならない艦隊数は異なる
		if (fleet_type_ == kFleetTypeNormal) {
			if (fi >= 1) break;
		}
		else {
			if (fi >= 2) break;
		}
		// 艦隊を1つづつ読み込んでいく
		auto& fleet = temp_f.second.get<object>();
		for (auto &temp_u : fleet) {
			// 艦船ID・レベル・運・cond値から艦娘を設定する
			auto& unit = temp_u.second.get<object>();
			auto id = unit.at("id").to_str() | to_i();
			auto level = unit.at("lv").to_str() | to_i_limit(1, 155);	//上限はいつか変わるかも？
			Kammusu temp_k = kammusu_db.Get(id, level).Reset();
			auto luck = unit.at("luck").to_str() | to_i_limit(0, 100);
			temp_k.SetLuck(luck);
			if (unit.count("cond")) {
				temp_k.SetCond(unit["cond"].to_str() | to_i_limit(0, 100));
			}
			// 装備ID・改修/熟練度・内部熟練度から装備を設定する
			int wi = 0;
			for (auto &temp_p : unit.at("items").get<object>()) {
				auto& parts = temp_p.second.get<object>();
				Weapon temp_w = weapon_db.Get(parts.at("id").to_str() | to_i());
				// 改修・外部熟練度・内部熟練度の処理
				if (temp_w.IsAir()) {
					level = parts.at("rf").to_str() | to_i_limit(0, 7);
					int level_detail = 0;
					auto it = parts.find("rf_detail");
					if (it != parts.end()) {
						level_detail = it->second.to_str() | to_i_limit(0, 120);
						level = ConvertDtoS(level_detail);
					}
					else {
						level_detail = ConvertStoD(level);
					}
					temp_w.SetLevelDetail(level_detail);
				}
				else {
					level = parts.at("rf").to_str() | to_i_limit(0, 10);
				}
				temp_w.SetLevel(level);
				// 艦娘に装備させる
				temp_k.SetWeapon(wi, temp_w);
				++wi;
				if (wi >= temp_k.GetSlots()) break;
			}
			// リストに加える
			unit_[fi].push_back(move(temp_k));
		}
		++fi;
	}
}
// コンストラクタ
Fleet::Fleet(const string &file_name, const Formation &formation, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, const SharedRand& rand, char_cvt::char_enc fileenc)
	: formation_(formation), rand_(rand)// 陣形はそのまま反映させる
{
	ENCODE_THROW_WITH_MESSAGE_IF(fileenc == char_cvt::char_enc::unknown, "unknown char enc type.")//文字コード自動判別なんてやらない
	// ファイルを読み込む
	ifstream fin(file_name);
	FILE_THROW_WITH_MESSAGE_IF(!fin.is_open(), "艦隊データが正常に読み込めませんでした.")
	if(char_cvt::char_enc::shift_jis != fileenc) skip_utf8_bom(fin, fileenc);
	this->LoadJson(fin, weapon_db, kammusu_db, fileenc);
}

Fleet::Fleet(std::istream & file, const Formation & formation, const WeaponDB & weapon_db, const KammusuDB & kammusu_db, const SharedRand& rand, char_cvt::char_enc fileenc)
	: formation_(formation), rand_(rand)// 陣形はそのまま反映させる
{
	ENCODE_THROW_WITH_MESSAGE_IF(fileenc == char_cvt::char_enc::unknown, "unknown char enc type.")//文字コード自動判別なんてやらない
	this->LoadJson(file, weapon_db, kammusu_db, fileenc);
}

// setter

void Fleet::SetRandGenerator(const SharedRand & rand) {
	this->rand_ = rand;
}

// getter
Formation Fleet::GetFormation() const noexcept { return formation_; }
vector<vector<Kammusu>>& Fleet::GetUnit() { return unit_; }
const vector<vector<Kammusu>>& Fleet::GetUnit() const { return unit_; }
FleetType Fleet::GetFleetType() const noexcept { return fleet_type_; }
// その他
vector<Kammusu>& Fleet::FirstUnit() { return unit_[0]; }
const vector<Kammusu>& Fleet::FirstUnit() const { return unit_[0]; }
vector<Kammusu>& Fleet::SecondUnit() { return unit_[unit_.size() - 1]; }
const vector<Kammusu>& Fleet::SecondUnit() const { return unit_[unit_.size() - 1]; }
size_t Fleet::FleetSize() const noexcept { return unit_.size(); }

//「艦隊数」(通常艦隊だと1、連合艦隊だと2)

size_t Fleet::UnitSize(const size_t fi) const noexcept { return unit_[fi].size(); }

//「艦隊」における艦数

size_t Fleet::FirstIndex() const noexcept { return 0; }

//第一艦隊のインデックス

size_t Fleet::SecondIndex() const noexcept { return unit_.size() - 1; }

// 中身を表示する
void Fleet::Put() const {
	cout << *this;
}

// 索敵値を計算する
double Fleet::SearchValue() const noexcept {
	// 2-5式(秋)を採用。将来的には複数形式を切り替えられるようにする
	double search_sum = 0.0;
	//司令部レベル(5の倍数で切り上げた)による補正
	int round_up5_level = ((level_ - 1) / 5 + 1) * 5;
	search_sum += round_up5_level * (-0.6142467);
	//艦娘・装備による補正
	for (auto &it_u : unit_) {
		for (auto &it_k : it_u) {
			if (it_k.Status() == kStatusLost) continue;
			search_sum += sqrt(it_k.GetSearch()) * 1.6841056;
			for (auto &it_w : it_k.GetWeapon()) {
				switch (it_w.GetWeaponClass()) {
				case kWeaponClassPB:	//艦爆
				case kWeaponClassPBF:	//艦爆
					search_sum += it_w.GetSearch() * 1.0376255;
					break;
				case kWeaponClassWB:	//水爆
					search_sum += it_w.GetSearch() * 1.7787282;
					break;
				case kWeaponClassPA:	//艦攻
					search_sum += it_w.GetSearch() * 1.3677954;
					break;
				case kWeaponClassPS:	//艦偵
					search_sum += it_w.GetSearch() * 1.6592780;
					break;
				case kWeaponClassWS:	//水偵
					search_sum += it_w.GetSearch() * 2.0000000;
					break;
				case kWeaponClassSmallR:	//小型電探
					search_sum += it_w.GetSearch() * 1.0045358;
					break;
				case kWeaponClassLargeR:	//大型電探
					search_sum += it_w.GetSearch() * 0.9906638;
					break;
				case kWeaponClassSL:	//探照灯
					search_sum += it_w.GetSearch() * 0.9067950;
					break;
				default:
					break;
				}
			}
		}
	}
	return floor(search_sum * 10.0 + 0.5) / 10.0;	//小数第2位を四捨五入
}

// 制空値を計算する
int Fleet::AntiAirScore() const noexcept {
	int anti_air_score = 0;
	for (auto &it_k : FirstUnit()) {
		if (it_k.Status() == kStatusLost) continue;
		for (auto wi = 0; wi < it_k.GetSlots(); ++wi) {
			if (!it_k.GetWeapon()[wi].IsAirFight()) continue;
			anti_air_score += it_k.GetWeapon()[wi].AntiAirScore(it_k.GetAir()[wi]);
		}
	}
	return anti_air_score;
}

//触接開始率を計算する
double Fleet::TrailerAircraftProb(const AirWarStatus &air_war_status) const {
	// 制空権確保時の確率を計算する
	double trailer_aircraft_prob = 0.0;
	for (auto &it_k : FirstUnit()) {
		if (it_k.Status() == kStatusLost) continue;
		for (auto wi = 0; wi < it_k.GetSlots(); ++wi) {
			auto it_w = it_k.GetWeapon()[wi];
			switch (it_w.GetWeaponClass()) {
			case kWeaponClassPS:
			case kWeaponClassPSS:
			case kWeaponClassDaiteiChan:
			case kWeaponClassWS:
			case kWeaponClassWSN:
				trailer_aircraft_prob += 0.04 * it_w.GetSearch() * sqrt(it_k.GetAir()[wi]);
			default:
				break;
			}
		}
	}
	// 制空段階によって補正を掛ける(航空優勢以外は試験実装)
	switch (air_war_status) {
	case kAirWarStatusBest:
		break;
	case kAirWarStatusGood:
		trailer_aircraft_prob *= 0.6;
		break;
	case kAirWarStatusNormal:
		trailer_aircraft_prob *= 0.4;
		break;
	case kAirWarStatusBad:
		trailer_aircraft_prob *= 0.2;
		break;
	case kAirWarStatusWorst:
		trailer_aircraft_prob *= 0.0;
		break;
	}
	return trailer_aircraft_prob;
}

// 攻撃力補正を計算する
double Fleet::TrailerAircraftPlus(){
	const static double all_attack_plus_list[] = { 1.12, 1.12, 1.17, 1.20 };
	for (auto &it_k :FirstUnit() ) {
		if (it_k.Status() == kStatusLost) continue;
		for (auto &it_w : it_k.GetWeapon()) {
			if (!it_w.IsAirTrailer()) continue;
			if (0.07 * it_w.GetSearch() >= rand_.RandReal()) {
				return all_attack_plus_list[it_w.GetHit()];
			}
		}
	}
	return 1.0;
}

// 発動する対空カットインの種類を判断する
int Fleet::AacType() {
	// まず、秋月型カットイン以外の判定を行う
	for (auto &it_u : unit_) {
		for (auto &it_k : it_u) {
			if (it_k.Status() == kStatusLost) continue;
			auto aac_type = it_k.AacType();
			if (aac_type <= 3) continue;
			if (it_k.AacProb(aac_type) < rand_.RandReal()) continue;
			return aac_type;
		}
	}
	// 次に、秋月型カットインの判定を行う
	for (auto &it_u : unit_) {
		for (auto &it_k : it_u) {
			if (it_k.Status() == kStatusLost) continue;
			auto aac_type = it_k.AacType();
			if (aac_type != limit(aac_type, 1, 3)) continue;
			if (it_k.AacProb(aac_type) < rand_.RandReal()) continue;
			return aac_type;
		}
	}
	return 0;
}

// 艦隊対空ボーナス値を計算する
int Fleet::AntiAirBonus() const {
	const static vector<vector<double>> kAntiAirBonusPer = { { 0.77, 0.91, 1.2, 0.77, 0.77 },{ 1.0, 1.2, 1.6, 1.0, 1.0 } };
	int fleets_anti_air_bonus = 0;
	for (auto &it_u : unit_) {
		for (auto &it_k : it_u) {
			if (it_k.Status() == kStatusLost) continue;
			double anti_air_bonus = 0.0;
			for (auto &it_w : it_k.GetWeapon()) {
				if (it_w.IsHAG() || it_w.Include(L"高射装置")) {
					anti_air_bonus += 0.35 * it_w.GetAntiAir();
				}
				else if (it_w.GetWeaponClass() == kWeaponClassSmallR || it_w.GetWeaponClass() == kWeaponClassLargeR) {
					anti_air_bonus += 0.4 * it_w.GetAntiAir();
				}
				else if (it_w.GetWeaponClass() == kWeaponClassAAA) {
					anti_air_bonus += 0.6 * it_w.GetAntiAir();
				}
				else {
					anti_air_bonus += 0.2 * it_w.GetAntiAir();
				}
			}
			fleets_anti_air_bonus += int(anti_air_bonus);
		}
	}
	return int(2 * kAntiAirBonusPer[(FirstUnit()[0].IsKammusu() ? 0 : 1)][formation_] * fleets_anti_air_bonus);
}

// 生存艦から艦娘をランダムに指定する(航空戦用)
int Fleet::RandomKammusu() {
	//生存艦をリストアップ
	vector<int> alived_list;
	for (auto ui = 0u; ui < FirstUnit().size(); ++ui) {
		if (FirstUnit()[ui].Status() != kStatusLost) alived_list.push_back(ui);
	}
	if (alived_list.size() == 0) return -1;
	return alived_list[rand_.RandInt(alived_list.size())];
}

// 生存する水上艦から艦娘をランダムに指定する
// ただしhas_bombがtrueの際は陸上型棲姫を避けるようになる
tuple<bool, KammusuIndex> Fleet::RandomKammusuNonSS(const bool &has_bomb, const TargetType &target_type) {
	// 攻撃する艦隊の対象を選択する
	vector<size_t> list;
	switch (target_type) {
	case kTargetTypeFirst:
		list = { FirstIndex() };
		break;
	case kTargetTypeSecond:
		list = { SecondIndex() };
		break;
	case kTargetTypeAll:
		list = { FirstIndex(), SecondIndex() };
		break;
	}
	//生存する水上艦をリストアップ
	vector<KammusuIndex> alived_list;
	for (auto &fi : list) {
		for (auto ui = 0u; ui < GetUnit()[fi].size(); ++ui) {
			auto &it_k = GetUnit()[fi][ui];
			if (it_k.Status() == kStatusLost) continue;
			if (it_k.IsSubmarine()) continue;
			if (has_bomb && it_k.GetShipClass() == kShipClassAF) continue;
			alived_list.push_back({ fi, ui });
		}
	}
	if (alived_list.size() == 0) return tuple<bool, KammusuIndex>(false, { 0 , 0 });
	return tuple<bool, KammusuIndex>(true, alived_list[rand_.RandInt(alived_list.size())]);
}

// 潜水の生存艦から艦娘をランダムに指定する
tuple<bool, KammusuIndex> Fleet::RandomKammusuSS(const size_t &fleet_index) {
	// 攻撃する艦隊の対象を選択する
	vector<size_t> list;
	switch (fleet_index) {
	case 0:
		list = { FirstIndex() };
		break;
	case 1:
		list = { SecondIndex() };
		break;
	}
	//生存する水上艦をリストアップ
	vector<KammusuIndex> alived_list;
	for (auto &fi : list) {
		for (auto ui = 0u; ui < GetUnit()[fi].size(); ++ui) {
			auto &it_k = GetUnit()[fi][ui];
			if (it_k.Status() == kStatusLost) continue;
			if (!it_k.IsSubmarine()) continue;
			alived_list.push_back({ fi, ui });
		}
	}
	if (alived_list.size() == 0) return tuple<bool, KammusuIndex>(false, { 0 , 0 });
	return tuple<bool, KammusuIndex>(true, alived_list[rand_.RandInt(alived_list.size())]);
}

template<typename CondFunc>
bool any_of(const std::vector<std::vector<Kammusu>>& unit, CondFunc cond) noexcept {
	for (auto &it_u : unit) {
		for (auto &it_k : it_u) {
			if (it_k.Status() == kStatusLost) continue;
			if (cond(it_k)) return true;
		}
	}
	return false;
}
// 艦載機をいずれかの艦が保有していた場合はtrue
bool Fleet::HasAir() const noexcept {
	return any_of(this->unit_, [](const Kammusu& it_k) -> bool { return it_k.HasAir(); });
}

// 航空戦に参加する艦載機をいずれかの艦が保有していた場合はtrue
bool Fleet::HasAirFight() const noexcept {
	return any_of(this->unit_, [](const Kammusu& it_k) -> bool { return it_k.HasAirFight(); });
}

// 触接に参加する艦載機をいずれかの艦が保有していた場合はtrue
bool Fleet::HasAirTrailer() const noexcept {
	return std::any_of(this->FirstUnit().begin(), this->FirstUnit().end(), [](const Kammusu& it_k) -> bool { return it_k.HasAirTrailer(); });
}

// 彩雲をいずれかの艦が保有していた場合はtrue
bool Fleet::HasAirPss() const noexcept {
	return std::any_of(this->FirstUnit().begin(), this->FirstUnit().end(), [](const Kammusu& it_k) -> bool { return it_k.HasAirPss(); });
}

std::ostream & operator<<(std::ostream & os, const Fleet & conf)
{
	os << "陣形：" << char_cvt::utf_16_to_shift_jis(kFormationStr[conf.formation_]) << "　司令部レベル：" << conf.level_ << "　形式：" << char_cvt::utf_16_to_shift_jis(kFleetTypeStr[conf.fleet_type_ - 1]) << endl;
	for (auto fi = 0u; fi < conf.unit_.size(); ++fi) {
		os << "　第" << (fi + 1) << "艦隊：" << endl;
		for (auto &it_k : conf.unit_[fi]) {
			os << "　　" << char_cvt::utf_16_to_shift_jis(it_k.GetNameLv()) << endl;
		}
	}
	os << endl;
	return os;
}

std::wostream & operator<<(std::wostream & os, const Fleet & conf)
{
	os << L"陣形：" << kFormationStr[conf.formation_] << L"　司令部レベル：" << conf.level_ << L"　形式：" << kFleetTypeStr[conf.fleet_type_ - 1] << endl;
	for (auto fi = 0u; fi < conf.unit_.size(); ++fi) {
		os << L"　第" << (fi + 1) << L"艦隊：" << endl;
		for (auto &it_k : conf.unit_[fi]) {
			os << L"　　" << it_k.GetNameLv() << endl;
		}
	}
	os << endl;
	return os;
}
