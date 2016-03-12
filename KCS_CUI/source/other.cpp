#include "base.hpp"
#include "other.hpp"
#include "fleet.hpp"
#include "result.hpp"
#include "char_convert.hpp"
#include <cctype>
#include <algorithm>

// 集計結果保存用クラスのコンストラクタ
ResultStat::ResultStat(const vector<Result> &result_db, const vector<vector<Kammusu>> &unit) noexcept {
	hp_min_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, 10000)));
	hp_max_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, -1)));
	damage_min_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, 10000)));
	damage_max_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, -1)));

	hp_ave_.resize(kBattleSize, vector<vector<double>>(kMaxFleetSize, vector<double>(kMaxUnitSize)));
	hp_sd_.resize(kBattleSize, vector<vector<double>>(kMaxFleetSize, vector<double>(kMaxUnitSize, 0.0)));
	damage_ave_.resize(kBattleSize, vector<vector<double>>(kMaxFleetSize, vector<double>(kMaxUnitSize)));
	damage_sd_.resize(kBattleSize, vector<vector<double>>(kMaxFleetSize, vector<double>(kMaxUnitSize, 0.0)));

	mvp_count_.resize(kMaxFleetSize, vector<int>(kMaxUnitSize, 0));
	heavy_damage_count_.resize(kMaxFleetSize, vector<int>(kMaxUnitSize, 0));

	win_reason_count_.resize(int(WinReason::Types), 0);

	all_count_ = result_db.size();
	reader_killed_count_ = 0;

	for (const auto& result : result_db) {
		++win_reason_count_[int(result.JudgeWinReason())];
		const bool special_mvp_flg = result.GetNightFlg() && (unit.size() > 1);
		for (size_t fi = 0; fi < unit.size(); ++fi) {
			size_t mvp_index = 0;
			int mvp_damage = -1;
			for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
				// 残り耐久・与ダメージ
				for (size_t bi = 0; bi < kBattleSize; ++bi) {
					auto hp = result.GetHP(bi, fi, ui);
					auto damage = result.GetDamage(bi, fi, ui);
					hp_min_[bi][fi][ui] = std::min(hp_min_[bi][fi][ui], hp);
					hp_max_[bi][fi][ui] = std::max(hp_max_[bi][fi][ui], hp);
					damage_min_[bi][fi][ui] = std::min(damage_min_[bi][fi][ui], damage);
					damage_max_[bi][fi][ui] = std::max(damage_max_[bi][fi][ui], damage);
					hp_ave_[bi][fi][ui] += hp;
					damage_ave_[bi][fi][ui] += damage;
				}
				// MVP・大破
				auto damage = result.GetDamage(0, fi, ui, special_mvp_flg);
				if (damage > mvp_damage) {
					mvp_index = ui;
					mvp_damage = damage;
				}
				if (result.GetHP(0, fi, ui) * 4 <= unit[fi][ui].GetMaxHP()) ++heavy_damage_count_[fi][ui];
			}
			++mvp_count_[fi][mvp_index];
		}
		// 旗艦撃破
		if (result.GetHP(1, 0, 0) == 0) ++reader_killed_count_;
	}
	// 平均
	const double all_count_inv = 1.0 / all_count_;
	for (auto& hp_ave2d : this->hp_ave_) for (auto& hp_ave1d : hp_ave2d) for (auto& hp_ave : hp_ave1d) hp_ave *= all_count_inv;
	for (auto& damage_ave2d : this->damage_ave_) for (auto& damage_ave1d : damage_ave2d) for (auto& damage_ave : damage_ave1d) damage_ave *= all_count_inv;
	// 標本標準偏差
	if (all_count_ > 1) {
		for (const auto& result : result_db) {
			for (size_t bi = 0; bi < kBattleSize; ++bi) {
				for (size_t fi = 0; fi < kMaxFleetSize; ++fi) {
					for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
						double temp1 = hp_ave_[bi][fi][ui] - result.GetHP(bi, fi, ui);
						hp_sd_[bi][fi][ui] += temp1 * temp1;
						double temp2 = damage_ave_[bi][fi][ui] - result.GetDamage(bi, fi, ui);
						damage_sd_[bi][fi][ui] += temp2 * temp2;
					}
				}
			}
		}
		const double all_count_inv2 = 1.0 / (all_count_ - 1);
		for (auto& hp_sd2d : this->hp_sd_) for (auto& hp_sd1d : hp_sd2d) for (auto& hp_sd : hp_sd1d) hp_sd = std::sqrt(hp_sd * all_count_inv2);
		for (auto& damage_sd2d : this->damage_sd_) for (auto& damage_sd1d : damage_sd2d) for (auto& damage_sd : damage_sd1d) damage_sd = std::sqrt(damage_sd * all_count_inv2);
	}
}

// 結果を標準出力に出力する
void ResultStat::Put(const vector<Fleet> &fleet) const noexcept {
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		wcout << (bi == kFriendSide ? L"自" : L"敵") << L"艦隊：" << endl;
		const auto &unit = fleet[bi].GetUnit();
		for (size_t fi = 0; fi < unit.size(); ++fi) {
			wcout << L"　第" << (fi + 1) << L"艦隊：" << endl;
			for (size_t ui = 0; ui < unit[fi].size(); ++ui) {
				wcout << L"　　" << unit[fi][ui].GetNameLv() << endl;
				wcout << L"　　　残耐久：" << L"[" << hp_min_[bi][fi][ui] << L"～" << hp_ave_[bi][fi][ui] << L"～" << hp_max_[bi][fi][ui] << L"] σ＝";
				if (all_count_ > 1) wcout << hp_sd_[bi][fi][ui] << endl; else wcout << L"―" << endl;
				wcout << L"　　　与ダメージ：" << L"[" << damage_min_[bi][fi][ui] << L"～" << damage_ave_[bi][fi][ui] << L"～" << damage_max_[bi][fi][ui] << L"] σ＝";
				if (all_count_ > 1) wcout << damage_sd_[bi][fi][ui] << endl; else wcout << L"―" << endl;
				if (bi == 0) {
					wcout << L"　　　MVP率：" << (100.0 * mvp_count_[fi][ui] / all_count_) << L"％ ";
					wcout << L"大破率：" << (100.0 * heavy_damage_count_[fi][ui] / all_count_) << L"％" << endl;
				}
			}
		}
	}
	wcout << L"旗艦撃破率：" << (100.0 * reader_killed_count_ / all_count_) << L"％" << endl;
	wcout << L"勝率：" << (100.0 * (win_reason_count_[int(WinReason::SS)] + win_reason_count_[int(WinReason::S)]
		+ win_reason_count_[int(WinReason::A)] + win_reason_count_[int(WinReason::B)]) / all_count_) << L"％" << endl;
	for (size_t i = 0; i < int(WinReason::Types); ++i) {
		wcout << L"　" << kWinReasonStrL[i] << L"：" << (100.0 * win_reason_count_[i] / all_count_) << L"％" << endl;
	}
}

// 結果をJSONファイルに出力する
void ResultStat::Put(const vector<Fleet> &fleet, const string &file_name, const bool &json_prettify_flg) const {
	ofstream fout(file_name);
	FILE_THROW_WITH_MESSAGE_IF(!fout.is_open(), "計算結果が正常に保存できませんでした.")
	picojson::object o;
	o["reader_killed_per"] = picojson::value(100.0 * reader_killed_count_ / all_count_);
	o["win_per"] = picojson::value(100.0 * (win_reason_count_[int(WinReason::SS)] + win_reason_count_[int(WinReason::S)] +
		win_reason_count_[int(WinReason::A)] + win_reason_count_[int(WinReason::B)]) / all_count_);
	picojson::object ox;
	for (size_t i = 0; i < int(WinReason::Types); ++i) {
		ox[kWinReasonStrS[i]] = picojson::value(100.0 * win_reason_count_[i] / all_count_);
	}
	o["win_reason_per"] = picojson::value(ox);
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		picojson::object o1;
		const auto &unit = fleet[bi].GetUnit();
		for (size_t fi = 0; fi < unit.size(); ++fi) {
			picojson::object o2;
			for (size_t ui = 0; ui < unit[fi].size(); ++ui) {
				picojson::object o3;
				o3["id"] = picojson::value(to_string(unit[fi][ui].GetID()));
				o3["lv"] = picojson::value(1.0 * unit[fi][ui].GetLevel());
				{
					picojson::object o4;
					o4["min"] = picojson::value(1.0 * hp_min_[bi][fi][ui]);
					o4["ave"] = picojson::value(hp_ave_[bi][fi][ui]);
					o4["max"] = picojson::value(1.0 * hp_max_[bi][fi][ui]);
					o4["sd"] = picojson::value((all_count_ > 1 ? 1.0 * hp_sd_[bi][fi][ui] : -1.0));
					o3["hp"] = picojson::value(o4);
				}
				{
					picojson::object o4;
					o4["min"] = picojson::value(1.0 * damage_min_[bi][fi][ui]);
					o4["ave"] = picojson::value(damage_ave_[bi][fi][ui]);
					o4["max"] = picojson::value(1.0 * damage_max_[bi][fi][ui]);
					o4["sd"] = picojson::value((all_count_ > 1 ? 1.0 * damage_sd_[bi][fi][ui] : -1.0));
					o3["damage"] = picojson::value(o4);
				}
				if (bi == 0) {
					o3["mvp_per"] = picojson::value(100.0 * mvp_count_[fi][ui] / all_count_);
					o3["heavy_damage_per"] = picojson::value(100.0 * heavy_damage_count_[fi][ui] / all_count_);
				}
				o2["s" + to_string(ui + 1)] = picojson::value(o3);
			}
			o1["f" + to_string(fi + 1)] = picojson::value(o2);
		}
		o["b" + to_string(bi + 1)] = picojson::value(o1);
	}

	fout << picojson::value(o).serialize(json_prettify_flg) << endl;
}

// 文字列(ファイル名)から拡張子を取り出す
// 参考：http://qiita.com/selflash/items/d6bdd0fcb677f4f8ca24
string GetExtension(const string &path) {
	// ドットの位置を検出する
	auto dot_pos = path.find_last_of('.');
	if (dot_pos == string::npos) return "";
	// 切り取る
	auto ext = path.substr(dot_pos + 1, path.find_last_not_of(' ') - dot_pos);
	// 拡張子を小文字化する
	for (auto& c : ext) c = static_cast<char>(std::tolower(c));
	return ext;
}
