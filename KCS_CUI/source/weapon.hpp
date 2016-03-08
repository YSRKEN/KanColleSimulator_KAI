#ifndef KCS_KAI_INC_WEAPON_HPP_
#define KCS_KAI_INC_WEAPON_HPP_

#include <cstdint>
#include <iostream>
#include <type_traits>
#include <sprout/string.hpp>
#include <sprout/algorithm.hpp>
#include <sprout/math.hpp>//Link error
//#include <sprout/math/sqrt.hpp>
#include <array>
// 種別
enum class WeaponClass : std::uint64_t {
	Gun        = 0x0000000000000001,	//主砲
	AP         = 0x0000000000000002,	//対艦強化弾
	SubGun     = 0x0000000000000004,	//副砲
	Torpedo    = 0x0000000000000008,	//魚雷
	SpecialSS  = 0x0000000000000010,	//特殊潜航艇
	PF         = 0x0000000000000020,	//艦上戦闘機
	PB         = 0x0000000000000040,	//艦上爆撃機
	PBF        = 0x0000000000000080,	//艦上爆撃機(爆戦)
	WB         = 0x0000000000000100,	//水上爆撃機
	PA         = 0x0000000000000200,	//艦上攻撃機
	PS         = 0x0000000000000400,	//艦上偵察機
	PSS        = 0x0000000000000800,	//艦上偵察機(彩雲)
	DaiteiChan = 0x0000000000001000,	//大型飛行艇
	WS         = 0x0000000000002000,	//水上偵察機
	WSN        = 0x0000000000004000,	//水上偵察機(夜偵)
	ASPP       = 0x0000000000008000,	//対潜哨戒機
	AJ         = 0x0000000000010000,	//オートジャイロ
	SmallR     = 0x0000000000020000,	//小型電探
	LargeR     = 0x0000000000040000,	//大型電探
	AAG        = 0x0000000000080000,	//対空機銃
	AAA        = 0x0000000000100000,	//対空強化弾
	AAD        = 0x0000000000200000,	//高射装置
	DP         = 0x0000000000400000,	//爆雷
	Sonar      = 0x0000000000800000,	//ソナー
	DC         = 0x0000000001000000,	//応急修理要員
	SL         = 0x0000000002000000,	//探照灯
	LB         = 0x0000000004000000,	//照明弾
	HQ         = 0x0000000008000000,	//艦隊司令部施設
	SSP        = 0x0000000010000000,	//水上艦要員
	CR         = 0x0000000020000000,	//戦闘糧食
	OS         = 0x0000000040000000,	//洋上補給
	Other      = 0x8000000000000000,	//その他
	AirFight = PF | PBF | PB | WB | PA,						//航空戦に参加する艦載機
	AirTrailer = PA | PS | PSS | DaiteiChan | WS | WSN,		//触接に参加する艦載機
	AirBomb = PBF | PB | WB | PA,							//開幕爆撃に参加する艦載機
	Air = AirFight | AirTrailer,							//(熟練度が存在する)艦載機
};
constexpr inline auto operator|(const WeaponClass& l, const WeaponClass& r) { return static_cast<WeaponClass>(static_cast<std::underlying_type_t<WeaponClass>>(l) | static_cast<std::underlying_type_t<WeaponClass>>(r)); }
template<class E, class T>
inline auto& operator<<(std::basic_ostream<E, T>& os, const WeaponClass& wc) { return os << static_cast<std::underlying_type_t<WeaponClass>>(wc); }
namespace detail {
	constexpr inline double AntiAirScore_impl(
		double anti_air_score_base, WeaponClass weapon_class, int level,
		const std::array<double, 8>& kBonusPF, const std::array<double, 8>& kBonusWB
	) noexcept {
		return (weapon_class == WeaponClass::PF)
			? anti_air_score_base + kBonusPF[level]
			: (weapon_class == WeaponClass::WB)
				? anti_air_score_base + kBonusWB[level]
				: anti_air_score_base;
	}
}
//装備クラス
class Weapon {
	int id_;					//装備ID
	weapon_str_t name_;
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
	int air_;					//現搭載数
public:
	// コンストラクタ
	constexpr Weapon() noexcept : Weapon(-1, {}, WeaponClass::Other, 0, 0, 0, 0, 0, 0, 0, 0, 0, kRangeNone, 0, 0, 0) {}
	constexpr Weapon(
		const int id, const weapon_str_t& name, const WeaponClass weapon_class, const int defense,
		const int attack, const int torpedo, const int bomb, const int anti_air, const int anti_sub,
		const int hit, const int evade, const int search, const Range range, const int level, const int level_detail, const int air
	) noexcept 
		: id_(id), name_(name), weapon_class_(weapon_class), defense_(defense), attack_(attack),
		torpedo_(torpedo), bomb_(bomb), anti_air_(anti_air), anti_sub_(anti_sub), hit_(hit),
		evade_(evade), search_(search), range_(range), level_(level), level_detail_(level_detail), air_(air)
	{}
	// getter
	constexpr int GetID() const noexcept { return id_; }
	constexpr weapon_str_t GetName() const noexcept { return name_; }
	constexpr WeaponClass GetWeaponClass() const noexcept { return weapon_class_; }
	constexpr int GetDefense() const noexcept { return defense_; }
	constexpr int GetAttack() const noexcept { return attack_; }
	constexpr int GetTorpedo() const noexcept { return torpedo_; }
	constexpr int GetBomb() const noexcept { return bomb_; }
	constexpr int GetAntiAir() const noexcept { return anti_air_; }
	constexpr int GetAntiSub() const noexcept { return anti_sub_; }
	constexpr int GetHit() const noexcept { return hit_; }
	constexpr int GetEvade() const noexcept { return evade_; }
	constexpr int GetSearch() const noexcept { return search_; }
	constexpr Range GetRange() const noexcept { return range_; }
	constexpr int GetLevel() const noexcept { return level_; }
	constexpr int GetAir() const noexcept { return air_; }
	// setter
	void SetLevel(const int level);
	void SetLevelDetail(const int level_detail);
	void SetAir(const int air);
	// その他
	void Put() const;					//中身を表示する
	constexpr int AntiAirScore(const int airs) const noexcept {	//制空値を計算する
		return static_cast<int>(detail::AntiAirScore_impl(
			this->anti_air_ * sprout::sqrt(airs) + sprout::sqrt(1.0 * level_detail_ / 10),
			this->weapon_class_, this->level_, 
			{ { 0,0,2,5,9,14,14,22 } }, { { 0,0,1,1,1,3,3,6 } }
		));
	}
	// 指定の種別か判定する
	bool Is(const WeaponClass& wc) const noexcept { return (static_cast<std::underlying_type_t<WeaponClass>>(weapon_class_) & static_cast<std::underlying_type_t<WeaponClass>>(wc)) != 0; }
	bool IsHAG() const noexcept;					//高角砲ならtrue
	bool Include(const weapon_str_t&) const noexcept;	//名前に特定の文字が含まれていればtrue
	friend std::ostream& operator<<(std::ostream& os, const Weapon& conf);
	friend std::wostream& operator<<(std::wostream& os, const Weapon& conf);
};
std::ostream& operator<<(std::ostream& os, const Weapon& conf);
std::wostream& operator<<(std::wostream& os, const Weapon& conf);

// 文字列を種別に変換する
WeaponClass ToWC(const string);

// 外部熟練度(Simple)を内部熟練度(Detail)に変換する
int ConvertStoD(const int&);

// 内部熟練度を外部熟練度に変換する
int ConvertDtoS(const int&);

namespace detail {
	struct WeaponClass_cvt_t {
		weapon_str_t str;
		WeaponClass wc;
	};
	struct ToWC_helper {};
	namespace ToWC_cvt {
		static constexpr std::array<WeaponClass_cvt_t, 32> cvt = { {//sorted by first string
			{ L"その他", WeaponClass::Other },
			{ L"オートジャイロ", WeaponClass::AJ },
			{ L"ソナー", WeaponClass::Sonar },
			{ L"主砲", WeaponClass::Gun },
			{ L"副砲", WeaponClass::SubGun },
			{ L"大型電探", WeaponClass::LargeR },
			{ L"大型飛行艇", WeaponClass::DaiteiChan },
			{ L"対潜哨戒機", WeaponClass::ASPP },
			{ L"対空強化弾", WeaponClass::AAA },
			{ L"対空機銃", WeaponClass::AAG },
			{ L"対艦強化弾", WeaponClass::AP },
			{ L"小型電探", WeaponClass::SmallR },
			{ L"応急修理要員", WeaponClass::DC },
			{ L"戦闘糧食", WeaponClass::CR },
			{ L"探照灯", WeaponClass::SL },
			{ L"水上偵察機", WeaponClass::WS },
			{ L"水上偵察機(夜偵)", WeaponClass::WSN },
			{ L"水上爆撃機", WeaponClass::WB },
			{ L"水上艦要員", WeaponClass::SSP },
			{ L"洋上補給", WeaponClass::OS },
			{ L"照明弾", WeaponClass::LB },
			{ L"爆雷", WeaponClass::DP },
			{ L"特殊潜航艇", WeaponClass::SpecialSS },
			{ L"艦上偵察機", WeaponClass::PS },
			{ L"艦上偵察機(彩雲)", WeaponClass::PSS },
			{ L"艦上戦闘機", WeaponClass::PF },
			{ L"艦上攻撃機", WeaponClass::PA },
			{ L"艦上爆撃機", WeaponClass::PB },
			{ L"艦上爆撃機(爆戦)", WeaponClass::PBF },
			{ L"艦隊司令部施設", WeaponClass::HQ },
			{ L"高射装置", WeaponClass::AAD },
			{ L"魚雷", WeaponClass::Torpedo },
		} };
	}
	constexpr bool operator<(const WeaponClass_cvt_t& l, const weapon_str_t& r) {
		return l.str < r;
	}
	template<typename iterator, typename Compare>
	constexpr WeaponClass convert_or_default(iterator it, iterator end, weapon_str_t val, WeaponClass default_, Compare comp) {
		return (it != end && comp(val, it->str)) ? it->wc : default_;
	}
	template<typename Compare>
	constexpr WeaponClass ToWC_operator_impl(weapon_str_t str, std::array<WeaponClass_cvt_t, 32> cvt, Compare comp) {
		return convert_or_default(
			sprout::lower_bound(cvt.begin(), cvt.end(), str, comp),
			cvt.end(), str, WeaponClass::Other, comp
		);
	}
	template<std::size_t N, std::enable_if_t<!(weapon_str_capacity< N), std::nullptr_t> = nullptr>
	constexpr WeaponClass operator| (const wchar_t(&str)[N], ToWC_helper) {
		return ToWC_operator_impl(weapon_str_t(str), ToWC_cvt::cvt, sprout::less<>());
	}
}
constexpr detail::ToWC_helper ToWC() { return{}; }
#endif //KCS_KAI_INC_WEAPON_HPP_
