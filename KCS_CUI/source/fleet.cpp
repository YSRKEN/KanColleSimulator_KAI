﻿#include "base.hpp"
#include "fleet.hpp"
#include "utf8bomskip.hpp"
#include <limits>
using namespace std::string_literals;
namespace detail {
	template<typename T>
	struct to_i_limit_helper {
		constexpr to_i_limit_helper(const T &val_min, const T &val_max) : min(val_min), max(val_max) {}
		const T& min;
		const T& max;
	};

	//SFINAEしないと`std::underlying_type_t<some arithmetic type>``のインスタンス化が要求されるのでstd::conditional_tは使えない

	template<typename T, std::enable_if_t<std::is_arithmetic<T>::value, std::nullptr_t> = nullptr>//arithmetic version
	inline T operator|(const std::string& str, const to_i_limit_helper<T>& info) {
		const auto val = str | to_i<T>();
		return (val < info.min) ? info.min : (info.max < val) ? info.max : val;
	}
	template<typename T, std::enable_if_t<std::is_enum<T>::value, std::nullptr_t> = nullptr>//enum version
	inline T operator|(const std::string& str, const to_i_limit_helper<T>& info) {
		using BaseType = std::underlying_type_t<T>;//基底型の取得
		const auto val = str | to_i<BaseType>();
		//enum classにoperator<を要求しないためにキャストが必要
		return (val < static_cast<BaseType>(info.min)) ? info.min : (static_cast<BaseType>(info.max) < val) ? info.max : static_cast<T>(val);
	}
}
template<typename T, std::enable_if_t<std::is_integral<T>::value || std::is_enum<T>::value, std::nullptr_t> = nullptr>
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

void Fleet::LoadJson(std::istream & file, char_cvt::char_enc fileenc)
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
	fleet_type_ = o | GetWithLimitOrDefault("type", FleetType::Normal, FleetType::CombinedDrum, FleetType::Normal);
	if (fleet_type_ != FleetType::Normal && formation_ == kFormationEchelon) {
		// 連合艦隊に梯形陣は存在しないので、とりあえず単横陣(第一警戒航行序列)に変更しておく
		formation_ = kFormationAbreast;
	}
	if (fleet_type_ == FleetType::Normal) {
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
		if (fleet_type_ == FleetType::Normal) {
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
			Kammusu temp_k = Kammusu::Get(id, level).Reset(false);
			auto luck = unit.at("luck").to_str() | to_i_limit(0, 100);
			temp_k.SetLuck(luck);
			if (unit.count("cond")) {
				temp_k.SetCond(unit["cond"].to_str() | to_i_limit(0, 100));
			}
			// 装備ID・改修/熟練度・内部熟練度から装備を設定する
			size_t wi = 0;
			for (auto &temp_p : unit.at("items").get<object>()) {
				auto& parts = temp_p.second.get<object>();
				Weapon temp_w = Weapon::Get(parts.at("id").to_str() | to_i());
				// 改修・外部熟練度・内部熟練度の処理
				if (temp_w.AnyOf(WeaponClass::Air)) {
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
			temp_k.AacType_();	//事前に対空カットインの種類を判別しておく
			unit_[fi].push_back(move(temp_k));
		}
		++fi;
	}
}
// コンストラクタ
Fleet::Fleet(const string &file_name, const Formation &formation, char_cvt::char_enc fileenc)
	: formation_(formation)// 陣形はそのまま反映させる
{
	ENCODE_THROW_WITH_MESSAGE_IF(fileenc == char_cvt::char_enc::unknown, "unknown char enc type.")//文字コード自動判別なんてやらない
	// ファイルを読み込む
	ifstream fin(file_name);
	FILE_THROW_WITH_MESSAGE_IF(!fin.is_open(), "艦隊データが正常に読み込めませんでした.")
#ifdef _WIN32
	if(char_cvt::char_enc::shift_jis != fileenc)
#endif //_WIN32
		skip_utf8_bom(fin, fileenc);
	this->LoadJson(fin, fileenc);
}

Fleet::Fleet(std::istream & file, const Formation & formation, char_cvt::char_enc fileenc)
	: formation_(formation)// 陣形はそのまま反映させる
{
	ENCODE_THROW_WITH_MESSAGE_IF(fileenc == char_cvt::char_enc::unknown, "unknown char enc type.")//文字コード自動判別なんてやらない
	this->LoadJson(file, fileenc);
}

// setter
void Fleet::SetFormation(const Formation formation) { formation_ = formation; }

void Fleet::ResizeUnit(size_t size) { unit_.resize(size); }

// getter
Formation Fleet::GetFormation() const noexcept { return formation_; }
vector<vector<Kammusu>>& Fleet::GetUnit() noexcept { return unit_; }
const vector<vector<Kammusu>>& Fleet::GetUnit() const noexcept { return unit_; }
FleetType Fleet::GetFleetType() const noexcept { return fleet_type_; }
// その他
//「艦隊数」(通常艦隊だと1、連合艦隊だと2)
size_t Fleet::FleetSize() const noexcept { return unit_.size(); }
//「艦隊」における艦数
size_t Fleet::UnitSize(const size_t fi) const noexcept { return unit_[fi].size(); }
//第一艦隊のインデックス
size_t Fleet::FirstIndex() const noexcept { return 0; }
//第ニ艦隊のインデックス
size_t Fleet::SecondIndex() const noexcept { return unit_.size() - 1; }

// 中身を表示する
void Fleet::Put() const {
	cout << *this;
}

// cond値を変更する
void Fleet::ChangeCond(const SimulateMode simulate_mode, const Result &result) noexcept {
	// 全体に適用されるもの
	int cond_change = 0;
	switch (simulate_mode) {
	case kSimulateModeDN:
		cond_change -= 3;
		if (result.GetNightFlg()) cond_change -= 2;
		break;
	case kSimulateModeD:
		cond_change -= 3;
		break;
	case kSimulateModeN:
		cond_change -= 2;
		break;
	}
	switch (result.JudgeWinReason()) {
	case WinReason::SS:
	case WinReason::S:
		cond_change += 4;
		break;
	case WinReason::A:
		cond_change += 3;
		break;
	case WinReason::B:
		cond_change += 2;
		break;
	case WinReason::C:
		cond_change += 1;
		break;
	default:
		break;
	}
	for (auto &it_u : unit_) {
		for (auto &it_k : it_u) {
			it_k.ChangeCond(cond_change);
		}
	}
	// 個別に適用されるもの
	for(size_t fi = 0; fi < FleetSize(); ++fi){
		// 艦隊旗艦は無条件でcond値+3
		unit_[fi][0].ChangeCond(3);
		// 艦隊MVPはcond値+10(敗北Eの際を除く)
		if (result.JudgeWinReason() != WinReason::E) {
			// 連合艦隊の場合、夜戦に突入すると昼戦でのダメージがMVP計算に関係しなくなる(！？)
			bool special_mvp_flg = result.GetNightFlg() && (fleet_type_ != FleetType::Normal);
			// 計算を行う
			size_t mvp_index = 0;
			int mvp_damage = result.GetDamage(0, fi, 0, special_mvp_flg);
			for (size_t ui = 1; ui < unit_[fi].size(); ++ui) {
				if (mvp_damage < result.GetDamage(0, fi, ui, special_mvp_flg)) {
					mvp_damage = result.GetDamage(0, fi, ui, special_mvp_flg);
					mvp_index = ui;
				}
			}
			unit_[fi][mvp_index].ChangeCond(10);
		}
	}
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
				case WC("艦上爆撃機"):	//艦爆
				case WC("艦上爆撃機(爆戦)"):	//艦爆
					search_sum += it_w.GetSearch() * 1.0376255;
					break;
				case WC("水上爆撃機"):	//水爆
					search_sum += it_w.GetSearch() * 1.7787282;
					break;
				case WC("艦上攻撃機"):	//艦攻
					search_sum += it_w.GetSearch() * 1.3677954;
					break;
				case WC("艦上偵察機"):	//艦偵
				case WC("艦上偵察機(彩雲)"):	//艦偵
					search_sum += it_w.GetSearch() * 1.6592780;
					break;
				case WC("水上偵察機"):	//水偵
				case WC("水上偵察機(夜偵)"):	//水偵
					search_sum += it_w.GetSearch() * 2.0000000;
					break;
				case WC("小型電探"):	//小型電探
					search_sum += it_w.GetSearch() * 1.0045358;
					break;
				case WC("大型電探"):	//大型電探
					search_sum += it_w.GetSearch() * 0.9906638;
					break;
				case WC("探照灯"):	//探照灯
				case WC("大型探照灯"):	//大型探照灯
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

//判定式(33)で索敵値を計算する
double Fleet::SearchValue_33() const noexcept
{
	double search_sum = 0.0;
	int alive_count = 0;
	//司令部レベルによる補正
	search_sum -= ceil(level_ * 0.4);
	//艦娘・装備による補正
	for (auto &it_u : unit_) {
		for (auto &it_k : it_u) {
			if (it_k.Status() == kStatusLost) continue;
			alive_count++;
			search_sum += sqrt(it_k.GetSearch());
			for (auto &it_w : it_k.GetWeapon()) {
				switch (it_w.GetWeaponClass()) {
				case WC("水上爆撃機"):	//水爆
					search_sum += it_w.GetSearch() * 1.1;
					break;
				case WC("艦上攻撃機"):	//艦攻
					search_sum += it_w.GetSearch() * 0.8;
					break;
				case WC("艦上偵察機"):	//艦偵
				case WC("艦上偵察機(彩雲)"):	//彩雲
					search_sum += it_w.GetSearch() * 1.0;
					break;
				case WC("水上偵察機"):	//水偵
				case WC("水上偵察機(夜偵)"):	//夜偵
					search_sum += (it_w.GetSearch() + sqrt(it_w.GetLevel()) * 1.2) * 1.2;
					break;
				case WC("小型電探"):	//小型電探
				case WC("大型電探"):	//大型電探
					search_sum += (it_w.GetSearch() + sqrt(it_w.GetLevel()) * 1.25) * 0.6;
					break;
				default:	//その他装備は係数0.6、改修効果は乗らないとする
					search_sum += it_w.GetSearch() * 0.6;
					break;
				}
			}
		}
	}
	//隻数による補正
	search_sum += 2 * (6 - alive_count);
	return floor(search_sum * 10.0 + 0.5) / 10.0;	//小数第2位を四捨五入
}

// 制空値を計算する
int Fleet::AntiAirScore() const noexcept {
	int anti_air_score = 0;
	for (auto &it_k : this->GetUnit().front()) {
		if (it_k.Status() == kStatusLost) continue;
		anti_air_score += it_k.SumWeapons([](const auto& it_w) {
			return it_w.AnyOf(WeaponClass::AirFight) ? it_w.AntiAirScore(it_w.GetAir()) : 0;
		});
	}
	return anti_air_score;
}

//触接開始率を計算する
double Fleet::TrailerAircraftProb(const AirWarStatus &air_war_status) const {
	// 制空権確保時の確率を計算する
	double trailer_aircraft_prob = 0.0;
	for (auto &it_k : this->GetUnit().front()) {
		if (it_k.Status() == kStatusLost) continue;
		trailer_aircraft_prob += it_k.SumWeapons([](const auto& it_w) {
			return it_w.AnyOf(WC("艦上偵察機") | WC("艦上偵察機(彩雲)") | WC("大型飛行艇") | WC("水上偵察機") | WC("水上偵察機(夜偵)")) ? 0.04 * it_w.GetSearch() * sqrt(it_w.GetAir()) : 0;
		});
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
double Fleet::TrailerAircraftPlus() const {
	const static double all_attack_plus_list[] = { 1.12, 1.12, 1.17, 1.20 };
	for (const auto &it_k :this->GetUnit().front() ) {
		if (it_k.Status() == kStatusLost) continue;
		for (auto &it_w : it_k.GetWeapon()) {
			if (!it_w.AnyOf(WeaponClass::AirTrailer) || it_w.GetAir() == 0) continue;
			if (0.07 * it_w.GetSearch() >= SharedRand::RandReal()) {
				return all_attack_plus_list[it_w.GetHit()];
			}
		}
	}
	return 1.0;
}

// 発動する対空カットインの種類を判断する
int Fleet::AacType() const {
	// まず、秋月型カットイン以外の判定を行う
	for (const auto &it_u : unit_) {
		for (auto &it_k : it_u) {
			if (it_k.Status() == kStatusLost) continue;
			auto aac_type = it_k.AacType();
			if (aac_type <= 3) continue;
			if (it_k.AacProb(aac_type) < SharedRand::RandReal()) continue;
			return aac_type;
		}
	}
	// 次に、秋月型カットインの判定を行う
	for (auto &it_u : unit_) {
		for (auto &it_k : it_u) {
			if (it_k.Status() == kStatusLost) continue;
			auto aac_type = it_k.AacType();
			if (aac_type != limit(aac_type, 1, 3)) continue;
			if (it_k.AacProb(aac_type) < SharedRand::RandReal()) continue;
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
			double anti_air_bonus = it_k.SumWeapons([](const auto& it_w) {return it_w.AntiAirBonus(); });
			fleets_anti_air_bonus += int(anti_air_bonus);
		}
	}
	return int(2 * kAntiAirBonusPer[(this->GetUnit().front()[0].IsKammusu() ? 0 : 1)][formation_] * fleets_anti_air_bonus);
}

// 生存艦から艦娘をランダムに指定する(航空戦用)
tuple<bool, size_t> Fleet::RandomKammusu() const {
	//生存艦をリストアップ
	std::array<size_t, kMaxUnitSize> alived_list;
	size_t alived_list_size = 0;
	for (size_t ui = 0; ui < this->GetUnit().front().size(); ++ui) {
		if (this->GetUnit().front()[ui].Status() != kStatusLost) {
			alived_list[alived_list_size] = ui;
			++alived_list_size;
		}
	}
	if (alived_list_size == 0) return tuple<bool, size_t>(false, 0);
	return tuple<bool, size_t>(true, SharedRand::select_random_in_range(alived_list, alived_list_size));
}

// 生存する水上艦から艦娘をランダムに指定する
// ただしhas_bombがtrueの際は陸上型棲姫を避けるようになる
tuple<bool, KammusuIndex> Fleet::RandomKammusuNonSS(const bool has_bomb, const TargetType target_type, const bool has_sl) const {
	// 攻撃する艦隊の対象を選択する
	std::array<size_t, kMaxFleetSize> list;
	size_t list_fleets = 1;
	switch (target_type) {
	case kTargetTypeFirst:
		list = {{ FirstIndex(), FirstIndex() }};
		list_fleets = 1;
		break;
	case kTargetTypeSecond:
		list = {{ SecondIndex(), SecondIndex() }};
		list_fleets = 1;
		break;
	case kTargetTypeAll:
		list = {{ FirstIndex(), SecondIndex() }};
		list_fleets = 2;
		break;
	}
	//生存する水上艦をリストアップ
	std::array<KammusuIndex, kMaxFleetSize * kMaxUnitSize> alived_list;
	std::array<double, kMaxFleetSize * kMaxUnitSize> alived_list_weight;
	size_t alived_list_size = 0;
	for (size_t li = 0; li < list_fleets; ++li) {
		size_t fi = list[li];
		for (size_t ui = 0; ui < GetUnit()[fi].size(); ++ui) {
			// 艦を選択
			const auto &it_k = GetUnit()[fi][ui];
			// 撃沈されていたら選択できない
			if (it_k.Status() == kStatusLost) continue;
			// 潜水艦なら選択できない
			if (it_k.IsSubmarine()) continue;
			// 対地攻撃絡み
			if (has_bomb && it_k.AnyOf(SC("陸上型"))) continue;
			// 1隻追加
			alived_list[alived_list_size] = { fi, ui };
			alived_list_weight[alived_list_size] = 1.0;
			// 夜戦探照灯補正
			// http://ch.nicovideo.jp/HSG/blomaga/ar1015220
			if (has_sl) {
				for (const auto &it_w : it_k.GetWeapon()) {
					if (it_w.AnyOf(WC("探照灯"))) {
						// こちらの「0.04」は推測結果
						alived_list_weight[alived_list_size] += 1.0 + 0.04 * it_w.GetLevel();
						break;
					}
					if (it_w.AnyOf(WC("大型探照灯"))) {
						// こちらの「0.04」は推測ですらない(探照灯に倣っただけ)
						alived_list_weight[alived_list_size] += 3.6 + 0.04 * it_w.GetLevel();
						break;
					}
				}
			}
			++alived_list_size;
		}
	}
	// 対象が存在しない場合はfalseを返す
	if (alived_list_size == 0) return tuple<bool, KammusuIndex>(false, { 0 , 0 });
	// 選びようがない場合
	if (alived_list_size == 1) return tuple<bool, KammusuIndex>(true, alived_list[0]);
	// 攻撃対象をルーレット選択
	double roulette_size = 0.0;
	for (size_t k = 0; k < alived_list_size; ++k) {
		roulette_size += alived_list_weight[k];
	}
	double roulette_oracle = SharedRand::RandReal(0.0, roulette_size);
	auto roulette_index = alived_list[0];
	double roulette_sum = alived_list_weight[0];
	for (size_t k = 1; k < alived_list_size; ++k) {
		if (roulette_sum > roulette_oracle) {
			roulette_index = alived_list[k];
			break;
		}
		roulette_sum += alived_list_weight[k];
	}
	return tuple<bool, KammusuIndex>(true, roulette_index);
}

// 潜水の生存艦から艦娘をランダムに指定する
tuple<bool, KammusuIndex> Fleet::RandomKammusuSS(const size_t fleet_index) const {
	INVAID_ARGUMENT_THROW_WITH_MESSAGE_IF(1 < fleet_index, "fleet_index is iregal");
	// 攻撃する艦隊の対象を選択する
	const size_t fi = (0 == fleet_index) ? FirstIndex() : SecondIndex();
	//生存する潜水艦をリストアップ
	vector<KammusuIndex> alived_list;
	for (size_t ui = 0; ui < GetUnit()[fi].size(); ++ui) {
		const auto &it_k = GetUnit()[fi][ui];
		if (it_k.Status() == kStatusLost) continue;
		if (!it_k.IsSubmarine()) continue;
		alived_list.push_back({ fi, ui });
	}
	if (alived_list.size() == 0) return tuple<bool, KammusuIndex>(false, { 0 , 0 });
	return tuple<bool, KammusuIndex>(true, SharedRand::select_random_in_range(alived_list));
}

// 陸上型の生存艦から艦娘をランダムに指定する
tuple<bool, KammusuIndex> Fleet::RandomKammusuAF(const size_t fleet_index) const {
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
	//生存する陸上型をリストアップ
	vector<KammusuIndex> alived_list;
	for (auto &fi : list) {
		for (size_t ui = 0; ui < GetUnit()[fi].size(); ++ui) {
			const auto &it_k = GetUnit()[fi][ui];
			if (it_k.Status() == kStatusLost) continue;
			if (!(it_k.AnyOf(SC("陸上型")))) continue;
			alived_list.push_back({ fi, ui });
		}
	}
	if (alived_list.size() == 0) return tuple<bool, KammusuIndex>(false, { 0 , 0 });
	return tuple<bool, KammusuIndex>(true, SharedRand::select_random_in_range(alived_list));
}

template<typename CondFunc>
bool any_of(const std::vector<std::vector<Kammusu>>& unit, CondFunc&& cond) noexcept {
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
	return std::any_of(this->GetUnit().front().begin(), this->GetUnit().front().end(), [](const Kammusu& it_k) -> bool { return it_k.HasAirTrailer(); });
}

// 彩雲をいずれかの艦が保有していた場合はtrue
bool Fleet::HasAirPss() const noexcept {
	return std::any_of(this->GetUnit().front().begin(), this->GetUnit().front().end(), [](const Kammusu& it_k) -> bool { return it_k.HasAirPss(); });
}

// 探照灯や照明弾をいずれかの艦が保有していた場合はtrue
bool Fleet::HasLights() const noexcept {
	return any_of(this->unit_, [](const Kammusu& it_k) -> bool { return it_k.HasLights(); });
}

// 大破以上の艦が存在していた場合はtrue
bool Fleet::HasHeavyDamage() const noexcept {
	return any_of(this->unit_, [](const Kammusu& it_k) -> bool { return it_k.Status() >= kStatusHeavyDamage; });
}

// 陸上型がいた場合はtrue
bool Fleet::HasAF() const noexcept {
	return any_of(this->unit_, [](const Kammusu& it_k) -> bool { return it_k.GetShipClass() == ShipClass::AF && it_k.Status() != kStatusLost; });
}

std::ostream & operator<<(std::ostream & os, const Fleet & conf)
{
	os << "陣形：" << char_cvt::wstring2string(kFormationStr[conf.formation_]) << "　司令部レベル：" << conf.level_ << "　形式：" << char_cvt::wstring2string(kFleetTypeStr[int(conf.fleet_type_) - 1]) << endl;
	for (size_t fi = 0; fi < conf.FleetSize(); ++fi) {
		os << "　第" << (fi + 1) << "艦隊：" << endl;
		for (auto &it_k : conf.unit_[fi]) {
			os << "　　" << char_cvt::wstring2string(it_k.GetNameLv()) << " " << it_k.GetHP() << "/" << it_k.GetMaxHP() <<  endl;
		}
	}
	os << endl;
	return os;
}

std::wostream & operator<<(std::wostream & os, const Fleet & conf)
{
	os << L"陣形：" << kFormationStr[conf.formation_] << L"　司令部レベル：" << conf.level_ << L"　形式：" << kFleetTypeStr[int(conf.fleet_type_) - 1] << endl;
	for (size_t fi = 0; fi < conf.FleetSize(); ++fi) {
		os << L"　第" << (fi + 1) << L"艦隊：" << endl;
		for (auto &it_k : conf.unit_[fi]) {
			os << L"　　" << it_k.GetNameLv() << L" " << it_k.GetHP() << L"/" << it_k.GetMaxHP() << endl;
		}
	}
	os << endl;
	return os;
}
