/* KanColleSimulator Ver.1.0.0 */

#include "base.hpp"
#include "config.hpp"
#include "other.hpp"
#include "fleet.hpp"
#include "result.hpp"
#include "simulator.hpp"
#include "random.hpp"
#include <cassert>
#include <omp.h>//omp_get_thread_num()
int main(int argc, char *argv[]) {
	try {
		// 現在の設定を取得する
		Config config(argc, argv);
		config.Put();
		// データベースを読み込む
		WeaponDB weapon_db("slotitems.csv");
		KammusuDB kammusu_db("ships.csv");
		// ファイル拡張子により、処理内容を分岐させる
		auto ext = GetExtension(config.GetInputFilename(kEnemySide));
		if (ext == "json") {	//通常モード
			// ファイルから艦隊を読み込む
			vector<Fleet> fleet;
			fleet.reserve(kBattleSize);
			assert(fleet.empty());
			for (size_t i = 0; i < kBattleSize; ++i) {
				fleet.emplace_back(config.GetInputFilename(i), config.GetFormation(i), weapon_db, kammusu_db);
			}
			for (const auto& f : fleet) f.Put();
			// シミュレータを構築し、並列演算を行う
			auto seed = make_SharedRand().make_unique_rand_array<unsigned int>(config.CalcSeedArrSize());
			vector<Result> result_db(config.GetTimes());
			const auto process_begin_time = std::chrono::high_resolution_clock::now();
			#pragma omp parallel for num_threads(static_cast<int>(config.GetThreads()))
			for (int n = 0; n < static_cast<int>(config.GetTimes()); ++n) {
				Simulator simulator(fleet, seed[config.CalcSeedVNo(n)], kSimulateModeDN);//戦闘のたびにSimulatorインスタンスを設定する
				vector<Fleet> fleet_;
				std::tie(result_db[n], fleet_) = simulator.Calc();
			}
			const auto process_end_time = std::chrono::high_resolution_clock::now();
			cout << "処理時間：" << std::chrono::duration_cast<std::chrono::milliseconds>(process_end_time - process_begin_time).count() << "[ms]\n" << endl;
			// 集計を行う
			ResultStat result_stat(result_db, fleet[kFriendSide].GetUnit());
			if (config.GetOutputFilename().empty()) {
				// 標準出力モード
				result_stat.Put(fleet);
			}
			else {
				// ファイル出力モード
				result_stat.Put(fleet);	//一応標準出力にも出すようにする
				result_stat.Put(fleet, config.GetOutputFilename(), config.GetJsonPrettifyFlg());
			}
		}
		else if (ext == "map") {	//マップモード
			cout << config.GetInputFilename(kEnemySide) << endl;
		}
	}
	catch (const KCS_except::successful_termination&) {
		return 0;
	}
	catch (const KCS_except::config_error& er){
		std::cerr << er.what() << endl;
		return -1;
	}
	catch (const KCS_except::file_error& er){
		std::cerr << er.what() << endl;
		return -1;
	}
	catch (const KCS_except::encode_error& er){
		std::cerr << er.what() << endl;
		return -1;
	}
	catch (const std::exception& er) {
		std::cerr << "It was stopped by unhandled exception." << endl;
		std::cerr << er.what() << endl;
		return -1;
	}
	return 0;
}
