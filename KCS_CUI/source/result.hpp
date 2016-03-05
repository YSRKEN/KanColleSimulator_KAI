#pragma once

// 勝利判定
enum class WinReason : std::uint_fast8_t {
	SS = 0,
	S = 1,
	A = 2,
	B = 3,
	C = 4,
	D = 5,
	E = 6,
};

class Result {
	vector<vector<vector<int>>> hp_before_;
	vector<vector<vector<int>>> hp_;
	vector<vector<vector<int>>> damage_;
public:
	// コンストラクタ
	Result();
	// getter
	int GetHP(const int bi, const int fi, const int ui) const noexcept;
	int GetDamage(const int bi, const int fi, const int ui) const noexcept;
	int GerParam(const int type, const int bi, const int fi, const int ui) const noexcept;
	// setter
	void SetBeforeHP(const int bi, const int fi, const int ui, const int hp_before) noexcept { hp_before_[bi][fi][ui] = hp_before; }
	void SetHP(const int bi, const int fi, const int ui, const int hp) noexcept { hp_[bi][fi][ui] = hp; }
	void AddDamage(const int bi, const int fi, const int ui, const int damage) noexcept { damage_[bi][fi][ui] += damage; }
	// その他
	string Put() const;
	// 勝利判定
	WinReason JudgeWinReason() const noexcept;
};
