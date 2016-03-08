#ifndef KCS_KAI_INC_RESULT_HPP_
#define KCS_KAI_INC_RESULT_HPP_

// 勝利判定
enum class WinReason : std::uint_fast8_t {
	SS = 0,
	S = 1,
	A = 2,
	B = 3,
	C = 4,
	D = 5,
	E = 6,
	Types = 7,
};
const string kWinReasonStr[] = { "完全勝利SS", "勝利S", "勝利A", "戦術的勝利B", "戦術的敗北C", "敗北D", "敗北E" };
const wstring kWinReasonStrL[] = { L"完全勝利SS", L"勝利S", L"勝利A", L"戦術的勝利B", L"戦術的敗北C", L"敗北D", L"敗北E" };
const string kWinReasonStrS[] = { "SS", "S", "A", "B", "C", "D", "E" };

class Result {
	vector<vector<vector<int>>> hp_before_;
	vector<vector<vector<int>>> hp_;
	vector<vector<vector<int>>> damage_;
	vector<vector<vector<int>>> damage_night_;
	bool night_flg_;
public:
	// コンストラクタ
	Result();
	// getter
	int GetHP(const size_t bi, const size_t fi, const size_t ui) const noexcept;
	int GetDamage(const size_t bi, const size_t fi, const size_t ui, const bool special_mvp_flg = false) const noexcept;
	int GerParam(const size_t type, const size_t bi, const size_t fi, const size_t ui) const noexcept;
	bool GetNightFlg() const noexcept;
	// setter
	void SetBeforeHP(const size_t bi, const size_t fi, const size_t ui, const int hp_before) noexcept { hp_before_[bi][fi][ui] = hp_before; }
	void SetHP(const size_t bi, const size_t fi, const size_t ui, const int hp) noexcept { hp_[bi][fi][ui] = hp; }
	void AddDamage(const size_t bi, const size_t fi, const size_t ui, const int damage, const bool night_flg = false) noexcept {
		damage_[bi][fi][ui] += damage;
		if(night_flg) damage_night_[bi][fi][ui] += damage;
	}
	void SetNightFlg(const bool night_flg) noexcept { night_flg_ = night_flg; }
	// その他
	bool empty() const noexcept { return hp_before_.empty() && hp_.empty() && damage_.empty() && damage_night_.empty(); }
	string Put() const;
	// 勝利判定
	WinReason JudgeWinReason() const noexcept;
};

#endif //KCS_KAI_INC_RESULT_HPP_
