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
		cout << it.first << endl;
	}
	return;
}

//内容を表示する
void MapData::Put() {

}
