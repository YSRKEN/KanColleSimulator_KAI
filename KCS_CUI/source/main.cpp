/* KanColleSimulator Ver.1.0 */

#include "base.hpp"
#include "config.hpp"
#include "other.hpp"
#include "fleet.hpp"
#include "result.hpp"
#include "simulator.hpp"

int main(int argc, char *argv[]) {
	try {
//		TIME_ELAPSED(
			// 現在の設定を取得する
			Config config(argc, argv);
			config.Put();
			// データベースを読み込む
			WeaponDB weapon_db;
			weapon_db.Get(124).Put();
			KammusuDB kammusu_db;
			kammusu_db.Get(450, 155).Reset(weapon_db).Put();
			// ファイルから艦隊を読み込む
			vector<Fleet> fleet(kBattleSize);
			for (auto i = 0; i < kBattleSize; ++i) {
				fleet[i] = Fleet(config.GetInputFilename(i), config.GetFormation(i), weapon_db, kammusu_db);
				fleet[i].Put();
			}
			// シミュレータを構築し、並列演算を行う
			std::random_device rd;
			auto seed = rd();
			vector<Result> result_db(config.GetTimes());
			//#pragma omp parallel for num_threads(config.Threads())
			for (int n = 0; n < config.GetTimes(); ++n) {
				Simulator simulator(fleet, seed + n);
				result_db[n] = simulator.Calc();
			}
			// 集計を行う
			for (int n = 0; n < config.GetTimes(); ++n) {
				cout << result_db[n].Put() << "\n";
			}
//		);
	}
	catch (char *e) {
		std::cerr << "エラー：" << e << "\n";
	}
	return 0;
}
