#ifndef KCS_KAI_MAPDATA_H_
#define KCS_KAI_MAPDATA_H_

class MapData {
	vector<vector<Fleet>> fleet_;				//各マスごとの艦隊データ
	vector<SimulateMode> simulate_mode_;		//各マスごとの戦闘形式
public:
	// コンストラクタ
	MapData(const string &file_name, const WeaponDB &weapon_db, const KammusuDB &kammusu_db);
	// その他
	void Put();	//内容を表示する
};

#endif
