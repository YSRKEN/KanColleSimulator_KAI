#include "base.hpp"
#include "fleet.hpp"
#include "mapdata.hpp"
#include "utf8bomskip.hpp"

// コンストラクタ
MapData::MapData(const string &file_name, char_cvt::char_enc fileenc) {
	// ファイルを読み込む
	ifstream fin(file_name);
	FILE_THROW_WITH_MESSAGE_IF(!fin.is_open(), "マップデータが正常に読み込めませんでした.")
	if (char_cvt::char_enc::shift_jis != fileenc) skip_utf8_bom(fin, fileenc);
	using picojson::object;
	using picojson::array;
	using picojson::value;
	value v;
	fin >> v;
	// 読み込んだJSONデータを解析する
	auto& o = v.get<object>();
	auto &o2 = o.at("position").get<array>();
	for (auto &it : o2) {
		auto &o3 = it.get<object>();
		point_name_.push_back(char_cvt::string2wstring(o3.at("name").to_str(), fileenc));
		simulate_mode_.push_back(SimulateMode(stoi(o3.at("mode").to_str())) | limit(kSimulateModeDN, kSimulateModeN));
		auto &o4 = o3.at("pattern").get<array>();
		vector<Fleet> pattern;
		for (auto &it2 : o4) {
			Fleet temp;
			auto &o5 = it2.get<object>();
			auto formation_ = Formation(stoi(o5.at("form").to_str())) | limit(kFormationTrail, kFormationAbreast);
			temp.SetFormation(formation_);
			auto &o6 = o5.at("fleets").get<picojson::array>();
			temp.GetUnit().resize(1);
			for (auto &it3 : o6) {
				auto id = stoi(it3.to_str());
				auto kammusu = Kammusu::Get(id, 1).Reset(true);
				kammusu.AacType_();	//対空カットインの種類を判別しておく
				temp.GetUnit()[0].push_back(kammusu);
			}
			pattern.push_back(temp);
		}
		fleet_.push_back(pattern);
	}
	return;
}

// getter
size_t MapData::GetSize() const noexcept { return fleet_.size(); }
Fleet MapData::GetFleet(const size_t p) const {
	return rand_.select_random_in_range(fleet_[p]);
}
Fleet MapData::GetFleet(const size_t p, const size_t n) const noexcept {
	return fleet_[p][n];
}
SimulateMode MapData::GetSimulateMode(const size_t p) const noexcept { return simulate_mode_[p]; }
wstring MapData::GetPointName(const size_t p) const noexcept { return point_name_[p]; }

SharedRand MapData::GetGenerator() noexcept { return this->rand_; }

// setter
void MapData::SetRandGenerator(const unsigned int seed) { rand_ = SharedRand(seed); }

//内容を表示する
void MapData::Put() const {
	wcout << L"マス数：" << fleet_.size() << endl;
	for (size_t p = 0; p < fleet_.size(); ++p) {
		wcout << L"　" << point_name_[p] << L"マス：" << fleet_[p].size() << L"艦隊　" << kSimulateModeStr[simulate_mode_[p]] << endl;
		for (auto &it_f : fleet_[p]) {
			wcout << L"　　";
			for (size_t ui = 0; ui < it_f.UnitSize(0); ++ui) {
				if (ui != 0) wcout << L",";
				wcout << it_f.GetUnit().front()[ui].GetName();
			}
			wcout << L"　" << kFormationStr[it_f.GetFormation()] << endl;
		}
	}
	wcout << endl;
}
