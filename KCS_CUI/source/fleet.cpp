#include "base.hpp"
#include "fleet.hpp"

// コンストラクタ
Fleet::Fleet(const string &file_name, const Formation &formation, const WeaponDB &weapon_db, const KammusuDB &kammusu_db) {
	// 陣形はそのまま反映させる
	formation_ = formation;
	// ファイルを読み込む
	ifstream fin(file_name);
	if (!fin.is_open()) throw "艦隊データが正常に読み込めませんでした.";
	value v;
	fin >> v;
	// 読み込んだJSONデータを解析する
	auto& o = v.get<object>();
	//司令部レベル
	if (o.find("lv") != o.end()) {
		level_ = limit(stoi(o["lv"].to_str()), 1, 120);
	}
	else {
		level_ = 120;
	}
	//艦隊の形式
	if (o.find("type") != o.end()) {
		fleet_type_ = limit(FleetType(stoi(o["type"].to_str())), kFleetTypeNormal, kFleetTypeCombined);
	}
	else {
		fleet_type_ = kFleetTypeNormal;
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
			auto id = stoi(unit["id"].to_str());
			auto level = limit(stoi(unit["lv"].to_str()), 1, 155);	//上限はいつか変わるかも？
			Kammusu temp_k = kammusu_db.Get(id, level).Reset();
			auto luck = limit(stoi(unit["luck"].to_str()), 0, 100);
			temp_k.SetLuck(luck);
			if (unit.find("cond") != unit.end()) {
				temp_k.SetCond(limit(stoi(unit["cond"].to_str()), 0, 100));
			}
			// 装備ID・改修/熟練度・内部熟練度から装備を設定する
			int wi = 0;
			for (auto &temp_p : unit["items"].get<object>()) {
				auto& parts = temp_p.second.get<object>();
				id = stoi(parts["id"].to_str());
				Weapon temp_w = weapon_db.Get(id);
				// 改修・外部熟練度・内部熟練度の処理
				if (temp_w.IsAir()) {
					level = limit(stoi(parts["rf"].to_str()), 0, 7);
					int level_detail = 0;
					if (parts.find("rf_detail") != parts.end()) {
						level_detail = limit(stoi(parts["rf_detail"].to_str()), 0, 120);
						level = ConvertDtoS(level_detail);
					}
					else {
						level_detail = ConvertStoD(level);
					}
					temp_w.SetLevelDetail(level_detail);
				}
				else {
					level = limit(stoi(parts["rf"].to_str()), 0, 10);
				}
				temp_w.SetLevel(level);
				// 艦娘に装備させる
				temp_k.SetWeapon(wi, temp_w);
				++wi;
				if (wi >= temp_k.Slots()) break;
			}
			// リストに加える
			unit_[fi].push_back(temp_k);
		}
		++fi;
	}
}

// 中身を表示する
void Fleet::Put() const {
	cout << "陣形：" << kFormationStr[formation_] << "　司令部レベル：" << level_ << "　形式：" << kFleetTypeStr[fleet_type_ - 1] << "\n";
	for (auto fi = 0; fi < fleet_type_; ++fi) {
		cout << "　第" << (fi + 1) << "艦隊：\n";
		for (auto &it_k : unit_[fi]) {
			cout << "　　" << it_k.PutName() << "\n";
		}
	}
	cout << std::flush;
}
