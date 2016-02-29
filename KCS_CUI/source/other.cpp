#include "base.hpp"
#include "other.hpp"
#include "char_convert.hpp"
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
	inline vector<int> operator|(const vector<string> &arr_str, to_i_helper) {
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
		for (auto i = 0u; i < vec.size(); ++i) {
			hash[vec[i]] = i;
		}
		return hash;
	}
}
detail::ToHash_helper ToHash() noexcept { return{}; }
// 装備DBのコンストラクタ
WeaponDB::WeaponDB() {
	// ファイルを開く
	std::locale::global(std::locale("japanese"));
	ifstream ifs("slotitems.csv");
	if (!ifs.is_open()) throw "slotitems.csvが正常に読み込めませんでした.";
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
		Weapon temp_w(
			id, name, weapon_class, defense, attack, torpedo, bomb, anti_air,
			anti_sub, hit, evade, search, range, level, level_detail);
		hash_[id] = temp_w;
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
KammusuDB::KammusuDB() {
	// ファイルを開く
	std::locale::global(std::locale("japanese"));
	ifstream ifs("ships.csv");
	if (!ifs.is_open()) throw "ships.csvが正常に読み込めませんでした.";
	// 1行づつ読み込んでいく
	string temp_str;
	getline(ifs, temp_str);
	auto header = temp_str | Split(',') | ToHash();
	while (getline(ifs, temp_str)) {
		auto list           = temp_str | Split(',');

		const int id              = list[header.at("艦船ID")] | to_i();
		const ShipClass shipclass = static_cast<ShipClass>(list[header.at("艦種")] | to_i());
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
