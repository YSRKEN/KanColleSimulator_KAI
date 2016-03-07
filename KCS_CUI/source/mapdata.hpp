#ifndef KCS_KAI_MAPDATA_H_
#define KCS_KAI_MAPDATA_H_

class MapData {
	vector<vector<Fleet>> fleet_;				//各マスごとの艦隊データ
	vector<BattlePosition> battle_position_;	//各マスごとの陣形
};

#endif
