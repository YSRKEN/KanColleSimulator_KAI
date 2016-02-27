/* KanColleSimulator Ver.1.0 */

#include "base.hpp"
#include "config.hpp"
#include "other.hpp"
#include "fleet.hpp"

int main(int argc, char *argv[]) {
	try {
		TIME_ELAPSED(
			// 現在の設定を取得する
			Config config(argc, argv);
			config.Put();
			// データベースを読み込む
			WeaponDB weapon_db;
			KammusuDB kammusu_db;
			// ファイルから艦隊を読み込む
			vector<Fleet> fleet(kBattleSize);
			for (auto i = 0; i < kBattleSize; ++i) {
				fleet[i] = Fleet(config.InputFilename(i), config.GetFormation(i), weapon_db, kammusu_db);
				fleet[i].Put();
			}
		);
	}
	catch (const std::exception& er) {
		std::cerr << er.what() << std::endl;
	}
	catch (char *e) {
		std::cerr << "エラー：" << e << std::endl;
	}
	return 0;
}
