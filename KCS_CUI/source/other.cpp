#include "base.hpp"
#include "other.hpp"
#include "fleet.hpp"
#include "result.hpp"
#include "char_convert.hpp"
#include <cctype>
#include <algorithm>
enum class CsvParseLevel : std::size_t { kLevel1 = 0, kLevel99 = 1 };
namespace detail {
	struct Split_helper_index { char delim; std::size_t index; };
	struct Split_helper {
		char delim;
		Split_helper_index operator[](std::size_t n) const noexcept { return{ delim, n }; }
		Split_helper_index operator[](CsvParseLevel n) const noexcept { return this->operator[](static_cast<std::size_t>(n)); }
	};
	string operator| (const std::string& str, Split_helper_index info) {
		std::size_t pre = 0, pos = 0;
		for (size_t i = 0; i < info.index + 1; ++i) {
			pre = pos;
			pos = str.find_first_of(info.delim, pos) + 1;
		}
		return str.substr(pre, pos - pre - 1);
	}
	vector<string> operator| (const std::string& str, Split_helper info) {
		vector<string> re;
		size_t current = 0;
		for (size_t found; (found = str.find_first_of(info.delim, current)) != string::npos; current = found + 1) {
			re.emplace_back(str, current, found - current);
		}
		re.emplace_back(str, current, str.size() - current);
		return re;
	}
	vector<string> operator| (std::string&& str, Split_helper info) {
		vector<string> re;
		size_t current = 0;
		for (size_t found; (found = str.find_first_of(info.delim, current)) != string::npos; current = found + 1) {
			re.emplace_back(str, current, found - current);
		}
		str.erase(0, current);
		re.emplace_back(std::move(str));
		return re;
	}
}
detail::Split_helper Split(char delim) noexcept { return{ delim }; }
namespace detail {
	// 文字列配列を数字配列に変換する
	inline vector<int> operator|(const vector<string> &arr_str, to_i_helper<int>) {
		vector<int> arr_int;
		for (auto &it : arr_str) {
			arr_int.push_back(it | to_i());
		}
		return arr_int;
	}
	struct ToHash_helper {};
	// 配列をハッシュに変換する
	template<typename T>
	inline unordered_map<T, size_t> operator|(const vector<T> &vec, ToHash_helper) {
		unordered_map<T, size_t> hash;
		for (size_t i = 0; i < vec.size(); ++i) {
			hash[vec[i]] = i;
		}
		return hash;
	}
}
detail::ToHash_helper ToHash() noexcept { return{}; }
// 装備DBのコンストラクタ
WeaponDB::WeaponDB() 
	:hash_({
		{ 1, Weapon{ 1, L"12cm単装砲", L"主砲" | ToWC(), 0, 1, 0, 0, 1, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 2, Weapon{ 2, L"12.7cm連装砲", L"主砲" | ToWC(), 0, 2, 0, 0, 2, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 3, Weapon{ 3, L"10cm連装高角砲", L"主砲" | ToWC(), 0, 2, 0, 0, 7, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 4, Weapon{ 4, L"14cm単装砲", L"主砲" | ToWC(), 0, 2, 0, 0, 0, 0, 1, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 5, Weapon{ 5, L"15.5cm三連装砲", L"主砲" | ToWC(), 0, 7, 0, 0, 4, 0, 1, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 6, Weapon{ 6, L"20.3cm連装砲", L"主砲" | ToWC(), 0, 8, 0, 0, 3, 0, 0, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 7, Weapon{ 7, L"35.6cm連装砲", L"主砲" | ToWC(), 0, 15, 0, 0, 4, 0, 0, 0, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 8, Weapon{ 8, L"41cm連装砲", L"主砲" | ToWC(), 0, 20, 0, 0, 4, 0, 0, 0, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 9, Weapon{ 9, L"46cm三連装砲", L"主砲" | ToWC(), 0, 26, 0, 0, 5, 0, 0, 0, 0, static_cast<Range>(4), 0, 0, 0 } },
		{ 10, Weapon{ 10, L"12.7cm連装高角砲", L"主砲" | ToWC(), 0, 2, 0, 0, 4, 0, 1, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 11, Weapon{ 11, L"15.2cm単装砲", L"主砲" | ToWC(), 0, 2, 0, 0, 0, 0, 1, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 12, Weapon{ 12, L"15.5cm三連装副砲", L"主砲" | ToWC(), 0, 7, 0, 0, 3, 0, 2, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 13, Weapon{ 13, L"61cm三連装魚雷", L"魚雷" | ToWC(), 0, 0, 5, 0, 0, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 14, Weapon{ 14, L"61cm四連装魚雷", L"魚雷" | ToWC(), 0, 0, 7, 0, 0, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 15, Weapon{ 15, L"61cm四連装(酸素)魚雷", L"魚雷" | ToWC(), 0, 0, 10, 0, 0, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 16, Weapon{ 16, L"九七式艦攻", L"艦上攻撃機" | ToWC(), 0, 0, 5, 0, 0, 4, 0, 0, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 17, Weapon{ 17, L"天山", L"艦上攻撃機" | ToWC(), 0, 0, 7, 0, 0, 3, 0, 0, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 18, Weapon{ 18, L"流星", L"艦上攻撃機" | ToWC(), 0, 0, 10, 0, 0, 4, 0, 0, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 19, Weapon{ 19, L"九六式艦戦", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 2, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 20, Weapon{ 20, L"零式艦戦21型", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 5, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 21, Weapon{ 21, L"零式艦戦52型", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 6, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 22, Weapon{ 22, L"烈風", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 10, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 23, Weapon{ 23, L"九九式艦爆", L"艦上爆撃機" | ToWC(), 0, 0, 0, 5, 0, 3, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 24, Weapon{ 24, L"彗星", L"艦上爆撃機" | ToWC(), 0, 0, 0, 8, 0, 3, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 25, Weapon{ 25, L"零式水上偵察機", L"水上偵察機" | ToWC(), 0, 0, 0, 1, 1, 2, 1, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 26, Weapon{ 26, L"瑞雲", L"水上爆撃機" | ToWC(), 0, 0, 0, 4, 2, 4, 1, 0, 6, static_cast<Range>(0), 0, 0, 0 } },
		{ 27, Weapon{ 27, L"13号対空電探", L"小型電探" | ToWC(), 0, 0, 0, 0, 2, 0, 1, 0, 3, static_cast<Range>(0), 0, 0, 0 } },
		{ 28, Weapon{ 28, L"22号対水上電探", L"小型電探" | ToWC(), 0, 0, 0, 0, 0, 0, 3, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 29, Weapon{ 29, L"33号対水上電探", L"小型電探" | ToWC(), 0, 0, 0, 0, 0, 0, 5, 0, 7, static_cast<Range>(0), 0, 0, 0 } },
		{ 30, Weapon{ 30, L"21号対空電探", L"大型電探" | ToWC(), 0, 0, 0, 0, 4, 0, 2, 0, 4, static_cast<Range>(0), 0, 0, 0 } },
		{ 31, Weapon{ 31, L"32号対水上電探", L"大型電探" | ToWC(), 0, 0, 0, 0, 0, 0, 8, 0, 10, static_cast<Range>(0), 0, 0, 0 } },
		{ 32, Weapon{ 32, L"14号対空電探", L"大型電探" | ToWC(), 0, 0, 0, 0, 6, 0, 4, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 33, Weapon{ 33, L"改良型艦本式タービン", L"機関部強化" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 6, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 34, Weapon{ 34, L"強化型艦本式缶", L"機関部強化" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 10, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 35, Weapon{ 35, L"三式弾", L"対空強化弾" | ToWC(), 0, 0, 0, 0, 5, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 36, Weapon{ 36, L"九一式徹甲弾", L"対艦強化弾" | ToWC(), 0, 8, 0, 0, 0, 0, 1, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 37, Weapon{ 37, L"7.7mm機銃", L"対空機銃" | ToWC(), 0, 0, 0, 0, 2, 0, 0, 1, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 38, Weapon{ 38, L"12.7mm単装機銃", L"対空機銃" | ToWC(), 0, 0, 0, 0, 3, 0, 0, 1, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 39, Weapon{ 39, L"25mm連装機銃", L"対空機銃" | ToWC(), 0, 0, 0, 0, 5, 0, 0, 1, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 40, Weapon{ 40, L"25mm三連装機銃", L"対空機銃" | ToWC(), 0, 0, 0, 0, 6, 0, 0, 1, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 41, Weapon{ 41, L"甲標的", L"特殊潜航艇" | ToWC(), 0, 0, 12, 0, 0, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 42, Weapon{ 42, L"応急修理要員", L"応急修理要員" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 43, Weapon{ 43, L"応急修理女神", L"応急修理要員" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 44, Weapon{ 44, L"九四式爆雷投射機", L"爆雷" | ToWC(), 0, 0, 0, 0, 0, 5, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 45, Weapon{ 45, L"三式爆雷投射機", L"爆雷" | ToWC(), 0, 0, 0, 0, 0, 8, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 46, Weapon{ 46, L"九三式水中聴音機", L"ソナー" | ToWC(), 0, 0, 0, 0, 0, 6, 1, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 47, Weapon{ 47, L"三式水中探信儀", L"ソナー" | ToWC(), 0, 0, 0, 0, 0, 10, 2, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 48, Weapon{ 48, L"12.7cm単装高角砲", L"主砲" | ToWC(), 0, 1, 0, 0, 3, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 49, Weapon{ 49, L"25mm単装機銃", L"対空機銃" | ToWC(), 0, 0, 0, 0, 4, 0, 0, 1, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 50, Weapon{ 50, L"20.3cm(3号)連装砲", L"主砲" | ToWC(), 0, 10, 0, 0, 4, 0, 0, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 51, Weapon{ 51, L"12cm30連装噴進砲", L"対空機銃" | ToWC(), 0, 0, 0, 0, 8, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 52, Weapon{ 52, L"流星改", L"艦上攻撃機" | ToWC(), 0, 0, 13, 0, 0, 3, 0, 0, 2, static_cast<Range>(0), 0, 0, 0 } },
		{ 53, Weapon{ 53, L"烈風改", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 12, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 54, Weapon{ 54, L"彩雲", L"艦上偵察機(彩雲)" | ToWC(), 0, 0, 0, 0, 0, 0, 2, 0, 9, static_cast<Range>(0), 0, 0, 0 } },
		{ 55, Weapon{ 55, L"紫電改二", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 9, 0, 0, 3, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 56, Weapon{ 56, L"震電改", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 15, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 57, Weapon{ 57, L"彗星一二型甲", L"艦上爆撃機" | ToWC(), 0, 0, 0, 10, 0, 3, 0, 0, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 58, Weapon{ 58, L"61cm五連装(酸素)魚雷", L"魚雷" | ToWC(), 0, 0, 12, 0, 0, 0, 1, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 59, Weapon{ 59, L"零式水上観測機", L"水上偵察機" | ToWC(), 0, 0, 0, 1, 2, 4, 2, 0, 6, static_cast<Range>(0), 0, 0, 0 } },
		{ 60, Weapon{ 60, L"零式艦戦62型(爆戦)", L"艦上爆撃機(爆戦)" | ToWC(), 0, 0, 0, 4, 4, 3, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 61, Weapon{ 61, L"二式艦上偵察機", L"艦上偵察機" | ToWC(), 0, 0, 0, 0, 1, 0, 3, 0, 7, static_cast<Range>(0), 0, 0, 0 } },
		{ 62, Weapon{ 62, L"試製晴嵐", L"水上爆撃機" | ToWC(), 0, 0, 0, 11, 0, 6, 1, 0, 6, static_cast<Range>(0), 0, 0, 0 } },
		{ 63, Weapon{ 63, L"12.7cm連装砲B型改二", L"主砲" | ToWC(), 0, 3, 0, 0, 2, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 64, Weapon{ 64, L"Ju87C改", L"艦上爆撃機" | ToWC(), 0, 0, 0, 9, 0, 5, 1, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 65, Weapon{ 65, L"15.2cm連装砲", L"主砲" | ToWC(), 0, 4, 0, 0, 3, 0, 3, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 66, Weapon{ 66, L"8cm高角砲", L"副砲" | ToWC(), 0, 1, 0, 0, 6, 0, 2, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 67, Weapon{ 67, L"53cm艦首(酸素)魚雷", L"魚雷" | ToWC(), 0, 0, 15, 0, 0, 0, 2, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 68, Weapon{ 68, L"大発動艇", L"上陸用舟艇" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 69, Weapon{ 69, L"カ号観測機", L"オートジャイロ" | ToWC(), 0, 0, 0, 0, 0, 9, 1, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 70, Weapon{ 70, L"三式指揮連絡機(対潜)", L"対潜哨戒機" | ToWC(), 0, 0, 0, 0, 0, 7, 2, 0, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 71, Weapon{ 71, L"10cm連装高角砲(砲架)", L"副砲" | ToWC(), 0, 1, 0, 0, 7, 0, 1, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 72, Weapon{ 72, L"増設バルジ(中型艦)", L"追加装甲(中型)" | ToWC(), 7, 0, 0, 0, 0, 0, 0, -2, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 73, Weapon{ 73, L"増設バルジ(大型艦)", L"追加装甲(大型)" | ToWC(), 9, 0, 0, 0, 0, 0, 0, -3, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 74, Weapon{ 74, L"探照灯", L"探照灯" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 0, 2, static_cast<Range>(0), 0, 0, 0 } },
		{ 75, Weapon{ 75, L"ドラム缶(輸送用)", L"簡易輸送部材" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 76, Weapon{ 76, L"38cm連装砲", L"主砲" | ToWC(), 0, 16, 0, 0, 1, 0, 1, 0, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 77, Weapon{ 77, L"15cm連装副砲", L"副砲" | ToWC(), 0, 4, 0, 0, 2, 0, 2, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 78, Weapon{ 78, L"12.7cm単装砲", L"主砲" | ToWC(), 0, 2, 0, 0, 0, 0, 1, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 79, Weapon{ 79, L"瑞雲(六三四空)", L"水上爆撃機" | ToWC(), 0, 0, 0, 6, 2, 5, 1, 0, 6, static_cast<Range>(0), 0, 0, 0 } },
		{ 80, Weapon{ 80, L"瑞雲12型", L"水上爆撃機" | ToWC(), 0, 0, 0, 7, 3, 5, 1, 0, 6, static_cast<Range>(0), 0, 0, 0 } },
		{ 81, Weapon{ 81, L"瑞雲12型(六三四空)", L"水上爆撃機" | ToWC(), 0, 0, 0, 9, 3, 6, 1, 0, 7, static_cast<Range>(0), 0, 0, 0 } },
		{ 82, Weapon{ 82, L"九七式艦攻(九三一空)", L"艦上攻撃機" | ToWC(), 0, 0, 6, 0, 0, 7, 0, 0, 2, static_cast<Range>(0), 0, 0, 0 } },
		{ 83, Weapon{ 83, L"天山(九三一空)", L"艦上攻撃機" | ToWC(), 0, 0, 9, 0, 0, 8, 0, 0, 2, static_cast<Range>(0), 0, 0, 0 } },
		{ 84, Weapon{ 84, L"2cm 四連装FlaK 38", L"対空機銃" | ToWC(), 0, 0, 0, 0, 7, 0, 1, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 85, Weapon{ 85, L"3.7cm FlaK M42", L"対空機銃" | ToWC(), 0, 1, 0, 0, 8, 0, 1, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 86, Weapon{ 86, L"艦艇修理施設", L"艦艇修理施設" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 87, Weapon{ 87, L"新型高温高圧缶", L"機関部強化" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 13, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 88, Weapon{ 88, L"22号対水上電探改四", L"小型電探" | ToWC(), 0, 0, 0, 0, 0, 2, 8, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 89, Weapon{ 89, L"21号対空電探改", L"大型電探" | ToWC(), 0, 0, 0, 0, 5, 0, 3, 1, 6, static_cast<Range>(0), 0, 0, 0 } },
		{ 90, Weapon{ 90, L"20.3cm(2号)連装砲", L"主砲" | ToWC(), 0, 9, 0, 0, 3, 0, 1, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 91, Weapon{ 91, L"12.7cm連装高角砲(後期型)", L"主砲" | ToWC(), 0, 2, 0, 0, 5, 1, 1, 1, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 92, Weapon{ 92, L"毘式40mm連装機銃", L"対空機銃" | ToWC(), 0, 0, 0, 0, 6, 0, 0, 1, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 93, Weapon{ 93, L"九七式艦攻(友永隊)", L"艦上攻撃機" | ToWC(), 0, 0, 11, 0, 1, 5, 3, 0, 4, static_cast<Range>(0), 0, 0, 0 } },
		{ 94, Weapon{ 94, L"天山一二型(友永隊)", L"艦上攻撃機" | ToWC(), 0, 0, 14, 0, 1, 6, 3, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 95, Weapon{ 95, L"潜水艦53cm艦首魚雷(8門)", L"魚雷" | ToWC(), 0, 0, 16, 0, 0, 0, 3, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 96, Weapon{ 96, L"零式艦戦21型(熟練)", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 8, 0, 2, 2, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 97, Weapon{ 97, L"九九式艦爆(熟練)", L"艦上爆撃機" | ToWC(), 0, 0, 0, 7, 1, 4, 2, 0, 2, static_cast<Range>(0), 0, 0, 0 } },
		{ 98, Weapon{ 98, L"九七式艦攻(熟練)", L"艦上攻撃機" | ToWC(), 0, 0, 8, 0, 0, 5, 2, 0, 2, static_cast<Range>(0), 0, 0, 0 } },
		{ 99, Weapon{ 99, L"九九式艦爆(江草隊)", L"艦上爆撃機" | ToWC(), 0, 0, 0, 10, 0, 5, 4, 0, 3, static_cast<Range>(0), 0, 0, 0 } },
		{ 100, Weapon{ 100, L"彗星(江草隊)", L"艦上爆撃機" | ToWC(), 0, 0, 0, 13, 1, 5, 4, 0, 4, static_cast<Range>(0), 0, 0, 0 } },
		{ 101, Weapon{ 101, L"照明弾", L"照明弾" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 102, Weapon{ 102, L"九八式水上偵察機(夜偵)", L"水上偵察機(夜偵)" | ToWC(), 0, 0, 0, 0, 0, 1, 1, 0, 3, static_cast<Range>(0), 0, 0, 0 } },
		{ 103, Weapon{ 103, L"試製35.6cm三連装砲", L"主砲" | ToWC(), 0, 18, 0, 0, 5, 0, 2, 0, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 104, Weapon{ 104, L"35.6cm連装砲(ダズル迷彩)", L"主砲" | ToWC(), 0, 15, 0, 0, 5, 0, 1, 1, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 105, Weapon{ 105, L"試製41cm三連装砲", L"主砲" | ToWC(), 0, 22, 0, 0, 5, 0, 2, 0, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 106, Weapon{ 106, L"13号対空電探改", L"小型電探" | ToWC(), 0, 0, 0, 0, 4, 0, 2, 1, 4, static_cast<Range>(0), 0, 0, 0 } },
		{ 107, Weapon{ 107, L"艦隊司令部施設", L"艦隊司令部施設" | ToWC(), 0, 0, 0, 0, 1, 0, 1, 1, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 108, Weapon{ 108, L"熟練艦載機整備員", L"航空要員" | ToWC(), 0, 10, 0, 0, 1, 0, 1, 0, 1, static_cast<Range>(3), 0, 0, 0 } },
		{ 109, Weapon{ 109, L"零戦52型丙(六〇一空)", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 9, 0, 1, 1, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 110, Weapon{ 110, L"烈風(六〇一空)", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 11, 0, 1, 2, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 111, Weapon{ 111, L"彗星(六〇一空)", L"艦上爆撃機" | ToWC(), 0, 0, 0, 11, 0, 4, 1, 0, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 112, Weapon{ 112, L"天山(六〇一空)", L"艦上攻撃機" | ToWC(), 0, 0, 10, 0, 0, 4, 1, 0, 2, static_cast<Range>(0), 0, 0, 0 } },
		{ 113, Weapon{ 113, L"流星(六〇一空)", L"艦上攻撃機" | ToWC(), 0, 0, 13, 0, 0, 5, 1, 0, 3, static_cast<Range>(0), 0, 0, 0 } },
		{ 114, Weapon{ 114, L"38cm連装砲改", L"主砲" | ToWC(), 0, 17, 0, 0, 2, 0, 3, 0, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 115, Weapon{ 115, L"Ar196改", L"水上偵察機" | ToWC(), 0, 0, 0, 1, 1, 5, 2, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 116, Weapon{ 116, L"一式徹甲弾", L"対艦強化弾" | ToWC(), 0, 9, 0, 0, 0, 0, 2, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 117, Weapon{ 117, L"試製46cm連装砲", L"主砲" | ToWC(), 0, 23, 0, 0, 4, 0, 1, 0, 0, static_cast<Range>(4), 0, 0, 0 } },
		{ 118, Weapon{ 118, L"紫雲", L"水上偵察機" | ToWC(), 0, 0, 0, 1, 0, 2, 1, 0, 8, static_cast<Range>(0), 0, 0, 0 } },
		{ 119, Weapon{ 119, L"14cm連装砲", L"主砲" | ToWC(), 0, 3, 0, 0, 0, 0, 2, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 120, Weapon{ 120, L"91式高射装置", L"高射装置" | ToWC(), 0, 0, 0, 0, 2, 0, 0, 1, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 121, Weapon{ 121, L"94式高射装置", L"高射装置" | ToWC(), 0, 0, 0, 0, 3, 0, 0, 1, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 122, Weapon{ 122, L"10cm連装高角砲+高射装置", L"主砲" | ToWC(), 0, 3, 0, 0, 10, 0, 1, 1, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 123, Weapon{ 123, L"SKC34 20.3cm連装砲", L"主砲" | ToWC(), 0, 10, 0, 0, 2, 0, 3, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 124, Weapon{ 124, L"FuMO25 レーダー", L"大型電探" | ToWC(), 0, 3, 0, 0, 7, 0, 10, 0, 9, static_cast<Range>(0), 0, 0, 0 } },
		{ 125, Weapon{ 125, L"61cm三連装(酸素)魚雷", L"魚雷" | ToWC(), 1, 0, 8, 0, 0, 0, 0, 1, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 126, Weapon{ 126, L"WG42 (Wurfgerat 42)", L"対地装備" | ToWC(), -1, 1, 0, 0, 0, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 127, Weapon{ 127, L"試製FaT仕様九五式酸素魚雷改", L"魚雷" | ToWC(), 0, 0, 14, 0, 0, 0, 7, 2, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 128, Weapon{ 128, L"試製51cm連装砲", L"主砲" | ToWC(), 0, 30, 0, 0, 5, 0, 1, -1, 0, static_cast<Range>(4), 0, 0, 0 } },
		{ 129, Weapon{ 129, L"熟練見張員", L"水上艦要員" | ToWC(), 0, 0, 0, 0, 1, 0, 2, 3, 2, static_cast<Range>(0), 0, 0, 0 } },
		{ 130, Weapon{ 130, L"12.7cm高角砲+高射装置", L"副砲" | ToWC(), 0, 1, 0, 0, 8, 0, 1, 1, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 131, Weapon{ 131, L"25mm三連装機銃 集中配備", L"対空機銃" | ToWC(), 0, 0, 0, 0, 9, 0, 0, 1, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 132, Weapon{ 132, L"零式水中聴音機", L"ソナー" | ToWC(), 0, 0, 0, 0, 0, 11, 1, 1, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 133, Weapon{ 133, L"381mm/50 三連装砲", L"主砲" | ToWC(), 0, 20, 0, 0, 2, 0, -3, -1, 0, static_cast<Range>(4), 0, 0, 0 } },
		{ 134, Weapon{ 134, L"OTO 152mm三連装速射砲", L"副砲" | ToWC(), 1, 8, 0, 0, 2, 0, 1, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 135, Weapon{ 135, L"90mm単装高角砲", L"副砲" | ToWC(), 0, 1, 0, 0, 8, 0, 1, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 136, Weapon{ 136, L"プリエーゼ式水中防御隔壁", L"追加装甲(大型)" | ToWC(), 7, 0, 0, 0, 0, 0, 0, -1, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 137, Weapon{ 137, L"381mm/50 三連装砲改", L"主砲" | ToWC(), 0, 21, 0, 0, 4, 0, -1, -1, 0, static_cast<Range>(4), 0, 0, 0 } },
		{ 138, Weapon{ 138, L"二式大艇", L"大型飛行艇" | ToWC(), 0, 0, 0, 0, 0, 1, 1, 0, 12, static_cast<Range>(0), 0, 0, 0 } },
		{ 139, Weapon{ 139, L"15.2cm連装砲改", L"主砲" | ToWC(), 0, 5, 0, 0, 3, 0, 4, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 140, Weapon{ 140, L"96式150cm探照灯", L"探照灯" | ToWC(), 0, 0, 0, 0, 1, 0, 0, 0, 3, static_cast<Range>(0), 0, 0, 0 } },
		{ 141, Weapon{ 141, L"32号対水上電探改", L"大型電探" | ToWC(), 0, 0, 0, 0, 0, 0, 9, 0, 11, static_cast<Range>(0), 0, 0, 0 } },
		{ 142, Weapon{ 142, L"15m二重測距儀+21号電探改二", L"大型電探" | ToWC(), 1, 1, 0, 0, 8, 0, 9, 1, 7, static_cast<Range>(0), 0, 0, 0 } },
		{ 143, Weapon{ 143, L"九七式艦攻(村田隊)", L"艦上攻撃機" | ToWC(), 0, 0, 12, 0, 1, 5, 2, 0, 4, static_cast<Range>(0), 0, 0, 0 } },
		{ 144, Weapon{ 144, L"天山一二型(村田隊)", L"艦上攻撃機" | ToWC(), 0, 0, 15, 0, 1, 6, 2, 0, 4, static_cast<Range>(0), 0, 0, 0 } },
		{ 145, Weapon{ 145, L"戦闘糧食", L"戦闘糧食" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 146, Weapon{ 146, L"洋上補給", L"洋上補給" | ToWC(), -2, 0, 0, 0, 0, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 147, Weapon{ 147, L"120mm連装砲", L"主砲" | ToWC(), 0, 3, 0, 0, 2, 0, 1, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 148, Weapon{ 148, L"試製南山", L"艦上爆撃機" | ToWC(), 0, 0, 0, 11, 1, 4, 0, 0, 2, static_cast<Range>(0), 0, 0, 0 } },
		{ 149, Weapon{ 149, L"四式水中聴音機", L"ソナー" | ToWC(), 0, 0, 0, 0, 0, 12, 1, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 150, Weapon{ 150, L"秋刀魚の缶詰", L"戦闘糧食" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 151, Weapon{ 151, L"試製景雲(艦偵型)", L"艦上偵察機" | ToWC(), 0, 2, 0, 0, 0, 0, 2, 0, 11, static_cast<Range>(0), 0, 0, 0 } },
		{ 152, Weapon{ 152, L"零式艦戦52型(熟練)", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 9, 0, 1, 2, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 153, Weapon{ 153, L"零戦52型丙(付岩井小隊)", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 10, 0, 1, 2, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 154, Weapon{ 154, L"零戦62型(爆戦/岩井隊)", L"艦上戦闘機" | ToWC(), 0, 0, 0, 4, 7, 3, 1, 2, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 155, Weapon{ 155, L"零戦21型(付岩本小隊)", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 9, 0, 1, 3, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 156, Weapon{ 156, L"零戦52型甲(付岩本小隊)", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 11, 0, 1, 3, 1, static_cast<Range>(0), 0, 0, 0 } },
		{ 157, Weapon{ 157, L"零式艦戦53型(岩本隊)", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 12, 0, 2, 4, 3, static_cast<Range>(0), 0, 0, 0 } },
		{ 158, Weapon{ 158, L"Bf109T改", L"艦上戦闘機" | ToWC(), 0, 1, 0, 0, 8, 0, 0, 4, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 159, Weapon{ 159, L"Fw190T改", L"艦上戦闘機" | ToWC(), 0, 2, 0, 0, 10, 0, 0, 2, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 160, Weapon{ 160, L"10.5cm連装砲", L"副砲" | ToWC(), 0, 3, 0, 0, 6, 0, 2, 1, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 161, Weapon{ 161, L"16inch三連装砲 Mk.7", L"主砲" | ToWC(), 1, 24, 0, 0, 3, 0, 4, 0, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 501, Weapon{ 501, L"5inch単装砲", L"主砲" | ToWC(), 0, 1, 0, 0, 0, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 502, Weapon{ 502, L"5inch連装砲", L"主砲" | ToWC(), 0, 2, 0, 0, 0, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 503, Weapon{ 503, L"3inch単装高角砲", L"主砲" | ToWC(), 0, 1, 0, 0, 1, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 504, Weapon{ 504, L"5inch単装高射砲", L"主砲" | ToWC(), 0, 2, 0, 0, 2, 0, 0, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 505, Weapon{ 505, L"8inch三連装砲", L"主砲" | ToWC(), 0, 8, 0, 0, 2, 0, 0, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 506, Weapon{ 506, L"6inch連装速射砲", L"主砲" | ToWC(), 0, 3, 0, 0, 3, 0, 0, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 507, Weapon{ 507, L"14inch連装砲", L"主砲" | ToWC(), 0, 10, 0, 0, 4, 0, 0, 0, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 508, Weapon{ 508, L"16inch連装砲", L"主砲" | ToWC(), 0, 15, 0, 0, 4, 0, 0, 0, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 509, Weapon{ 509, L"16inch三連装砲", L"主砲" | ToWC(), 0, 20, 0, 0, 5, 0, 0, 0, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 510, Weapon{ 510, L"5inch単装高射砲", L"主砲" | ToWC(), 0, 1, 0, 0, 2, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 511, Weapon{ 511, L"6inch単装砲", L"副砲" | ToWC(), 0, 1, 0, 0, 0, 0, 0, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 512, Weapon{ 512, L"12.5inch連装副砲", L"副砲" | ToWC(), 0, 7, 0, 0, 3, 0, 0, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 513, Weapon{ 513, L"21inch魚雷前期型", L"魚雷" | ToWC(), 0, 0, 2, 0, 0, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 514, Weapon{ 514, L"21inch魚雷後期型", L"魚雷" | ToWC(), 0, 0, 5, 0, 0, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 515, Weapon{ 515, L"高速深海魚雷", L"魚雷" | ToWC(), 0, 0, 10, 0, 0, 0, 0, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 516, Weapon{ 516, L"深海棲艦攻", L"艦上攻撃機" | ToWC(), 0, 0, 4, 0, 0, 2, 0, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 517, Weapon{ 517, L"深海棲艦攻 Mark.II", L"艦上攻撃機" | ToWC(), 0, 0, 6, 0, 0, 4, 0, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 518, Weapon{ 518, L"深海棲艦攻 Mark.III", L"艦上攻撃機" | ToWC(), 0, 0, 11, 0, 4, 7, 0, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 519, Weapon{ 519, L"深海棲艦戦", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 2, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 520, Weapon{ 520, L"深海棲艦戦 Mark.II", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 5, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 521, Weapon{ 521, L"深海棲艦戦 Mark.III", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 9, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 522, Weapon{ 522, L"飛び魚艦戦", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 13, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 523, Weapon{ 523, L"深海棲艦爆", L"艦上爆撃機" | ToWC(), 0, 0, 0, 3, 0, 1, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 524, Weapon{ 524, L"深海棲艦爆 Mark.II", L"艦上爆撃機" | ToWC(), 0, 0, 0, 6, 0, 2, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 525, Weapon{ 525, L"深海棲艦偵察機", L"水上偵察機" | ToWC(), 0, 0, 0, 1, 1, 1, 0, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 526, Weapon{ 526, L"飛び魚偵察機", L"水上偵察機" | ToWC(), 0, 0, 0, 4, 2, 2, 0, 0, 10, static_cast<Range>(0), 0, 0, 0 } },
		{ 527, Weapon{ 527, L"対空レーダ― Mark.I", L"小型電探" | ToWC(), 0, 0, 0, 0, 5, 0, 5, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 528, Weapon{ 528, L"水上レーダ― Mark.I", L"小型電探" | ToWC(), 0, 0, 0, 0, 0, 0, 10, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 529, Weapon{ 529, L"水上レーダ― Mark.II", L"小型電探" | ToWC(), 0, 0, 0, 0, 0, 0, 15, 0, 10, static_cast<Range>(0), 0, 0, 0 } },
		{ 530, Weapon{ 530, L"対空レーダ― Mark.II", L"小型電探" | ToWC(), 0, 0, 0, 0, 10, 0, 5, 0, 10, static_cast<Range>(0), 0, 0, 0 } },
		{ 531, Weapon{ 531, L"深海水上レーダー", L"大型電探" | ToWC(), 0, 0, 0, 0, 5, 5, 24, 3, 16, static_cast<Range>(0), 0, 0, 0 } },
		{ 532, Weapon{ 532, L"深海対空レーダ―", L"大型電探" | ToWC(), 0, 0, 0, 0, 18, 5, 16, 2, 12, static_cast<Range>(0), 0, 0, 0 } },
		{ 533, Weapon{ 533, L"改良型深海タービン", L"機関部強化" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 10, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 534, Weapon{ 534, L"強化型深海缶", L"機関部強化" | ToWC(), 0, 0, 0, 0, 0, 0, 0, 15, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 535, Weapon{ 535, L"対空散弾", L"対空強化弾" | ToWC(), 0, 0, 0, 0, 10, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 536, Weapon{ 536, L"劣化徹甲弾", L"対艦強化弾" | ToWC(), 0, 15, 0, 0, 0, 0, 5, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 537, Weapon{ 537, L"12.7mm機銃", L"対空機銃" | ToWC(), 0, 0, 0, 0, 2, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 538, Weapon{ 538, L"20mm機銃", L"対空機銃" | ToWC(), 0, 0, 0, 0, 4, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 539, Weapon{ 539, L"40mm二連装機関砲", L"対空機銃" | ToWC(), 0, 0, 0, 0, 8, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 540, Weapon{ 540, L"40mm四連装機関砲", L"対空機銃" | ToWC(), 0, 0, 0, 0, 12, 0, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 541, Weapon{ 541, L"深海烏賊魚雷", L"魚雷" | ToWC(), 0, 0, 18, 0, 0, 0, 5, 0, 0, static_cast<Range>(1), 0, 0, 0 } },
		{ 542, Weapon{ 542, L"深海爆雷投射機", L"爆雷" | ToWC(), 0, 0, 0, 0, 0, 7, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 543, Weapon{ 543, L"深海ソナー", L"ソナー" | ToWC(), 0, 0, 0, 0, 0, 9, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 544, Weapon{ 544, L"深海爆雷投射機 Mk.II", L"爆雷" | ToWC(), 0, 0, 0, 0, 0, 13, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 545, Weapon{ 545, L"深海ソナー Mk.II", L"ソナー" | ToWC(), 0, 0, 0, 0, 0, 16, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 546, Weapon{ 546, L"飛び魚艦爆", L"艦上爆撃機" | ToWC(), 0, 0, 0, 10, 8, 7, 0, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 547, Weapon{ 547, L"深海猫艦戦", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 10, 0, 1, 0, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 548, Weapon{ 548, L"深海地獄艦爆", L"艦上爆撃機" | ToWC(), 0, 0, 0, 11, 0, 4, 3, 0, 3, static_cast<Range>(0), 0, 0, 0 } },
		{ 549, Weapon{ 549, L"深海復讐艦攻", L"艦上攻撃機" | ToWC(), 0, 0, 13, 0, 4, 5, 2, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 550, Weapon{ 550, L"5inch連装両用莢砲", L"主砲" | ToWC(), 0, 2, 0, 0, 9, 0, 3, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 551, Weapon{ 551, L"20inch連装砲", L"主砲" | ToWC(), 0, 27, 0, 0, 4, 0, 3, 0, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 552, Weapon{ 552, L"15inch要塞砲", L"主砲" | ToWC(), 0, 13, 0, 0, 0, 0, 4, 0, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 553, Weapon{ 553, L"4inch連装両用砲+CIC", L"主砲" | ToWC(), 0, 5, 0, 0, 15, 0, 5, 0, 0, static_cast<Range>(2), 0, 0, 0 } },
		{ 554, Weapon{ 554, L"深海水上攻撃機", L"水上爆撃機" | ToWC(), 0, 0, 0, 8, 4, 8, 1, 0, 6, static_cast<Range>(0), 0, 0, 0 } },
		{ 555, Weapon{ 555, L"深海水上攻撃機改", L"水上爆撃機" | ToWC(), 0, 0, 0, 13, 9, 10, 3, 0, 7, static_cast<Range>(0), 0, 0, 0 } },
		{ 556, Weapon{ 556, L"深海猫艦戦改", L"艦上戦闘機" | ToWC(), 0, 0, 0, 0, 12, 0, 2, 2, 0, static_cast<Range>(0), 0, 0, 0 } },
		{ 557, Weapon{ 557, L"深海地獄艦爆改", L"艦上爆撃機" | ToWC(), 0, 0, 0, 15, 0, 8, 4, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ 558, Weapon{ 558, L"深海復讐艦攻改", L"艦上攻撃機" | ToWC(), 0, 0, 16, 0, 5, 9, 3, 0, 6, static_cast<Range>(0), 0, 0, 0 } },
		{ 559, Weapon{ 559, L"深海FCS+CIC", L"ソナー" | ToWC(), 0, 0, 0, 0, 6, 20, 5, 3, 3, static_cast<Range>(0), 0, 0, 0 } },
		{ 560, Weapon{ 560, L"深海探照灯", L"探照灯" | ToWC(), 0, 2, 0, 0, 0, 0, 0, 0, 2, static_cast<Range>(0), 0, 0, 0 } },
		{ 563, Weapon{ 563, L"8inch長射程連装砲", L"主砲" | ToWC(), 0, 8, 0, 0, 1, 0, -1, 0, 0, static_cast<Range>(3), 0, 0, 0 } },
		{ 564, Weapon{ 564, L"深海水上偵察観測機", L"水上偵察機" | ToWC(), 0, 0, 0, 0, 2, 2, 5, 0, 5, static_cast<Range>(0), 0, 0, 0 } },
		{ -1, Weapon{} }
	})
{}

// 装備DBからデータを読みだす
Weapon WeaponDB::Get(const int id) const {
	return hash_.at(id);
}
Weapon WeaponDB::Get(const int id, std::nothrow_t) const noexcept {
	try {
		return hash_.at(id);
	}
	catch (...) {
		return{};
	}
}
namespace detail{
	template<CsvParseLevel KammusuLv>Kammusu::DependOnLv PaeseCsvToKammusu(const std::unordered_map<string, std::size_t>& header, const vector<string>& list){
		const int max_hp   = list[header.at("耐久")] | Split('.')[KammusuLv] | to_i();
		const int defense  = list[header.at("装甲")] | Split('.')[KammusuLv] | to_i();
		const int attack   = list[header.at("火力")] | Split('.')[KammusuLv] | to_i();
		const int torpedo  = list[header.at("雷撃")] | Split('.')[KammusuLv] | to_i();
		const int anti_air = list[header.at("対空")] | Split('.')[KammusuLv] | to_i();
		const int evade    = list[header.at("回避")] | Split('.')[KammusuLv] | to_i();
		const int anti_sub = list[header.at("対潜")] | Split('.')[KammusuLv] | to_i();
		const int search   = list[header.at("索敵")] | Split('.')[KammusuLv] | to_i();
		return { max_hp, defense, attack, torpedo, anti_air, evade, anti_sub, search, static_cast<std::size_t>(KammusuLv) };
	}
}
// 艦娘DBのコンストラクタ
KammusuDB::KammusuDB(const char* csv_name) {
	// ファイルを開く
	std::locale::global(std::locale("japanese"));
	ifstream ifs(csv_name);
	FILE_THROW_WITH_MESSAGE_IF(!ifs.is_open(), string(csv_name) + "が正常に読み込めませんでした.")
	// 1行づつ読み込んでいく
	string temp_str;
	getline(ifs, temp_str);
	auto header = temp_str | Split(',') | ToHash();
	while (getline(ifs, temp_str)) {
		if (temp_str.find("null") != string::npos) continue;
		auto list           = temp_str | Split(',');

		const int id              = list[header.at("艦船ID")] | to_i();
		const ShipClass shipclass = static_cast<ShipClass>(1 << ((list[header.at("艦種")] | to_i()) - 1));
		const Speed speed         = list[header.at("速力")] | ToSpeed();
		const Range range         = static_cast<Range>(list[header.at("射程")] | to_i());
		const int slots           = list[header.at("スロット数")] | to_i();
		const bool kammusu_flg    = 0 != (list[header.at("艦娘フラグ")] | to_i());
		const int luck            = list[header.at("運")] | Split('.')[0] | to_i();
		
		wstring name              = char_cvt::shift_jis_to_utf_16(list[header.at("艦名")]);
		vector<int> max_airs      = list[header.at("搭載数")] | Split('.') | to_i();
		vector<int> first_weapons = list[header.at("初期装備")] | Split('.') | to_i();
		// まずLv1の方を代入する
		hash_lv1_[id]  = Kammusu(
			detail::PaeseCsvToKammusu<CsvParseLevel::kLevel1>(header, list),
			id, name, shipclass, luck, speed, range, slots, max_airs, first_weapons, kammusu_flg
		);
		// 次にLv99の方を処理する
		hash_lv99_[id] = Kammusu(
			detail::PaeseCsvToKammusu<CsvParseLevel::kLevel99>(header, list),
			id, move(name), shipclass, luck, speed, range, slots, move(max_airs), move(first_weapons), kammusu_flg
		);
	}
	// ダミーデータを代入する
	hash_lv1_[-1] = Kammusu();
	hash_lv99_[-1] = Kammusu();
}

namespace detail {
	int CalcHPWhenMarriage(const Kammusu& temp_k, const Kammusu &kammusu_lv99)
	{
		// ケッコンによる耐久上昇はややこしい
		int new_max_hp = temp_k.GetMaxHP();
		if (new_max_hp < 10) {
			new_max_hp += 3;
		}
		else if (new_max_hp < 30) {
			new_max_hp += 4;
		}
		else if (new_max_hp < 40) {
			new_max_hp += 5;
		}
		else if (new_max_hp < 50) {
			new_max_hp += 6;
		}
		else if (new_max_hp < 70) {
			new_max_hp += 7;
		}
		else if (new_max_hp <= 90) {
			new_max_hp += 8;
		}
		else {
			new_max_hp += 9;
		}
		return std::min(kammusu_lv99.GetMaxHP(), new_max_hp);
	}
}

// 艦娘DBからデータを読みだす
// idで指定した艦戦IDの艦娘を、レベルがlevelの状態にして返す
// ただし装甲・火力・雷撃・対空は改修MAXの状態とする
Kammusu KammusuDB::Get(const int id, const int level) const {
	if(hash_lv99_.find(id) == hash_lv99_.end()) return Kammusu();
	Kammusu temp_k = hash_lv99_.at(id);
	const Kammusu &kammusu_lv1 = hash_lv1_.at(id);
	const Kammusu &kammusu_lv99 = hash_lv99_.at(id);
	// 練度で上昇する箇所を補完する
	temp_k.SetMaxHP(kammusu_lv1.GetMaxHP());
	temp_k.SetEvade(int(1.0 * (kammusu_lv99.GetEvade() -kammusu_lv1.GetEvade()) * level / 99 + kammusu_lv1.GetEvade()));
	temp_k.SetAntiSub(int(1.0 * (kammusu_lv99.GetAntiSub() - kammusu_lv1.GetAntiSub()) * level / 99 + kammusu_lv1.GetAntiSub()));
	temp_k.SetSearch(int(1.0 * (kammusu_lv99.GetSearch() - kammusu_lv1.GetSearch()) * level / 99 + kammusu_lv1.GetSearch()));
	temp_k.SetLevel(level);
	if (level >= 100) {
		temp_k.SetMaxHP(detail::CalcHPWhenMarriage(temp_k, kammusu_lv99));
		// ケッコンによる運上昇は+3～+6までランダムなのでとりあえず+4とした
		temp_k.SetLuck(temp_k.GetLuck() + 4);
	}
	return temp_k;
}

// 集計結果保存用クラスのコンストラクタ
ResultStat::ResultStat(const vector<Result> &result_db, const vector<vector<Kammusu>> &unit) noexcept {
	hp_min_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, 10000)));
	hp_max_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, -1)));
	damage_min_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, 10000)));
	damage_max_.resize(kBattleSize, vector<vector<int>>(kMaxFleetSize, vector<int>(kMaxUnitSize, -1)));

	hp_ave_.resize(kBattleSize, vector<vector<double>>(kMaxFleetSize, vector<double>(kMaxUnitSize)));
	hp_sd_.resize(kBattleSize, vector<vector<double>>(kMaxFleetSize, vector<double>(kMaxUnitSize, 0.0)));
	damage_ave_.resize(kBattleSize, vector<vector<double>>(kMaxFleetSize, vector<double>(kMaxUnitSize)));
	damage_sd_.resize(kBattleSize, vector<vector<double>>(kMaxFleetSize, vector<double>(kMaxUnitSize, 0.0)));

	mvp_count_.resize(kMaxFleetSize, vector<int>(kMaxUnitSize, 0));
	heavy_damage_count_.resize(kMaxFleetSize, vector<int>(kMaxUnitSize, 0));

	win_reason_count_.resize(int(WinReason::Types), 0);

	all_count_ = result_db.size();
	reader_killed_count_ = 0;

	for (size_t ti = 0; ti < all_count_; ++ti) {
		++win_reason_count_[int(result_db[ti].JudgeWinReason())];
		bool special_mvp_flg = result_db[ti].GetNightFlg() && (unit.size() > 1);
		for (size_t fi = 0; fi < unit.size(); ++fi) {
			size_t mvp_index = 0;
			int mvp_damage = -1;
			for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
				// 残り耐久・与ダメージ
				for (size_t bi = 0; bi < kBattleSize; ++bi) {
					auto hp = result_db[ti].GetHP(bi, fi, ui);
					auto damage = result_db[ti].GetDamage(bi, fi, ui);
					hp_min_[bi][fi][ui] = std::min(hp_min_[bi][fi][ui], hp);
					hp_max_[bi][fi][ui] = std::max(hp_max_[bi][fi][ui], hp);
					damage_min_[bi][fi][ui] = std::min(damage_min_[bi][fi][ui], damage);
					damage_max_[bi][fi][ui] = std::max(damage_max_[bi][fi][ui], damage);
					hp_ave_[bi][fi][ui] += hp;
					damage_ave_[bi][fi][ui] += damage;
				}
				// MVP・大破
				auto damage = result_db[ti].GetDamage(0, fi, ui, special_mvp_flg);
				if (damage > mvp_damage) {
					mvp_index = ui;
					mvp_damage = damage;
				}
				if (result_db[ti].GetHP(0, fi, ui) * 4 <= unit[fi][ui].GetMaxHP()) ++heavy_damage_count_[fi][ui];
			}
			++mvp_count_[fi][mvp_index];
		}
		// 旗艦撃破
		if (result_db[ti].GetHP(1, 0, 0) == 0) ++reader_killed_count_;
	}
	// 平均
	double all_count_inv = 1.0 / all_count_;
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		for (size_t fi = 0; fi < kMaxFleetSize; ++fi) {
			for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
				hp_ave_[bi][fi][ui] *= all_count_inv;
				damage_ave_[bi][fi][ui] *= all_count_inv;
			}
		}
	}
	// 標本標準偏差
	if (all_count_ > 1) {
		for (size_t ti = 0; ti < all_count_; ++ti) {
			for (size_t bi = 0; bi < kBattleSize; ++bi) {
				for (size_t fi = 0; fi < kMaxFleetSize; ++fi) {
					for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
						double temp1 = hp_ave_[bi][fi][ui] - result_db[ti].GetHP(bi, fi, ui);
						hp_sd_[bi][fi][ui] += temp1 * temp1;
						double temp2 = damage_ave_[bi][fi][ui] - result_db[ti].GetDamage(bi, fi, ui);
						damage_sd_[bi][fi][ui] += temp2 * temp2;
					}
				}
			}
		}
		double all_count_inv2 = 1.0 / (all_count_ - 1);
		for (size_t bi = 0; bi < kBattleSize; ++bi) {
			for (size_t fi = 0; fi < kMaxFleetSize; ++fi) {
				for (size_t ui = 0; ui < kMaxUnitSize; ++ui) {
					hp_sd_[bi][fi][ui] = sqrt(hp_sd_[bi][fi][ui] * all_count_inv2);
					damage_sd_[bi][fi][ui] = sqrt(damage_sd_[bi][fi][ui] * all_count_inv2);
				}
			}
		}
	}
}

// 結果を標準出力に出力する
void ResultStat::Put(const vector<Fleet> &fleet) const noexcept {
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		wcout << (bi == kFriendSide ? L"自" : L"敵") << L"艦隊：" << endl;
		const auto &unit = fleet[bi].GetUnit();
		for (size_t fi = 0; fi < unit.size(); ++fi) {
			wcout << L"　第" << (fi + 1) << L"艦隊：" << endl;
			for (size_t ui = 0; ui < unit[fi].size(); ++ui) {
				wcout << L"　　" << unit[fi][ui].GetNameLv() << endl;
				wcout << L"　　　残耐久：" << L"[" << hp_min_[bi][fi][ui] << L"～" << hp_ave_[bi][fi][ui] << L"～" << hp_max_[bi][fi][ui] << L"] σ＝";
				if (all_count_ > 1) wcout << hp_sd_[bi][fi][ui] << endl; else wcout << L"―" << endl;
				wcout << L"　　　与ダメージ：" << L"[" << damage_min_[bi][fi][ui] << L"～" << damage_ave_[bi][fi][ui] << L"～" << damage_max_[bi][fi][ui] << L"] σ＝";
				if (all_count_ > 1) wcout << damage_sd_[bi][fi][ui] << endl; else wcout << L"―" << endl;
				if (bi == 0) {
					wcout << L"　　　MVP率：" << (100.0 * mvp_count_[fi][ui] / all_count_) << L"％ ";
					wcout << L"大破率：" << (100.0 * heavy_damage_count_[fi][ui] / all_count_) << L"％" << endl;
				}
			}
		}
	}
	wcout << L"旗艦撃破率：" << (100.0 * reader_killed_count_ / all_count_) << L"％" << endl;
	wcout << L"勝率：" << (100.0 * (win_reason_count_[int(WinReason::SS)] + win_reason_count_[int(WinReason::S)]
		+ win_reason_count_[int(WinReason::A)] + win_reason_count_[int(WinReason::B)]) / all_count_) << L"％" << endl;
	for (size_t i = 0; i < int(WinReason::Types); ++i) {
		wcout << L"　" << kWinReasonStrL[i] << L"：" << (100.0 * win_reason_count_[i] / all_count_) << L"％" << endl;
	}
}

// 結果をJSONファイルに出力する
void ResultStat::Put(const vector<Fleet> &fleet, const string &file_name, const bool &json_prettify_flg) const {
	ofstream fout(file_name);
	FILE_THROW_WITH_MESSAGE_IF(!fout.is_open(), "計算結果が正常に保存できませんでした.")
	picojson::object o;
	o["reader_killed_per"] = picojson::value(100.0 * reader_killed_count_ / all_count_);
	o["win_per"] = picojson::value(100.0 * (win_reason_count_[int(WinReason::SS)] + win_reason_count_[int(WinReason::S)] +
		win_reason_count_[int(WinReason::A)] + win_reason_count_[int(WinReason::B)]) / all_count_);
	picojson::object ox;
	for (size_t i = 0; i < int(WinReason::Types); ++i) {
		ox[kWinReasonStrS[i]] = picojson::value(100.0 * win_reason_count_[i] / all_count_);
	}
	o["win_reason_per"] = picojson::value(ox);
	for (size_t bi = 0; bi < kBattleSize; ++bi) {
		picojson::object o1;
		const auto &unit = fleet[bi].GetUnit();
		for (size_t fi = 0; fi < unit.size(); ++fi) {
			picojson::object o2;
			for (size_t ui = 0; ui < unit[fi].size(); ++ui) {
				picojson::object o3;
				o3["id"] = picojson::value(to_string(unit[fi][ui].GetID()));
				o3["lv"] = picojson::value(1.0 * unit[fi][ui].GetLevel());
				{
					picojson::object o4;
					o4["min"] = picojson::value(1.0 * hp_min_[bi][fi][ui]);
					o4["ave"] = picojson::value(hp_ave_[bi][fi][ui]);
					o4["max"] = picojson::value(1.0 * hp_max_[bi][fi][ui]);
					o4["sd"] = picojson::value((all_count_ > 1 ? 1.0 * hp_sd_[bi][fi][ui] : -1.0));
					o3["hp"] = picojson::value(o4);
				}
				{
					picojson::object o4;
					o4["min"] = picojson::value(1.0 * damage_min_[bi][fi][ui]);
					o4["ave"] = picojson::value(damage_ave_[bi][fi][ui]);
					o4["max"] = picojson::value(1.0 * damage_max_[bi][fi][ui]);
					o4["sd"] = picojson::value((all_count_ > 1 ? 1.0 * damage_sd_[bi][fi][ui] : -1.0));
					o3["damage"] = picojson::value(o4);
				}
				if (bi == 0) {
					o3["mvp_per"] = picojson::value(100.0 * mvp_count_[fi][ui] / all_count_);
					o3["heavy_damage_per"] = picojson::value(100.0 * heavy_damage_count_[fi][ui] / all_count_);
				}
				o2["s" + to_string(ui + 1)] = picojson::value(o3);
			}
			o1["f" + to_string(fi + 1)] = picojson::value(o2);
		}
		o["b" + to_string(bi + 1)] = picojson::value(o1);
	}

	fout << picojson::value(o).serialize(json_prettify_flg) << endl;
}

// 文字列をデリミタで区切り分割する
//vector<string> Split(const string &str, char delim) {
//	vector<string> re;
//	size_t current = 0;
//	for (size_t found; (found = str.find_first_of(delim, current)) != string::npos; current = found + 1) {
//		re.emplace_back(str, current, found - current);
//	}
//	re.emplace_back(str, current, str.size() - current);
//	return re;
//}

// 文字列(ファイル名)から拡張子を取り出す
// 参考：http://qiita.com/selflash/items/d6bdd0fcb677f4f8ca24
string GetExtension(const string &path) {
	// ドットの位置を検出する
	auto dot_pos = path.find_last_of('.');
	if (dot_pos == string::npos) return "";
	// 切り取る
	auto ext = path.substr(dot_pos + 1, path.find_last_not_of(' ') - dot_pos);
	// 拡張子を小文字化する
	for (auto& c : ext) c = std::tolower(c);
	return ext;
}
