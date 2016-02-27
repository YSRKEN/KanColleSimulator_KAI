#include "base.hpp"
#include "config.hpp"

// コンストラクタ
Config::Config(int argc, char *argv[]) {
	if (argc < 4) {
		throw "引数の数が足りていません.";
	}
	// 各オプションのデフォルト値を設定する
	input_filename_.resize(kBattleSize);
	formation_.resize(kBattleSize);
	fill(formation_.begin(), formation_.end(), kFormationTrail);
	times_ = 1;
	threads_ = 1;
	// オプションの文字列を読み込む
	for (auto i = 1; i < argc; ++i) {
		// 一旦stringに落としこむ
		string temp(argv[i]);
		// オプション用文字列なら、各オプションに反映させる
		if (temp == "-i") {
			// 入力ファイル名
			if (argc - i <= 2) throw "コマンドライン引数が異常です.";
			input_filename_[0] = argv[i + 1];
			input_filename_[1] = argv[i + 2];
			i += 2;
		}
		else if (temp == "-f") {
			// 陣形
			if (argc - i <= 2) throw "コマンドライン引数が異常です.";
			formation_[0] = static_cast<Formation>(stoi(argv[i + 1]));
			formation_[1] = static_cast<Formation>(stoi(argv[i + 2]));
			i += 2;
		}
		else if (temp == "-n") {
			// 試行回数
			if (argc - i <= 1) throw "コマンドライン引数が異常です.";
			times_ = stoi(argv[i + 1]);
			if (times_ <= 0) times_ = 1;
			++i;
		}
		else if (temp == "-t") {
			// 実行スレッド数
			if (argc - i <= 1) throw "コマンドライン引数が異常です.";
			threads_ = stoi(argv[i + 1]);
			if (threads_ <= 0) threads_ = 1;
			++i;
		}
		else if (temp == "-o") {
			// 出力ファイル名
			if (argc - i <= 1) throw "コマンドライン引数が異常です.";
			output_filename_ = argv[i + 1];
			++i;
		}
	}
	// 入力ファイル名は必須であることに注意する
	if (input_filename_[0] == "" && input_filename_[1] == "") {
		throw "入力ファイル名は必ず指定してください.";
	}
}

// 中身を表示する
void Config::Put() const{
	cout << "入力ファイル名：\n";
	for (auto &it : input_filename_) {
		cout << "　" << it << "\n";
	}
	cout << "陣形指定：\n";
	for (auto &it : formation_) {
		cout << "　" << kFormationStr[it] << "\n";
	}
	cout << "試行回数：" << times_ << "\n";
	cout << "スレッド数：" << threads_ << "\n";
	cout << "出力ファイル名：\n　" << (output_filename_ != "" ? output_filename_ : "<なし>") << "\n\n";
}
