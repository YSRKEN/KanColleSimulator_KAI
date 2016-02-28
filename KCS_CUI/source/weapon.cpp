#include "base.hpp"
#include "weapon.hpp"

// �R���X�g���N�^
Weapon::Weapon() : Weapon(-1, "�Ȃ�", kWeaponClassOther, 0, 0, 0, 0, 0, 0, 0, 0, 0, kRangeNone, 0) {}

Weapon::Weapon(
	const int id, const string name, const WeaponClass weapon_class, const int defense,
	const int attack, const int torpedo, const int bomb, const int anti_air, const int anti_sub,
	const int hit, const int evade, const int search, const Range range, const int level):
	id_(id), name_(name), weapon_class_(weapon_class), defense_(defense), attack_(attack),
	torpedo_(torpedo), bomb_(bomb), anti_air_(anti_air), anti_sub_(anti_sub), hit_(hit),
	evade_(evade), search_(search), range_(range), level_(level) {}

// ���g��\������
void Weapon::Put() {
	cout << "����ID�F" << id_ << "\n";
	cout << "�@�������F" << name_ << "�@��ʁF" << weapon_class_ << "\n";
	cout << "�@���b�F" << defense_ << "�@�ΉF" << attack_ << "�@�����F" << torpedo_ << "�@�����F" << bomb_ << "\n";
	cout << "�@�΋�F" << anti_air_ << "�@�ΐ��F" << anti_sub_ << "�@�����F" << hit_ << "�@����F" << evade_ << "\n";
	cout << "�@���G�F" << search_ << "�@�˒��F" << kRangeStr[range_] << "�@���C/�n���F" << level_ << "\n";
}

// ���������ʂɕϊ�����
WeaponClass ToWC(const string str) {
	for (auto i = 0u; i < kWeaponClassStr.size(); ++i) {
		if (str == kWeaponClassStr[i]) return static_cast<WeaponClass>(i);
	}
	return kWeaponClassOther;
}
