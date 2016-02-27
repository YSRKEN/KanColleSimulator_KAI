#include "base.hpp"
#include "other.hpp"

// 装備DBのコンストラクタ
WeaponDB::WeaponDB() {
	// ファイルを開く
	std::locale::global(std::locale("japanese"));
	ifstream ifs("slotitems.csv");
	if (!ifs.is_open()) throw "slotitems.csvが正常に読み込めませんでした.";
	// 1行づつ読み込んでいく
	string temp_str;
	getline(ifs, temp_str);
	auto header = ToHash(Split(temp_str, ','));
	while (getline(ifs, temp_str)) {
		auto list         = Split(temp_str, ',');
		auto id           = stoi(list[header.at("装備ID")]);
		auto name         = list[header.at("装備名")];
		auto weapon_class = ToWC(list[header.at("種別")]);
		auto defense      = stoi(list[header.at("装甲")]);
		auto attack       = stoi(list[header.at("火力")]);
		auto torpedo      = stoi(list[header.at("雷撃")]);
		auto bomb         = stoi(list[header.at("爆装")]);
		auto anti_air     = stoi(list[header.at("対空")]);
		auto anti_sub     = stoi(list[header.at("対潜")]);
		auto hit          = stoi(list[header.at("命中")]);
		auto evade        = stoi(list[header.at("回避")]);
		auto search       = stoi(list[header.at("索敵")]);
		auto range        = static_cast<Range>(stoi(list[header.at("射程")]));
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
Weapon WeaponDB::Get(const int id) const{
	if (hash_.find(id) != hash_.end()) {
		return hash_.at(id);
	}
	else {
		return hash_.at(-1);
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
	auto header = ToHash(Split(temp_str, ','));
	while (getline(ifs, temp_str)) {
		// まずLv1の方を代入する
		auto list = Split(temp_str, ',');
		auto id = stoi(list[header.at("艦船ID")]);
		auto name = list[header.at("艦名")];
		auto shipclass = static_cast<ShipClass>(stoi(list[header.at("艦種")]));
		auto max_hp = stoi(Split(list[header.at("耐久")], '.')[0]);
		auto defense = stoi(Split(list[header.at("装甲")], '.')[0]);
		auto attack = stoi(Split(list[header.at("火力")], '.')[0]);
		auto torpedo = stoi(Split(list[header.at("雷撃")], '.')[0]);
		auto anti_air = stoi(Split(list[header.at("対空")], '.')[0]);
		auto luck = stoi(Split(list[header.at("運")], '.')[0]);
		auto speed = ToSpeed(list[header.at("速力")]);
		auto range = static_cast<Range>(stoi(list[header.at("射程")]));
		auto slots = stoi(list[header.at("スロット数")]);
		auto max_airs = ToInt(Split(list[header.at("搭載数")], '.'));
		auto evade = stoi(Split(list[header.at("回避")], '.')[0]);
		auto anti_sub = stoi(Split(list[header.at("対潜")], '.')[0]);
		auto search = stoi(Split(list[header.at("索敵")], '.')[0]);
		auto first_weapons = ToInt(Split(list[header.at("初期装備")], '.'));
		auto kammusu_flg = (stoi(list[header.at("艦娘フラグ")]) == 1);
		Kammusu temp_k1(id, name, shipclass, max_hp, defense, attack, torpedo, anti_air, luck, speed,
			range, slots, max_airs, evade, anti_sub, search, first_weapons, kammusu_flg, 1);
		hash_lv1_[id] = move(temp_k1);
		// 次にLv99の方を処理する
		max_hp = stoi(Split(list[header.at("耐久")], '.')[1]);
		defense = stoi(Split(list[header.at("装甲")], '.')[1]);
		attack = stoi(Split(list[header.at("火力")], '.')[1]);
		torpedo = stoi(Split(list[header.at("雷撃")], '.')[1]);
		anti_air = stoi(Split(list[header.at("対空")], '.')[1]);
		evade = stoi(Split(list[header.at("回避")], '.')[1]);
		anti_sub = stoi(Split(list[header.at("対潜")], '.')[1]);
		search = stoi(Split(list[header.at("索敵")], '.')[1]);
		Kammusu temp_k2(id, name, shipclass, max_hp, defense, attack, torpedo, anti_air, luck, speed,
			range, slots, max_airs, evade, anti_sub, search, first_weapons, kammusu_flg, 99);
		hash_lv99_[id] = move(temp_k2);
	}
	// ダミーデータを代入する
	hash_lv1_[-1] = Kammusu();
	hash_lv99_[-1] = Kammusu();
}

// 艦娘DBからデータを読みだす
// idで指定した艦戦IDの艦娘を、レベルがlevelの状態にして返す
// ただし装甲・火力・雷撃・対空は改修MAXの状態とする
Kammusu KammusuDB::Get(const int id, const int level) const {
	if(hash_lv99_.find(id) == hash_lv99_.end()) return Kammusu();
	Kammusu temp_k = hash_lv99_.at(id);
	const Kammusu &kammusu_lv1 = hash_lv1_.at(id), &kammusu_lv99 = hash_lv99_.at(id);
	// 練度で上昇する箇所を補完する
	temp_k.SetMaxHP(kammusu_lv1.MaxHP());
	temp_k.SetEvade(int(1.0 * (kammusu_lv99.Evade() -kammusu_lv1.Evade()) * level / 99 + kammusu_lv1.Evade()));
	temp_k.SetAntiSub(int(1.0 * (kammusu_lv99.AntiSub() - kammusu_lv1.AntiSub()) * level / 99 + kammusu_lv1.AntiSub()));
	temp_k.SetSearch(int(1.0 * (kammusu_lv99.Search() - kammusu_lv1.Search()) * level / 99 + kammusu_lv1.Search()));
	temp_k.SetLevel(level);
	if (level >= 100) {
		// ケッコンによる耐久上昇はややこしい
		int new_max_hp = temp_k.MaxHP();
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
		if (new_max_hp > kammusu_lv99.MaxHP()) new_max_hp = kammusu_lv99.MaxHP();
		temp_k.SetMaxHP(new_max_hp);
		// ケッコンによる運上昇は+3～+6までランダムなのでとりあえず+4とした
		temp_k.SetLuck(temp_k.Luck() + 4);
	}
	return temp_k;
}

// 文字列をデリミタで区切り分割する
vector<string> Split(const string &str, const char &delim) {
	vector<string> list;
	std::istringstream iss(str);
	string temp;
	while (getline(iss, temp, delim)) {
		list.push_back(temp);
	}
	return list;
}

// 文字列配列を数字配列に変換する
vector<int> ToInt(const vector<string> &arr_str) {
	vector<int> arr_int;
	for (auto &it : arr_str) {
		arr_int.push_back(stoi(it));
	}
	return arr_int;
}

// 配列をハッシュに変換する
template<typename T>
unordered_map<T, size_t> ToHash(const vector<T> &vec) {
	unordered_map<T, size_t> hash;
	for (auto i = 0u; i < vec.size(); ++i) {
		hash[vec[i]] = i;
	}
	return hash;
}
