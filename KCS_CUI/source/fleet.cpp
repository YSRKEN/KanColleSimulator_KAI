#include "base.hpp"
#include "fleet.hpp"

// コンストラクタ
Fleet::Fleet(const string &file_name, const Formation &formation, const WeaponDB &weapon_db, const KammusuDB &kammusu_db) {
	// ファイルを読み込む

	// 陣形はそのまま反映させる
	formation_ = formation;
}

// 中身を表示する
void Fleet::Put() const {
	cout << "陣形：" << kFormationStr[formation_] << "\n";
}