/* KanColleSimulator */

#include "base.hpp"
#include "config.hpp"
#include "other.hpp"
#include "fleet.hpp"
#include "result.hpp"
#include "simulator.hpp"
#include "random.hpp"
#include "mapdata.hpp"
#include <atomic>
#include <cassert>
#include <locale>
#include <mutex>
#include <thread>
using namespace std::chrono;

class NormalMode {
	const Config& config;
	const std::vector<Fleet> fleet;
	std::atomic<size_t> global_n{ 0 };
	std::vector<Result> result_db;

	static auto read(const Config& config) {
		// ファイルから艦隊を読み込む
		vector<Fleet> fleet;
		fleet.reserve(kBattleSize);
		assert(fleet.empty());
		for (size_t i = 0; i < kBattleSize; ++i) {
			fleet.emplace_back(config.GetInputFilename(i), config.GetFormation(i));
		}
		return fleet;
	}
public:
	NormalMode(const Config& config) : config(config), fleet(read(config)), result_db(config.GetTimes()) {
			for (const auto& f : fleet) f.Put();
	}
	void work() {
		size_t n;
		while ((n = global_n++) < config.GetTimes()) {
			Simulator simulator(fleet, kSimulateModeDN);//戦闘のたびにSimulatorインスタンスを設定する
			vector<Fleet> fleet_;
			std::tie(result_db[n], fleet_) = simulator.Calc();
		}
	}
	void show_result() {
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
};

class MapMode {
	const Config& config;
	const MapData map_Data;
	const Fleet my_fleet;
	std::mutex mutex;
	std::atomic<size_t> global_n{ 0 };
	std::vector<size_t> point_count;
	std::vector<Result> result_db;

public:
	MapMode(const Config& config) : config(config), map_Data(config.GetInputFilename(kEnemySide)), my_fleet(config.GetInputFilename(kFriendSide), kFormationTrail), point_count(map_Data.GetSize()) {
		// ファイルから艦隊とマップを読み込む
		my_fleet.Put();
		map_Data.Put();
	}
	void work() {
		auto map_data_this_thread = map_Data;
		vector<size_t> local_point_count(map_Data.GetSize());
		vector<Result> local_result_db;
		local_result_db.reserve(map_Data.GetSize() * config.GetTimes());
		size_t n;
		while ((n = global_n++) < config.GetTimes()) {
			// 自艦隊をセットする
			vector<Fleet> fleet(kBattleSize);
			fleet[kFriendSide] = my_fleet;
			// マップを進ませる
			for (size_t p = 0; p < map_data_this_thread.GetSize(); ++p) {
				++local_point_count[p];
				// 敵艦隊をセットする
				fleet[kEnemySide] = map_data_this_thread.GetFleet(p);
				// 敵艦隊の形態、および戦闘モードにより自艦隊の陣形を変更する
				if (fleet[kEnemySide].GetUnit().front().front().IsSubmarine()) {
					fleet[kFriendSide].SetFormation(kFormationAbreast);
				} else if (map_data_this_thread.GetSimulateMode(p) == kSimulateModeN) {
					fleet[kFriendSide].SetFormation(config.GetFormation(kFriendSide));
				} else {
					fleet[kFriendSide].SetFormation(kFormationTrail);
				}
				// シミュレートを行う
				Simulator simulator(fleet, map_data_this_thread.GetSimulateMode(p));
				vector<Fleet> fleet_(kBattleSize);
				Result result_;
				std::tie(result_, fleet_) = simulator.Calc();
				// 結果を元の配列に書き戻す
				fleet[kFriendSide] = fleet_[kFriendSide];
				if (p != map_data_this_thread.GetSize() - 1) {
					// 大破していたら撤退する
					if (fleet[kFriendSide].HasHeavyDamage()) break;
				} else {
					// ボスマスなら結果を記録する
					local_result_db.push_back(result_);
				}
			}
		}
		std::lock_guard<std::mutex> guard{ mutex };
		for (size_t i = 0; i < point_count.size(); i++)
			point_count[i] += local_point_count[i];
		std::copy(std::begin(local_result_db), std::end(local_result_db), std::back_inserter(result_db));
	}
	void show_result() {
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
		fleet[kEnemySide] = map_Data.GetFleet(map_Data.GetSize() - 1, 0);
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
};

template<class Mode>
void run(const time_point<high_resolution_clock>& preprocess_begin_time, Config& config) {
	Mode mode{ config };
	const auto preprocess_end_time = high_resolution_clock::now();
	cout << "preprocess:" << duration_cast<nanoseconds>(preprocess_end_time - preprocess_begin_time).count() << "[ns]\n" << endl;
	const auto process_begin_time = high_resolution_clock::now();
	if (1 < config.GetThreads()) {
		std::vector<std::thread> threads;
		threads.reserve(config.GetThreads());
		for (size_t i = 0; i < config.GetThreads(); i++)
			threads.emplace_back(&Mode::work, &mode);
		for (auto& thread : threads)
			thread.join();
	} else
		mode.work();
	const auto process_end_time = high_resolution_clock::now();
	cout << "処理時間：" << duration_cast<milliseconds>(process_end_time - process_begin_time).count() << "[ms]\n" << endl;
	mode.show_result();
}

int main(int argc, char *argv[]) {
	try {
		const auto preprocess_begin_time = high_resolution_clock::now();
		std::locale::global(std::locale("japanese"));
		std::wcout.imbue(std::locale(""));
		// 現在の設定を取得する
		Config config(argc, argv);
		config.Put();
		// ファイル拡張子により、処理内容を分岐させる
		auto ext = GetExtension(config.GetInputFilename(kEnemySide));
		if (ext == "json") {	//通常モード
			run<NormalMode>(preprocess_begin_time, config);
		}
		else if (ext == "map") {	//マップモード
			run<MapMode>(preprocess_begin_time, config);
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
