#pragma once

class Result {
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
	void SetHP(const int bi, const int fi, const int ui, const int hp) noexcept { hp_[bi][fi][ui] = hp; }
	void AddDamage(const int bi, const int fi, const int ui, const int damage) noexcept { damage_[bi][fi][ui] += damage; }
	// その他
	string Put() const;
};
