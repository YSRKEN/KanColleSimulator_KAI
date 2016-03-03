#include "base.hpp"
#include "config.hpp"
#include <codecvt>
#include "char_convert.hpp"
#include <unordered_map>
#include <functional>
#include <array>

struct ForConfigImpl {
	std::array<string, kBattleSize> input_filename_;	//入力ファイル名
	std::array<Formation, kBattleSize> formation_;		//陣形指定
	int times_;		//試行回数
	int threads_;	//スレッド数
	string output_filename_;	//出力ファイル名
	bool json_prettify_flg_;	//出力ファイルを整形するか
};
struct Config::Impl {
	ForConfigImpl e;
};
// コンストラクタ

Config::Config() : pimpl(new Impl()){}

// コンストラクタ
Config::Config(int argc, char *argv[]) : pimpl(new Impl()) {
	CONFIG_THROW_WITH_MESSAGE_IF( argc < 4, "引数の数が足りていません." )
	// 各オプションのデフォルト値を設定する
	fill(this->pimpl->e.formation_.begin(), this->pimpl->e.formation_.end(), kFormationTrail);
	this->pimpl->e.times_ = 1;
	this->pimpl->e.threads_ = 1;
	this->pimpl->e.json_prettify_flg_ = true;
	// オプションの文字列を読み込む
	for (auto i = 1; i < argc; ++i) {
		// 一旦stringに落としこむ
		string temp(argv[i]);
		// オプション用文字列なら、各オプションに反映させる
		if (temp == "-i") {
			// 入力ファイル名
			CONFIG_THROW_WITH_MESSAGE_IF( argc - i <= 2, "コマンドライン引数が異常です." )
			this->pimpl->e.input_filename_[0] = argv[i + 1];
			this->pimpl->e.input_filename_[1] = argv[i + 2];
			i += 2;
		}
		else if (temp == "-f") {
			// 陣形
			CONFIG_THROW_WITH_MESSAGE_IF( argc - i <= 2, "コマンドライン引数が異常です." )
			this->pimpl->e.formation_[0] = static_cast<Formation>(argv[i + 1] | to_i());
			this->pimpl->e.formation_[1] = static_cast<Formation>(argv[i + 2] | to_i());
			i += 2;
		}
		else if (temp == "-n") {
			// 試行回数
			CONFIG_THROW_WITH_MESSAGE_IF( argc - i <= 1, "コマンドライン引数が異常です." )
			this->pimpl->e.times_ = argv[i + 1] | to_i();
			if (this->pimpl->e.times_ <= 0) this->pimpl->e.times_ = 1;
			++i;
		}
		else if (temp == "-t") {
			// 実行スレッド数
			CONFIG_THROW_WITH_MESSAGE_IF( argc - i <= 1, "コマンドライン引数が異常です." )
				this->pimpl->e.threads_ = argv[i + 1] | to_i();
			if (this->pimpl->e.threads_ <= 0) this->pimpl->e.threads_ = 1;
			++i;
		}
		else if (temp == "-o") {
			// 出力ファイル名
			CONFIG_THROW_WITH_MESSAGE_IF( argc - i <= 1, "コマンドライン引数が異常です." )
			this->pimpl->e.output_filename_ = argv[i + 1];
			++i;
		}
		else if (temp == "--no-result-json-prettify") {
			// 出力ファイルを整形するかのフラグ
			this->pimpl->e.json_prettify_flg_ = false;
		}
		else if (temp == "--result-json-prettify") {
			// 出力ファイルを整形するかのフラグ
			this->pimpl->e.json_prettify_flg_ = true;
		}
	}
	// 入力ファイル名は必須であることに注意する
	CONFIG_THROW_WITH_MESSAGE_IF(this->pimpl->e.input_filename_[0] == "" && this->pimpl->e.input_filename_[1] == "", "入力ファイル名は必ず指定してください." )
}

Config::~Config() = default;

Config::Config(Config && o) : pimpl(std::move(o.pimpl)) {}

Config & Config::operator=(Config && o)
{
	this->pimpl = std::move(o.pimpl);
	return *this;
}

// 中身を表示する
void Config::Put() const{
	cout << *this << endl;
}


// getter

const string & Config::GetInputFilename(const int n) const noexcept { return this->pimpl->e.input_filename_[n]; }

std::wstring Config::GetInputFilenameW(const int n) const
{
	return char_cvt::shift_jis_to_utf_16(this->pimpl->e.input_filename_[n]);
}

Formation Config::GetFormation(const int n) const noexcept { return this->pimpl->e.formation_[n]; }

int Config::GetTimes() const noexcept { return this->pimpl->e.times_; }

int Config::GetThreads() const noexcept { return this->pimpl->e.threads_; }

const string & Config::GetOutputFilename() noexcept { return this->pimpl->e.output_filename_; }

bool Config::GetJsonPrettifyFlg() const noexcept { return this->pimpl->e.json_prettify_flg_; }

std::ostream & operator<<(std::ostream & os, const Config & conf)
{
	os << "入力ファイル名：" << endl;
	os << "陣形指定：" << endl;
	for (auto &it : conf.pimpl->e.formation_) {
		os << "　" << char_cvt::utf_16_to_shift_jis(kFormationStr[it]) << endl;
	}
	os
		<< "試行回数：" << conf.pimpl->e.times_ << endl
		<< "スレッド数：" << conf.pimpl->e.threads_ << endl
		<< "出力ファイル名：\n　" << (conf.pimpl->e.output_filename_ != "" ? conf.pimpl->e.output_filename_ : "<なし>") << endl;
	return os;
}

std::wostream & operator<<(std::wostream & os, const Config & conf)
{
	os << L"入力ファイル名：" << endl;
	os << L"陣形指定：" << endl;
	for (auto &it : conf.pimpl->e.formation_) {
		os << L"　" << kFormationStr[it] << endl;
	}
	os
		<< L"試行回数：" << conf.pimpl->e.times_ << endl
		<< L"スレッド数：" << conf.pimpl->e.threads_ << endl
		<< L"出力ファイル名：" << endl
		<< L"　" << (conf.pimpl->e.output_filename_ != "" ? char_cvt::shift_jis_to_utf_16(conf.pimpl->e.output_filename_) : L"<なし>") << endl;
	return os;
}
