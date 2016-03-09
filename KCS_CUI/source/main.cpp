/* KanColleSimulator Ver.1.2.1 */

#include "base.hpp"
#include "config.hpp"
#include "other.hpp"
#include "fleet.hpp"
#include "result.hpp"
#include "simulator.hpp"
#include "random.hpp"
#include "mapdata.hpp"
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
			// ファイルから艦隊とマップを読み込む
			Fleet my_fleet(config.GetInputFilename(kFriendSide), kFormationTrail, weapon_db, kammusu_db);
			MapData map_Data(config.GetInputFilename(kEnemySide), weapon_db, kammusu_db);
			my_fleet.Put();
			map_Data.Put();
			// Simulatorを構築し、並列演算を行う
			auto seed = make_SharedRand().make_unique_rand_array<unsigned int>(config.CalcSeedArrSize());
			const auto process_begin_time = std::chrono::high_resolution_clock::now();
			vector<Result> result_db;
			vector<size_t> point_count(map_Data.GetSize(), 0);
			//#pragma omp parallel for num_threads(static_cast<int>(config.GetThreads()))
			for (int n = 0; n < static_cast<int>(config.GetTimes()); ++n) {
				map_Data.SetRandGenerator(seed[config.CalcSeedVNo(n)]);
				// 自艦隊をセットする
				vector<Fleet> fleet(kBattleSize);
				fleet[kFriendSide] = my_fleet;
				// マップを進ませる
				for (size_t p = 0; p < map_Data.GetSize(); ++p) {
					++point_count[p];
					// 敵艦隊をセットする
					fleet[kEnemySide] = map_Data.GetFleet(p);
					// 敵艦隊の形態、および戦闘モードにより自艦隊の陣形を変更する
					if (fleet[kEnemySide].FirstUnit()[0].IsSubmarine()) {
						fleet[kFriendSide].SetFormation(kFormationAbreast);
					}
					else if (map_Data.GetSimulateMode(p) == kSimulateModeN) {
						fleet[kFriendSide].SetFormation(config.GetFormation(kFriendSide));
					}
					else {
						fleet[kFriendSide].SetFormation(kFormationTrail);
					}
					// シミュレートを行う
					Simulator simulator(fleet, seed[config.CalcSeedVNo(n)], map_Data.GetSimulateMode(p));
					vector<Fleet> fleet_;
					Result result_;
					std::tie(result_, fleet_) = simulator.Calc();
					// 結果を元の配列に書き戻す
					fleet[kFriendSide] = fleet_[kFriendSide];
					if (p != map_Data.GetSize() - 1) {
						// 大破していたら撤退する
						if (fleet[kFriendSide].HasHeavyDamage()) break;
					}else{
						// ボスマスなら結果を記録する
						result_db.push_back(result_);
					}
				}
			}
			const auto process_end_time = std::chrono::high_resolution_clock::now();
			cout << "処理時間：" << std::chrono::duration_cast<std::chrono::milliseconds>(process_end_time - process_begin_time).count() << "[ms]\n" << endl;
			// 集計を行う
			for (size_t p = 0; p < map_Data.GetSize(); ++p) {
				wcout << map_Data.GetPointName(p) << L"マス到達率：" << (100.0 * point_count[p] / config.GetTimes()) << L"％" << endl;
				if (p != map_Data.GetSize() - 1) {
					cout << "道中撤退率：" << (100.0 * (point_count[p] - point_count[p + 1]) / point_count[p]) << "％\n" << endl;
				}
				else {
					cout << "\nボスマスにおける統計：" << endl;
				}
			}
			vector<Fleet> fleet(kBattleSize);
			fleet[kFriendSide] = my_fleet;
			fleet[kEnemySide] = map_Data.GetFleet(map_Data.GetSize() - 1);
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
