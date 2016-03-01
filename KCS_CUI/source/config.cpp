#include "base.hpp"
#include "config.hpp"
#include <codecvt>
#include "char_convert.hpp"
// コンストラクタ
Config::Config(int argc, char *argv[]) {
	CONFIG_THROW_WITH_MESSAGE_IF( argc < 4, "引数の数が足りていません." )
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
			CONFIG_THROW_WITH_MESSAGE_IF( argc - i <= 2, "コマンドライン引数が異常です." )
			input_filename_[0] = argv[i + 1];
			input_filename_[1] = argv[i + 2];
			i += 2;
		}
		else if (temp == "-f") {
			// 陣形
			CONFIG_THROW_WITH_MESSAGE_IF( argc - i <= 2, "コマンドライン引数が異常です." )
			formation_[0] = static_cast<Formation>(argv[i + 1] | to_i());
			formation_[1] = static_cast<Formation>(argv[i + 2] | to_i());
			i += 2;
		}
		else if (temp == "-n") {
			// 試行回数
			CONFIG_THROW_WITH_MESSAGE_IF( argc - i <= 1, "コマンドライン引数が異常です." )
			times_ = argv[i + 1] | to_i();
			if (times_ <= 0) times_ = 1;
			++i;
		}
		else if (temp == "-t") {
			// 実行スレッド数
			CONFIG_THROW_WITH_MESSAGE_IF( argc - i <= 1, "コマンドライン引数が異常です." )
			threads_ = argv[i + 1] | to_i();
			if (threads_ <= 0) threads_ = 1;
			++i;
		}
		else if (temp == "-o") {
			// 出力ファイル名
			CONFIG_THROW_WITH_MESSAGE_IF( argc - i <= 1, "コマンドライン引数が異常です." )
			output_filename_ = argv[i + 1];
			++i;
		}
	}
	// 入力ファイル名は必須であることに注意する
	CONFIG_THROW_WITH_MESSAGE_IF(input_filename_[0] == "" && input_filename_[1] == "", "入力ファイル名は必ず指定してください." )
}

// 中身を表示する
void Config::Put() const{
	cout << *this << endl;
}

std::wstring Config::GetInputFilenameW(const int n) const
{
	return char_cvt::shift_jis_to_utf_16(this->input_filename_[n]);
}

std::ostream & operator<<(std::ostream & os, const Config & conf)
{
	os << "入力ファイル名：" << endl;
	os << "陣形指定：" << endl;
	for (auto &it : conf.formation_) {
		os << "　" << char_cvt::utf_16_to_shift_jis(kFormationStr[it]) << endl;
	}
	os
		<< "試行回数：" << conf.times_ << endl
		<< "スレッド数：" << conf.threads_ << endl
		<< "出力ファイル名：\n　" << (conf.output_filename_ != "" ? conf.output_filename_ : "<なし>") << endl;
	return os;
}

std::wostream & operator<<(std::wostream & os, const Config & conf)
{
	os << L"入力ファイル名：" << endl;
	os << L"陣形指定：" << endl;
	for (auto &it : conf.formation_) {
		os << L"　" << kFormationStr[it] << endl;
	}
	os
		<< L"試行回数：" << conf.times_ << endl
		<< L"スレッド数：" << conf.threads_ << endl
		<< L"出力ファイル名：" << endl
		<< L"　" << (conf.output_filename_ != "" ? char_cvt::shift_jis_to_utf_16(conf.output_filename_) : L"<なし>") << endl;
	return os;
}
