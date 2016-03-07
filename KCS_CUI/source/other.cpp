#include "base.hpp"
#include "other.hpp"
#include "fleet.hpp"
#include "result.hpp"
#include "char_convert.hpp"
#include <cctype>
#include <algorithm>
enum class CsvParseLevel : std::size_t { kLevel1 = 0, kLevel99 = 1 };
namespace detail {
	struct Split_helper_index { char delim; std::size_t index; };
	struct Split_helper {
		char delim;
		Split_helper_index operator[](std::size_t n) const noexcept { return{ delim, n }; }
		Split_helper_index operator[](CsvParseLevel n) const noexcept { return this->operator[](static_cast<std::size_t>(n)); }
	};
	string operator| (const std::string& str, Split_helper_index info) {
		std::size_t pre = 0, pos = 0;
		for (size_t i = 0; i < info.index + 1; ++i) {
			pre = pos;
			pos = str.find_first_of(info.delim, pos) + 1;
		}
		return str.substr(pre, pos - pre - 1);
	}
	vector<string> operator| (const std::string& str, Split_helper info) {
		vector<string> re;
		size_t current = 0;
		for (size_t found; (found = str.find_first_of(info.delim, current)) != string::npos; current = found + 1) {
			re.emplace_back(str, current, found - current);
		}
		re.emplace_back(str, current, str.size() - current);
		return re;
	}
	vector<string> operator| (std::string&& str, Split_helper info) {
		vector<string> re;
		size_t current = 0;
		for (size_t found; (found = str.find_first_of(info.delim, current)) != string::npos; current = found + 1) {
			re.emplace_back(str, current, found - current);
		}
		str.erase(0, current);
		re.emplace_back(std::move(str));
		return re;
	}
}
detail::Split_helper Split(char delim) noexcept { return{ delim }; }
namespace detail {
	// 文字列配列を数字配列に変換する
	inline vector<int> operator|(const vector<string> &arr_str, to_i_helper<int>) {
		vector<int> arr_int;
		for (auto &it : arr_str) {
			arr_int.push_back(it | to_i());
		}
		return arr_int;
	}
	struct ToHash_helper {};
	// 配列をハッシュに変換する
	template<typename T>
	inline unordered_map<T, size_t> operator|(const vector<T> &vec, ToHash_helper) {
		unordered_map<T, size_t> hash;
		for (size_t i = 0; i < vec.size(); ++i) {
			hash[vec[i]] = i;
		}
		return hash;
	}
}
detail::ToHash_helper ToHash() noexcept { return{}; }
// 装備DBのコンストラクタ
WeaponDB::WeaponDB(const char* csv_name) {
	// ファイルを開く
	std::locale::global(std::locale("japanese"));
	ifstream ifs(csv_name);
	FILE_THROW_WITH_MESSAGE_IF(!ifs.is_open(), string(csv_name) + "が正常に読み込めませんでした.")
	// 1行づつ読み込んでいく
	string temp_str;
	getline(ifs, temp_str);
	auto header = temp_str | Split(',') | ToHash();
	while (getline(ifs, temp_str)) {
		auto list         = temp_str | Split(',');
		auto id           = list[header.at("装備ID")] | to_i();
		auto name         = char_cvt::shift_jis_to_utf_16(list[header.at("装備名")]);
		auto weapon_class = ToWC(list[header.at("種別")]);
		auto defense      = list[header.at("装甲")] | to_i();
		auto attack       = list[header.at("火力")] | to_i();
		auto torpedo      = list[header.at("雷撃")] | to_i();
		auto bomb         = list[header.at("爆装")] | to_i();
		auto anti_air     = list[header.at("対空")] | to_i();
		auto anti_sub     = list[header.at("対潜")] | to_i();
		auto hit          = list[header.at("命中")] | to_i();
		auto evade        = list[header.at("回避")] | to_i();
		auto search       = list[header.at("索敵")] | to_i();
		auto range        = static_cast<Range>(list[header.at("射程")] | to_i());
		auto level        = 0;
		auto level_detail = 0;
		auto air          = 0;
		
		hash_[id] = Weapon(
			id, name, weapon_class, defense, attack, torpedo, bomb, anti_air,
			anti_sub, hit, evade, search, range, level, level_detail, air
		);
	}
	// ダミーデータを代入する
	hash_[-1] = Weapon();
}

// 装備DBからデータを読みだす
Weapon WeaponDB::Get(const int id) const {
	return hash_.at(id);
}
Weapon WeaponDB::Get(const int id, std::nothrow_t) const noexcept {
	try {
		return hash_.at(id);
	}
	catch (...) {
		return{};
	}
}
namespace detail{
	template<CsvParseLevel KammusuLv>Kammusu::DependOnLv PaeseCsvToKammusu(const std::unordered_map<string, std::size_t>& header, const vector<string>& list){
		const int max_hp   = list[header.at("耐久")] | Split('.')[KammusuLv] | to_i();
		const int defense  = list[header.at("装甲")] | Split('.')[KammusuLv] | to_i();
		const int attack   = list[header.at("火力")] | Split('.')[KammusuLv] | to_i();
		const int torpedo  = list[header.at("雷撃")] | Split('.')[KammusuLv] | to_i();
		const int anti_air = list[header.at("対空")] | Split('.')[KammusuLv] | to_i();
		const int evade    = list[header.at("回避")] | Split('.')[KammusuLv] | to_i();
		const int anti_sub = list[header.at("対潜")] | Split('.')[KammusuLv] | to_i();
		const int search   = list[header.at("索敵")] | Split('.')[KammusuLv] | to_i();
		return { max_hp, defense, attack, torpedo, anti_air, evade, anti_sub, search, static_cast<std::size_t>(KammusuLv) };
	}
}
// 艦娘DBのコンストラクタ
KammusuDB::KammusuDB(const char* csv_name) {
	// ファイルを開く
	std::locale::global(std::locale("japanese"));
	ifstream ifs(csv_name);
	FILE_THROW_WITH_MESSAGE_IF(!ifs.is_open(), string(csv_name) + "が正常に読み込めませんでした.")
	// 1行づつ読み込んでいく
	string temp_str;
	getline(ifs, temp_str);
	auto header = temp_str | Split(',') | ToHash();
	while (getline(ifs, temp_str)) {
		if (temp_str.find("null") != string::npos) continue;
		auto list           = temp_str | Split(',');

		const int id              = list[header.at("艦船ID")] | to_i();
		const ShipClass shipclass = static_cast<ShipClass>(1 << ((list[header.at("艦種")] | to_i()) - 1));
		const Speed speed         = list[header.at("速力")] | ToSpeed();
		const Range range         = static_cast<Range>(list[header.at("射程")] | to_i());
		const int slots           = list[header.at("スロット数")] | to_i();
		const bool kammusu_flg    = 0 != (list[header.at("艦娘フラグ")] | to_i());
		const int luck            = list[header.at("運")] | Split('.')[0] | to_i();
		
		wstring name              = char_cvt::shift_jis_to_utf_16(list[header.at("艦名")]);
		vector<int> max_airs      = list[header.at("搭載数")] | Split('.') | to_i();
		vector<int> first_weapons = list[header.at("初期装備")] | Split('.') | to_i();
		// まずLv1の方を代入する
		hash_lv1_[id]  = Kammusu(
			detail::PaeseCsvToKammusu<CsvParseLevel::kLevel1>(header, list),
			id, name, shipclass, luck, speed, range, slots, max_airs, first_weapons, kammusu_flg
		);
		// 次にLv99の方を処理する
		hash_lv99_[id] = Kammusu(
			detail::PaeseCsvToKammusu<CsvParseLevel::kLevel99>(header, list),
			id, move(name), shipclass, luck, speed, range, slots, move(max_airs), move(first_weapons), kammusu_flg
		);
	}
	// ダミーデータを代入する
	hash_lv1_[-1] = Kammusu();
	hash_lv99_[-1] = Kammusu();
}

namespace detail {
	int CalcHPWhenMarriage(const Kammusu& temp_k, const Kammusu &kammusu_lv99)
	{
		// ケッコンによる耐久上昇はややこしい
		int new_max_hp = temp_k.GetMaxHP();
		if (new_max_hp < 10) {
			new_max_hp += 3;
		}
		else if (new_max_hp < 30) {
			new_max_hp += 4;
		}
		else if (new_max_hp < 40) {
			new_max_hp += 5;
		}
		else if (new_max_hp < 50) {
			new_max_hp += 6;
		}
		else if (new_max_hp < 70) {
			new_max_hp += 7;
		}
		else if (new_max_hp <= 90) {
			new_max_hp += 8;
		}
		else {
			new_max_hp += 9;
		}
		return std::min(kammusu_lv99.GetMaxHP(), new_max_hp);
	}
}

// 艦娘DBからデータを読みだす
// idで指定した艦戦IDの艦娘を、レベルがlevelの状態にして返す
// ただし装甲・火力・雷撃・対空は改修MAXの状態とする
Kammusu KammusuDB::Get(const int id, const int level) const {
	if(hash_lv99_.find(id) == hash_lv99_.end()) return Kammusu();
	Kammusu temp_k = hash_lv99_.at(id);
	const Kammusu &kammusu_lv1 = hash_lv1_.at(id);
	const Kammusu &kammusu_lv99 = hash_lv99_.at(id);
	// 練度で上昇する箇所を補完する
	temp_k.SetMaxHP(kammusu_lv1.GetMaxHP());
	temp_k.SetEvade(int(1.0 * (kammusu_lv99.GetEvade() -kammusu_lv1.GetEvade()) * level / 99 + kammusu_lv1.GetEvade()));
	temp_k.SetAntiSub(int(1.0 * (kammusu_lv99.GetAntiSub() - kammusu_lv1.GetAntiSub()) * level / 99 + kammusu_lv1.GetAntiSub()));
	temp_k.SetSearch(int(1.0 * (kammusu_lv99.GetSearch() - kammusu_lv1.GetSearch()) * level / 99 + kammusu_lv1.GetSearch()));
	temp_k.SetLevel(level);
	if (level >= 100) {
		temp_k.SetMaxHP(detail::CalcHPWhenMarriage(temp_k, kammusu_lv99));
		// ケッコンによる運上昇は+3～+6までランダムなのでとりあえず+4とした
		temp_k.SetLuck(temp_k.GetLuck() + 4);
	}
	return temp_k;
}

// 集計結果保存用クラスのコンストラクタ
ResultStat::ResultStat(const vector<Result> &result_db, const vector<vector<Kammusu>> &unit) noexcept {
	hp_min_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, 10000)));
	hp_max_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, -1)));
	damage_min_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, 10000)));
	damage_max_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, -1)));

	hp_ave_.resize(kBattleSize, vector<vector<double>>(kMaxFleetSize, vector<double>(kMaxUnitSize)));
	hp_sd_.resize(kBattleSize, vector<vector<double>>(kMaxFleetSize, vector<double>(kMaxUnitSize, 0.0)));
	damage_ave_.resize(kBattleSize, vector<vector<double>>(kMaxFleetSize, vector<double>(kMaxUnitSize)));
	damage_sd_.resize(kBattleSize, vector<vector<double>>(kMaxFleetSize, vector<double>(kMaxUnitSize, 0.0)));

	mvp_count_.resize(kMaxFleetSize, vector<int>(kMaxUnitSize, 0));
	heavy_damage_count_.resize(kMaxFleetSize, vector<int>(kMaxUnitSize, 0));

	win_reason_count_.resize(int(WinReason::Types), 0);

	all_count_ = result_db.size();
	reader_killed_count_ = 0;

	for (size_t ti = 0; ti < all_count_; ++ti) {
		++win_reason_count_[int(result_db[ti].JudgeWinReason())];
		bool special_mvp_flg = result_db[ti].GetNightFlg() && (unit.size() > 1);
		for (size_t fi = 0; fi < unit.size(); ++fi) {
			size_t mvp_index = 0;
			int mvp_damage = -1;
			for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
				// 残り耐久・与ダメージ
				for (size_t bi = 0; bi < kBattleSize; ++bi) {
					auto hp = result_db[ti].GetHP(bi, fi, ui);
					auto damage = result_db[ti].GetDamage(bi, fi, ui);
					hp_min_[bi][fi][ui] = std::min(hp_min_[bi][fi][ui], hp);
					hp_max_[bi][fi][ui] = std::max(hp_max_[bi][fi][ui], hp);
					damage_min_[bi][fi][ui] = std::min(damage_min_[bi][fi][ui], damage);
					damage_max_[bi][fi][ui] = std::max(damage_max_[bi][fi][ui], damage);
					hp_ave_[bi][fi][ui] += hp;
					damage_ave_[bi][fi][ui] += damage;
				}
				// MVP・大破
				auto damage = result_db[ti].GetDamage(0, fi, ui, special_mvp_flg);
				if (damage > mvp_damage) {
					mvp_index = ui;
					mvp_damage = damage;
				}
				if (result_db[ti].GetHP(0, fi, ui) * 4 <= unit[fi][ui].GetMaxHP()) ++heavy_damage_count_[fi][ui];
			}
			++mvp_count_[fi][mvp_index];
		}
		// 旗艦撃破
		if (result_db[ti].GetHP(1, 0, 0) == 0) ++reader_killed_count_;
	}
	// 平均
	double all_count_inv = 1.0 / all_count_;
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		for (size_t fi = 0; fi < kMaxFleetSize; ++fi) {
			for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
				hp_ave_[bi][fi][ui] *= all_count_inv;
				damage_ave_[bi][fi][ui] *= all_count_inv;
			}
		}
	}
	// 標本標準偏差
	if (all_count_ > 1) {
		for (size_t ti = 0; ti < all_count_; ++ti) {
			for (size_t bi = 0; bi < kBattleSize; ++bi) {
				for (size_t fi = 0; fi < kMaxFleetSize; ++fi) {
					for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
						double temp1 = hp_ave_[bi][fi][ui] - result_db[ti].GetHP(bi, fi, ui);
						hp_sd_[bi][fi][ui] += temp1 * temp1;
						double temp2 = damage_ave_[bi][fi][ui] - result_db[ti].GetDamage(bi, fi, ui);
						damage_sd_[bi][fi][ui] += temp2 * temp2;
					}
				}
			}
		}
		double all_count_inv2 = 1.0 / (all_count_ - 1);
		for (size_t bi = 0; bi < kBattleSize; ++bi) {
			for (size_t fi = 0; fi < kMaxFleetSize; ++fi) {
				for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
					hp_sd_[bi][fi][ui] = sqrt(hp_sd_[bi][fi][ui] * all_count_inv2);
					damage_sd_[bi][fi][ui] = sqrt(damage_sd_[bi][fi][ui] * all_count_inv2);
				}
			}
		}
	}
}

// 結果を標準出力に出力する
void ResultStat::Put(const vector<Fleet> &fleet) const noexcept {
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		wcout << (bi == kFriendSide ? L"自" : L"敵") << L"艦隊：" << endl;
		const auto &unit = fleet[bi].GetUnit();
		for (size_t fi = 0; fi < unit.size(); ++fi) {
			wcout << L"　第" << (fi + 1) << L"艦隊：" << endl;
			for (size_t ui = 0; ui < unit[fi].size(); ++ui) {
				wcout << L"　　" << unit[fi][ui].GetNameLv() << endl;
				wcout << L"　　　残耐久：" << L"[" << hp_min_[bi][fi][ui] << L"～" << hp_ave_[bi][fi][ui] << L"～" << hp_max_[bi][fi][ui] << L"] σ＝";
				if (all_count_ > 1) wcout << hp_sd_[bi][fi][ui] << endl; else wcout << L"―" << endl;
				wcout << L"　　　与ダメージ：" << L"[" << damage_min_[bi][fi][ui] << L"～" << damage_ave_[bi][fi][ui] << L"～" << damage_max_[bi][fi][ui] << L"] σ＝";
				if (all_count_ > 1) wcout << damage_sd_[bi][fi][ui] << endl; else wcout << L"―" << endl;
				if (bi == 0) {
					wcout << L"　　　MVP率：" << (100.0 * mvp_count_[fi][ui] / all_count_) << L"％ ";
					wcout << L"大破率：" << (100.0 * heavy_damage_count_[fi][ui] / all_count_) << L"％" << endl;
				}
			}
		}
	}
	wcout << L"旗艦撃破率：" << (100.0 * reader_killed_count_ / all_count_) << L"％" << endl;
	wcout << L"勝率：" << (100.0 * (win_reason_count_[int(WinReason::SS)] + win_reason_count_[int(WinReason::S)]
		+ win_reason_count_[int(WinReason::A)] + win_reason_count_[int(WinReason::B)]) / all_count_) << L"％" << endl;
	for (size_t i = 0; i < int(WinReason::Types); ++i) {
		wcout << L"　" << kWinReasonStrL[i] << L"：" << (100.0 * win_reason_count_[i] / all_count_) << L"％" << endl;
	}
}

// 結果をJSONファイルに出力する
void ResultStat::Put(const vector<Fleet> &fleet, const string &file_name, const bool &json_prettify_flg) const {
	ofstream fout(file_name);
	FILE_THROW_WITH_MESSAGE_IF(!fout.is_open(), "計算結果が正常に保存できませんでした.")
	picojson::object o;
	o["reader_killed_per"] = picojson::value(100.0 * reader_killed_count_ / all_count_);
	o["win_per"] = picojson::value(100.0 * (win_reason_count_[int(WinReason::SS)] + win_reason_count_[int(WinReason::S)] +
		win_reason_count_[int(WinReason::A)] + win_reason_count_[int(WinReason::B)]) / all_count_);
	picojson::object ox;
	for (size_t i = 0; i < int(WinReason::Types); ++i) {
		ox[kWinReasonStrS[i]] = picojson::value(100.0 * win_reason_count_[i] / all_count_);
	}
	o["win_reason_per"] = picojson::value(ox);
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		picojson::object o1;
		const auto &unit = fleet[bi].GetUnit();
		for (size_t fi = 0; fi < unit.size(); ++fi) {
			picojson::object o2;
			for (size_t ui = 0; ui < unit[fi].size(); ++ui) {
				picojson::object o3;
				o3["id"] = picojson::value(to_string(unit[fi][ui].GetID()));
				o3["lv"] = picojson::value(1.0 * unit[fi][ui].GetLevel());
				{
					picojson::object o4;
					o4["min"] = picojson::value(1.0 * hp_min_[bi][fi][ui]);
					o4["ave"] = picojson::value(hp_ave_[bi][fi][ui]);
					o4["max"] = picojson::value(1.0 * hp_max_[bi][fi][ui]);
					o4["sd"] = picojson::value((all_count_ > 1 ? 1.0 * hp_sd_[bi][fi][ui] : -1.0));
					o3["hp"] = picojson::value(o4);
				}
				{
					picojson::object o4;
					o4["min"] = picojson::value(1.0 * damage_min_[bi][fi][ui]);
					o4["ave"] = picojson::value(damage_ave_[bi][fi][ui]);
					o4["max"] = picojson::value(1.0 * damage_max_[bi][fi][ui]);
					o4["sd"] = picojson::value((all_count_ > 1 ? 1.0 * damage_sd_[bi][fi][ui] : -1.0));
					o3["damage"] = picojson::value(o4);
				}
				if (bi == 0) {
					o3["mvp_per"] = picojson::value(100.0 * mvp_count_[fi][ui] / all_count_);
					o3["heavy_damage_per"] = picojson::value(100.0 * heavy_damage_count_[fi][ui] / all_count_);
				}
				o2["s" + to_string(ui + 1)] = picojson::value(o3);
			}
			o1["f" + to_string(fi + 1)] = picojson::value(o2);
		}
		o["b" + to_string(bi + 1)] = picojson::value(o1);
	}

	fout << picojson::value(o).serialize(json_prettify_flg) << endl;
}

// 文字列をデリミタで区切り分割する
//vector<string> Split(const string &str, char delim) {
//	vector<string> re;
//	size_t current = 0;
//	for (size_t found; (found = str.find_first_of(delim, current)) != string::npos; current = found + 1) {
//		re.emplace_back(str, current, found - current);
//	}
//	re.emplace_back(str, current, str.size() - current);
//	return re;
//}

// 文字列(ファイル名)から拡張子を取り出す
// 参考：http://qiita.com/selflash/items/d6bdd0fcb677f4f8ca24
string GetExtension(const string &path) {
	// ドットの位置を検出する
	auto dot_pos = path.find_last_of('.');
	if (dot_pos == string::npos) return "";
	// 切り取る
	auto ext = path.substr(dot_pos + 1, path.find_last_not_of(' ') - dot_pos);
	// 拡張子を小文字化する
	for (auto& c : ext) c = std::tolower(c);
	return ext;
}
