#include "base.hpp"
#include "fleet.hpp"

void skip_utf8_bom(std::ifstream& fs, char_cvt::char_enc& enc) {
	int dst[3];
	for (auto& i : dst) i = fs.get();
	constexpr int utf8[] = { 0xEF, 0xBB, 0xBF };
	if (!std::equal(std::begin(dst), std::end(dst), utf8)) fs.seekg(0);
	else enc = char_cvt::char_enc::utf8;
}
void Fleet::LoadJson(std::istream & file, const WeaponDB & weapon_db, const KammusuDB & kammusu_db, char_cvt::char_enc fileenc)
{
	using picojson::object;
	using picojson::value;
	if (fileenc == char_cvt::char_enc::unknown) throw std::runtime_error("unknown char enc type.");//文字コード自動判別なんてやらない
	value v;
	file >> v;
	// 読み込んだJSONデータを解析する
	auto& o = v.get<object>();
	//司令部レベル
	if (o.find("lv") != o.end()) {
		level_ = limit(stoi(o["lv"].to_str()), 1, 120);
	}
	else {
		level_ = 120;
	}
	//艦隊の形式
	if (o.find("type") != o.end()) {
		fleet_type_ = limit(FleetType(stoi(o["type"].to_str())), kFleetTypeNormal, kFleetTypeCombined);
		if (fleet_type_ == kFleetTypeCombined && formation_ == kFormationEchelon) {
			// 連合艦隊に梯形陣は存在しないので、とりあえず単横陣(第一警戒航行序列)に変更しておく
			formation_ = kFormationAbreast;
		}
	}
	else {
		fleet_type_ = kFleetTypeNormal;
	}
	unit_.resize(fleet_type_);
	//艦娘・深海棲艦
	int fi = 0;	//読み込む際のインデックス
	for (auto &temp_f : o) {
		// 艦隊の形式によって、読まなければならない艦隊数は異なる
		if (fleet_type_ == kFleetTypeNormal) {
			if (fi >= 1) break;
		}
		else {
			if (fi >= 2) break;
		}
		// 艦隊を1つづつ読み込んでいく
		auto& fleet = temp_f.second.get<object>();
		for (auto &temp_u : fleet) {
			// 艦船ID・レベル・運・cond値から艦娘を設定する
			auto& unit = temp_u.second.get<object>();
			auto id = stoi(unit["id"].to_str());
			auto level = limit(stoi(unit["lv"].to_str()), 1, 155);	//上限はいつか変わるかも？
			Kammusu temp_k = kammusu_db.Get(id, level).Reset();
			auto luck = limit(stoi(unit["luck"].to_str()), 0, 100);
			temp_k.SetLuck(luck);
			if (unit.find("cond") != unit.end()) {
				temp_k.SetCond(limit(stoi(unit["cond"].to_str()), 0, 100));
			}
			// 装備ID・改修/熟練度・内部熟練度から装備を設定する
			int wi = 0;
			for (auto &temp_p : unit["items"].get<object>()) {
				auto& parts = temp_p.second.get<object>();
				id = stoi(parts["id"].to_str());
				Weapon temp_w = weapon_db.Get(id);
				// 改修・外部熟練度・内部熟練度の処理
				if (temp_w.IsAir()) {
					level = limit(stoi(parts["rf"].to_str()), 0, 7);
					int level_detail = 0;
					if (parts.find("rf_detail") != parts.end()) {
						level_detail = limit(stoi(parts["rf_detail"].to_str()), 0, 120);
						level = ConvertDtoS(level_detail);
					}
					else {
						level_detail = ConvertStoD(level);
					}
					temp_w.SetLevelDetail(level_detail);
				}
				else {
					level = limit(stoi(parts["rf"].to_str()), 0, 10);
				}
				temp_w.SetLevel(level);
				// 艦娘に装備させる
				temp_k.SetWeapon(wi, temp_w);
				++wi;
				if (wi >= temp_k.GetSlots()) break;
			}
			// リストに加える
			unit_[fi].push_back(temp_k);
		}
		++fi;
	}
}
// コンストラクタ
Fleet::Fleet(const string &file_name, const Formation &formation, const WeaponDB &weapon_db, const KammusuDB &kammusu_db, char_cvt::char_enc fileenc) 
	: formation_(formation)// 陣形はそのまま反映させる
{
	if (fileenc == char_cvt::char_enc::unknown) throw std::runtime_error("unknown char enc type.");//文字コード自動判別なんてやらない
	// ファイルを読み込む
	ifstream fin(file_name);
	if (!fin.is_open()) throw "艦隊データが正常に読み込めませんでした.";
	if(char_cvt::char_enc::shift_jis != fileenc) skip_utf8_bom(fin, fileenc);
	this->LoadJson(fin, weapon_db, kammusu_db, fileenc);
}

Fleet::Fleet(std::istream & file, const Formation & formation, const WeaponDB & weapon_db, const KammusuDB & kammusu_db, char_cvt::char_enc fileenc)
	: formation_(formation)// 陣形はそのまま反映させる
{
	if (fileenc == char_cvt::char_enc::unknown) throw std::runtime_error("unknown char enc type.");//文字コード自動判別なんてやらない
	this->LoadJson(file, weapon_db, kammusu_db, fileenc);
}

// 中身を表示する
void Fleet::Put() const {
	cout << *this;
}

std::ostream & operator<<(std::ostream & os, const Fleet & conf)
{
	os << "陣形：" << char_cvt::utf_16_to_shift_jis(kFormationStr[conf.formation_]) << "　司令部レベル：" << conf.level_ << "　形式：" << char_cvt::utf_16_to_shift_jis(kFleetTypeStr[conf.fleet_type_ - 1]) << endl;
	for (auto fi = 0; fi < conf.fleet_type_; ++fi) {
		os << "　第" << (fi + 1) << "艦隊：" << endl;
		for (auto &it_k : conf.unit_[fi]) {
			os << "　　" << char_cvt::utf_16_to_shift_jis(it_k.GetName()) << endl;
		}
	}
	os << endl;
	return os;
}

std::wostream & operator<<(std::wostream & os, const Fleet & conf)
{
	os << L"陣形：" << kFormationStr[conf.formation_] << L"　司令部レベル：" << conf.level_ << L"　形式：" << kFleetTypeStr[conf.fleet_type_ - 1] << endl;
	for (auto fi = 0; fi < conf.fleet_type_; ++fi) {
		os << L"　第" << (fi + 1) << L"艦隊：" << endl;
		for (auto &it_k : conf.unit_[fi]) {
			os << L"　　" << it_k.GetName() << endl;
		}
	}
	os << endl;
	return os;
}

double Fleet::SearchValue() const {
	// 2-5式(秋)を採用。将来的には複数形式を切り替えられるようにする
	double search_sum = 0.0;
	//司令部レベル(5の倍数で切り上げた)による補正
	int round_up5_level = ((level_ - 1) / 5 + 1) * 5;
	search_sum += round_up5_level * (-0.6142467);
	//艦娘・装備による補正
	for (auto fi = 0; fi < fleet_type_; ++fi) {
		for (auto &it_k : unit_[fi]) {
			search_sum += sqrt(it_k.GetSearch()) * 1.6841056;
			for (auto &it_w : it_k.GetWeapon()) {
				switch (it_w.GetWeaponClass()) {
				case kWeaponClassPB:	//艦爆
				case kWeaponClassPBF:	//艦爆
					search_sum += it_w.GetSearch() * 1.0376255;
					break;
				case kWeaponClassWB:	//水爆
					search_sum += it_w.GetSearch() * 1.7787282;
					break;
				case kWeaponClassPA:	//艦攻
					search_sum += it_w.GetSearch() * 1.3677954;
					break;
				case kWeaponClassPS:	//艦偵
					search_sum += it_w.GetSearch() * 1.6592780;
					break;
				case kWeaponClassWS:	//水偵
					search_sum += it_w.GetSearch() * 2.0000000;
					break;
				case kWeaponClassSmallR:	//小型電探
					search_sum += it_w.GetSearch() * 1.0045358;
					break;
				case kWeaponClassLargeR:	//大型電探
					search_sum += it_w.GetSearch() * 0.9906638;
					break;
				case kWeaponClassSL:	//探照灯
					search_sum += it_w.GetSearch() * 0.9067950;
					break;
				default:
					break;
				}
			}
		}
	}
	return floor(search_sum * 10.0 + 0.5) / 10.0;	//小数第2位を四捨五入
}
