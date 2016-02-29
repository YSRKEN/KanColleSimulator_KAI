﻿#include "base.hpp"
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
	if (fileenc == char_cvt::char_enc::unknown) throw std::runtime_error("unknown char enc type.");//文字コード自動判別なんてやらない
	//本当はobjectの検索keyはfileencによってbyte列が違うので文字列テーブルを作らなければならないが、幸いアルファベットのみなのでその作業をサボることにした
	picojson::value v;
	file >> v;
	// 読み込んだJSONデータを解析する
	auto& o = v.get<object>();
	//司令部レベル
	level_ = o | GetWithLimitOrDefault("lv", 1, 120, 120);
	//艦隊の形式
	fleet_type_ = o | GetWithLimitOrDefault("type", kFleetTypeNormal, kFleetTypeCombined, kFleetTypeNormal);
	if (fleet_type_ == kFleetTypeCombined && formation_ == kFormationEchelon) {
		// 連合艦隊に梯形陣は存在しないので、とりあえず単横陣(第一警戒航行序列)に変更しておく
		formation_ = kFormationAbreast;
	}
	unit_.resize(fleet_type_);
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
			auto id = unit["id"].to_str() | to_i();
			auto level = unit["lv"].to_str() | to_i_limit(1, 155);	//上限はいつか変わるかも？
			Kammusu temp_k = kammusu_db.Get(id, level).Reset();
			auto luck = unit["luck"].to_str() | to_i_limit(0, 100);
			temp_k.SetLuck(luck);
			if (unit.find("cond") != unit.end()) {
				temp_k.SetCond(unit["cond"].to_str() | to_i_limit(0, 100));
			}
			// 装備ID・改修/熟練度・内部熟練度から装備を設定する
			int wi = 0;
			for (auto &temp_p : unit["items"].get<object>()) {
				auto& parts = temp_p.second.get<object>();
				Weapon temp_w = weapon_db.Get(parts["id"].to_str() | to_i());
				// 改修・外部熟練度・内部熟練度の処理
				if (temp_w.IsAir()) {
					level = parts["rf"].to_str() | to_i_limit(0, 7);
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
					level = parts["rf"].to_str() | to_i_limit(0, 10);
				}
				temp_w.SetLevel(level);
				// 艦娘に装備させる
				temp_k.SetWeapon(wi, temp_w);
				++wi;
				if (wi >= temp_k.GetSlots()) break;
			}
			// リストに加える
			unit_[fi].push_back(temp_k);
		}
		++fi;
	}
}
// コンストラクタ
Fleet::Fleet(const string &file_name, const Formation &formation, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, char_cvt::char_enc fileenc) 
	: formation_(formation)// 陣形はそのまま反映させる
{
	if (fileenc == char_cvt::char_enc::unknown) throw std::runtime_error("unknown char enc type.");//文字コード自動判別なんてやらない
	// ファイルを読み込む
	ifstream fin(file_name);
	if (!fin.is_open()) throw "艦隊データが正常に読み込めませんでした.";
	if(char_cvt::char_enc::shift_jis != fileenc) skip_utf8_bom(fin, fileenc);
	this->LoadJson(fin, weapon_db, kammusu_db, fileenc);
}

Fleet::Fleet(std::istream & file, const Formation & formation, const WeaponDB & weapon_db, const KammusuDB & kammusu_db, char_cvt::char_enc fileenc)
	: formation_(formation)// 陣形はそのまま反映させる
{
	if (fileenc == char_cvt::char_enc::unknown) throw std::runtime_error("unknown char enc type.");//文字コード自動判別なんてやらない
	this->LoadJson(file, weapon_db, kammusu_db, fileenc);
}

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
	for (auto &it_u : unit_) {
		for (auto &it_k : it_u) {
			for (auto wi = 0; wi < it_k.GetSlots(); ++wi) {
				if (!it_k.GetWeapon()[wi].IsAirFight()) continue;
				anti_air_score += it_k.GetWeapon()[wi].AntiAirScore(it_k.GetAir()[wi]);
			}
		}
	}
	return anti_air_score;
}

//触接開始率を計算する
double Fleet::TrailerAircraftProb(const AirWarStatus &air_war_status) const {
	// 制空権確保時の確率を計算する
	double trailer_aircraft_prob = 0.0;
	for (auto &it_u : unit_) {
		for (auto &it_k : it_u) {
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

// 艦載機をいずれかの艦が保有していた場合はtrue
bool Fleet::HasAir() const noexcept {
	for (auto &it_u : unit_) {
		for (auto &it_k : it_u) {
			if (it_k.HasAir()) return true;
		}
	}
	return false;
}

// 航空戦に参加する艦載機をいずれかの艦が保有していた場合はtrue
bool Fleet::HasAirFight() const noexcept {
	for (auto &it_u : unit_) {
		for (auto &it_k : it_u) {
			if (it_k.HasAirFight()) return true;
		}
	}
	return false;
}

// 触接に参加する艦載機をいずれかの艦が保有していた場合はtrue
bool Fleet::HasAirTrailer() const noexcept {
	for (auto &it_u : unit_) {
		for (auto &it_k : it_u) {
			if (it_k.HasAirTrailer()) return true;
		}
	}
	return false;
}

std::ostream & operator<<(std::ostream & os, const Fleet & conf)
{
	os << "陣形：" << char_cvt::utf_16_to_shift_jis(kFormationStr[conf.formation_]) << "　司令部レベル：" << conf.level_ << "　形式：" << char_cvt::utf_16_to_shift_jis(kFleetTypeStr[conf.fleet_type_ - 1]) << endl;
	for (auto fi = 0; fi < conf.fleet_type_; ++fi) {
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
	for (auto fi = 0; fi < conf.fleet_type_; ++fi) {
		os << L"　第" << (fi + 1) << L"艦隊：" << endl;
		for (auto &it_k : conf.unit_[fi]) {
			os << L"　　" << it_k.GetNameLv() << endl;
		}
	}
	os << endl;
	return os;
}
