#pragma once

// 種別
enum WeaponClass {
	kWeaponClassGun, kWeaponClassAP, kWeaponClassSubGun, kWeaponClassTorpedo, kWeaponClassSpecialSS,
	kWeaponClassPF, kWeaponClassPB, kWeaponClassPBF, kWeaponClassWB, kWeaponClassPA, kWeaponClassPS,
	kWeaponClassPSS, kWeaponClassDaiteiChan, kWeaponClassWS, kWeaponClassWSN, kWeaponClassASPP, kWeaponClassAJ,
	kWeaponClassSmallR, kWeaponClassLargeR, kWeaponClassAAG, kWeaponClassAAA, kWeaponClassAAD, kWeaponClassDP,
	kWeaponClassSonar, kWeaponClassDC, kWeaponClassSL, kWeaponClassLB, kWeaponClassHQ, kWeaponClassSSP,
	kWeaponClassCR, kWeaponClassOS, kWeaponClassOther
};
const vector<string> kWeaponClassStr = {
	"主砲", "対艦強化弾", "副砲", "魚雷", "特殊潜航艇", "艦上戦闘機", "艦上爆撃機","艦上爆撃機(爆戦)", "水上爆撃機",
	"艦上攻撃機", "艦上偵察機", "艦上偵察機(彩雲)", "大型飛行艇", "水上偵察機", "水上偵察機(夜偵)", "対潜哨戒機",
	"オートジャイロ", "小型電探","大型電探", "対空機銃", "対空強化弾", "高射装置", "爆雷", "ソナー", "応急修理要員",
	"探照灯", "照明弾", "艦隊司令部施設", "水上艦要員", "戦闘糧食", "洋上補給", "その他"
};

//装備クラス
class Weapon {
	int id_;					//装備ID
	string name_;				//装備名
	WeaponClass weapon_class_;	//種別
	int defense_;				//装甲
	int attack_;				//火力
	int torpedo_;				//雷撃
	int bomb_;					//爆装
	int anti_air_;				//対空
	int anti_sub_;				//対潜
	int hit_;					//命中
	int evade_;					//回避
	int search_;				//索敵
	Range range_;				//射程
	int level_;					//装備改修度(0-10)、外部熟練度(0-7)
	int level_detail_;			//内部熟練度(0-120)
public:
	// コンストラクタ
	Weapon();
	Weapon(
		const int, const string, const WeaponClass, const int, const int, const int, const int,
		const int, const int, const int, const int, const int, const Range, const int, const int);
	// getter
	string Name() const { return name_; }
	// setter
	void SetLevel(const int level) { level_ = level; }
	void SetLevelDetail(const int level_detail) { level_detail_ = level_detail; }
	// その他
	void Put() const;	// 中身を表示する
	bool IsAir();		// (熟練度が存在する)艦載機ならtrue
};

// 文字列を種別に変換する
WeaponClass ToWC(const string);

// 外部熟練度(Simple)を内部熟練度(Detail)に変換する
int ConvertStoD(const int);

// 内部熟練度を外部熟練度に変換する
int ConvertDtoS(const int);
