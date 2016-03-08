#include "base.hpp"
#include "fleet.hpp"
#include "mapdata.hpp"

// コンストラクタ
MapData::MapData(const string &file_name, const WeaponDB &weapon_db, const KammusuDB &kammusu_db) {
	// ファイルを読み込む
	ifstream fin(file_name);
	FILE_THROW_WITH_MESSAGE_IF(!fin.is_open(), "マップデータが正常に読み込めませんでした.")
	using picojson::object;
	picojson::value v;
	fin >> v;
	// 読み込んだJSONデータを解析する
	auto& o = v.get<object>();
	for (auto &it : o) {
		if (it.first == "version") continue;
		point_name_.push_back(it.first);
		auto &o2 = it.second.get<picojson::object>();
		simulate_mode_.push_back(SimulateMode(stoi(o2.at("mode").to_str())) | limit(kSimulateModeDN, kSimulateModeN));
		auto &o3 = o2.at("pattern").get<picojson::array>();
		vector<Fleet> pattern;
		for (auto &it2 : o3) {
			Fleet temp;
			auto &o4 = it2.get<picojson::object>();
			auto formation_ = Formation(stoi(o4.at("form").to_str())) | limit(kFormationTrail, kFormationAbreast);
			temp.SetFormation(formation_);
			auto &o5 = o4.at("fleets").get<picojson::array>();
			for (auto &it3 : o5) {
				auto id = stoi(it3.to_str());
				auto kammusu = kammusu_db.Get(id, 1).Reset(weapon_db);
				temp.GetUnit().resize(1);
				temp.GetUnit()[0].push_back(kammusu);
			}
			pattern.push_back(temp);
		}
		fleet_.push_back(pattern);
	}
	return;
}

//内容を表示する
void MapData::Put() {

}
