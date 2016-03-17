#ifndef KCS_KAI_MAPDATA_H_
#define KCS_KAI_MAPDATA_H_
#include "char_convert.hpp"

class MapData {
	vector<vector<Fleet>> fleet_;				//�e�}�X���Ƃ̊͑��f�[�^
	vector<SimulateMode> simulate_mode_;		//�e�}�X���Ƃ̐퓬�`��
	vector<wstring> point_name_;				//�e�}�X�̖��O
public:
	// �R���X�g���N�^
	MapData(const string &file_name, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, char_cvt::char_enc fileenc = char_cvt::char_enc::utf8);
	// getter
	size_t GetSize() const noexcept;
	Fleet GetFleet(const size_t) const;
	Fleet GetFleet(const size_t, const size_t) const noexcept;
	SimulateMode GetSimulateMode(const size_t) const noexcept;
	wstring GetPointName(const size_t) const noexcept;
	// setter
	void SetRandGenerator(const unsigned int seed);
	// ���̑�
	void Put();	//���e��\������
};

#endif
