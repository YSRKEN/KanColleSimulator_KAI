#ifndef KCS_KAI_MAPDATA_H_
#define KCS_KAI_MAPDATA_H_
#include "char_convert.hpp"

class MapData {
	vector<vector<Fleet>> fleet_;				//各マスごとの艦隊データ
	vector<SimulateMode> simulate_mode_;		//各マスごとの戦闘形式
	vector<wstring> point_name_;				//各マスの名前
	SharedRand rand_;							//編成に使用する乱数生成器
public:
	// コンストラクタ
	MapData(const string &file_name, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, char_cvt::char_enc fileenc = char_cvt::char_enc::utf8);
	// getter
	size_t GetSize() const noexcept;
	Fleet GetFleet(const size_t) const;
	Fleet GetFleet(const size_t, const size_t) const noexcept;
	SimulateMode GetSimulateMode(const size_t) const noexcept;
	wstring GetPointName(const size_t) const noexcept;
	SharedRand GetGenerator() noexcept;
	// setter
	void SetRandGenerator(const unsigned int seed);
	// その他
	void Put();	//内容を表示する
};

#endif
