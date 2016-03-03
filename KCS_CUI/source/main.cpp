/* KanColleSimulator Ver.1.0 */

#include "base.hpp"
#include "config.hpp"
#include "other.hpp"
#include "fleet.hpp"
#include "result.hpp"
#include "simulator.hpp"
#define KCS_MEASURE_PROCESS_TIME
int main(int argc, char *argv[]) {
	try {
		// 現在の設定を取得する
		Config config(argc, argv);
		config.Put();
		// データベースを読み込む
		WeaponDB weapon_db;
		KammusuDB kammusu_db;
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
#if defined(KCS_MEASURE_PROCESS_TIME)
		const auto process_begin_time = std::chrono::high_resolution_clock::now();
#endif
		#pragma omp parallel for num_threads(config.GetThreads())
		for (int n = 0; n < config.GetTimes(); ++n) {
			Simulator simulator(fleet, seed + n, kSimulateModeDN);
			result_db[n] = simulator.Calc();
		}
#if defined(KCS_MEASURE_PROCESS_TIME)
		const auto process_end_time = std::chrono::high_resolution_clock::now();
		cout << "処理時間：" << std::chrono::duration_cast<std::chrono::milliseconds>(process_end_time - process_begin_time).count() << "[ms]\n" << endl;
#endif
		// 集計を行う
		ResultStat result_stat(result_db, fleet[kFriendSide].GetUnit());
		if (config.GetOutputFilename() == "") {
			// 標準出力モード
			result_stat.Put(fleet);
		}
		else {
			// ファイル出力モード
			result_stat.Put(fleet);	//一応標準出力にも出すようにする
			result_stat.Put(fleet, config.GetOutputFilename(), config.GetJsonPrettifyFlg());
		}
	}
	catch (const KCS_except::config_error& er){
		std::cerr << er.what() << endl;
	}
	catch (const KCS_except::file_error& er){
		std::cerr << er.what() << endl;
	}
	catch (const KCS_except::encode_error& er){
		std::cerr << er.what() << endl;
	}
	catch (const std::exception& er) {
		std::cerr << "It was stopped by unhandled exception." << endl;
		std::cerr << er.what() << endl;
	}
	return 0;
}
