﻿#ifndef KCS_KAI_MAPDATA_H_
#define KCS_KAI_MAPDATA_H_
#include "char_convert.hpp"

class MapData {
	vector<vector<Fleet>> fleet_;				//各マスごとの艦隊データ
	vector<SimulateMode> simulate_mode_;		//各マスごとの戦闘形式
	vector<wstring> point_name_;				//各マスの名前
public:
	// コンストラクタ
	MapData(const string &file_name, char_cvt::char_enc fileenc = char_cvt::char_enc::utf8);
	// getter
	size_t GetSize() const noexcept;
	Fleet GetFleet(const size_t) const;
	Fleet GetFleet(const size_t, const size_t) const noexcept;
	SimulateMode GetSimulateMode(const size_t) const noexcept;
	wstring GetPointName(const size_t) const noexcept;
	// その他
	void Put() const;	//内容を表示する
};

#endif
