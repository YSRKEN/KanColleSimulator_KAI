#include "base.hpp"
#include "kammusu.hpp"

// �R���X�g���N�^
Kammusu::Kammusu() :
	id_(-1), name_("�Ȃ�"), shipclass_(kShipClassDD), max_hp_(0), defense_(0), attack_(0),
	torpedo_(0), anti_air_(0), luck_(0), speed_(kSpeedNone), range_(kRangeNone), slots_(0),
	max_airs_({ 0, 0, 0, 0, 0 }), evade_(0), anti_sub_(0), search_(0), first_weapons_({ -1, -1, -1, -1, -1 }),
	kammusu_flg_(true), level_(1) {}

Kammusu::Kammusu(
	const int id, const string name, const ShipClass shipclass, const int max_hp, const int defense,
	const int attack, const int torpedo, const int anti_air, const int luck, const Speed speed,
	const Range range, const int slots, const vector<int> max_airs, const int evade, const int anti_sub,
	const int search, const vector<int> first_weapons, const bool kammusu_flg, const int level) :
	id_(id), name_(name), shipclass_(shipclass), max_hp_(max_hp), defense_(defense), attack_(attack),
	torpedo_(torpedo), anti_air_(anti_air), luck_(luck), speed_(speed), range_(range), slots_(slots),
	max_airs_(max_airs), evade_(evade), anti_sub_(anti_sub), search_(search), first_weapons_(first_weapons),
	kammusu_flg_(kammusu_flg), level_(level) {}

// ���g��\������
void Kammusu::Put() {
	cout << "�͑DID�F" << id_ << "\n";
	cout << "�@�͖��F" << name_ << "�@�͎�F" << kShipClassStr[shipclass_] << "\n";
	cout << "�@�ő�ϋv�F" << max_hp_ << "�@���b�F" << defense_ << "�@�ΉF" << attack_ << "�@�����F" << torpedo_ << "\n";
	cout << "�@�΋�F" << anti_air_ << "�@�^�F" << luck_ << "�@���́F" << kSpeedStr[speed_] << "�@�˒��F" << kRangeStr[range_] << "\n";
	cout << "�@�X���b�g���F" << slots_ << "�@�ő哋�ڐ��F";
	for (auto i = 0; i < slots_; ++i) {
		if (i != 0) cout << ",";
		cout << max_airs_[i];
	}
	cout << "�@����F" << evade_ << "�@�ΐ��F" << anti_sub_ << "\n";
	cout << "�@���G�F" << search_ << "�@�͖����H�F" << (kammusu_flg_ ? "�͂�" : "������") << "�@���x���F" << level_ << "�@���ϋv�F" << hp_ << "\n";
	cout << "�@�����F";
	for (auto i = 0; i < slots_; ++i) {
		if (i != 0) cout << ",";
		cout << weapons_[i].Name() << "(" << airs_[i] << ")";
	}
	cout << "\n";
	cout << "�@cond�l�F" << cond_ << "�@�c�e��(��)�F" << ammo_ << "�@�c�R��(��)" << fuel_ << "\n";
}

// �ύX�ȕ��������Z�b�g����
Kammusu Kammusu::Reset() {
	hp_ = max_hp_;
	airs_ = max_airs_;
	weapons_.resize(slots_, Weapon());
	cond_ = 49;
	ammo_ = 100;
	fuel_ = 100;
	return *this;
}

/*Kammusu Kammusu::Reset(const WeaponDB &weapon_db) {
	this->Reset();
	for (auto i = 0; i < slots_; ++i) {
		weapons_[i] = weapon_db.Get(first_weapons_[i]);
	}
	return *this;
}*/

// ������𑬗͂ɕϊ�����
Speed ToSpeed(const string &str) {
	if (str == "10") return kSpeedHigh;
	if (str == "5") return kSpeedLow;
	return kSpeedNone;
}
