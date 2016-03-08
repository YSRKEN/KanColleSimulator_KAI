#ifndef KCS_KAI_INC_BASE_HPP_
#define KCS_KAI_INC_BASE_HPP_

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <type_traits>
#include "exception.hpp"
#include "arithmetic_convert.hpp"
#pragma warning( disable : 4592)

using std::cout;
using std::bitset;
using std::endl;
using std::fill;
using std::ifstream;
using std::move;
using std::ofstream;
using std::stoi;
using std::string;
using std::wstring;
using std::to_string;
using std::tuple;
using std::unordered_map;
using std::vector;
using std::wcout;
using std::size_t;

constexpr int kBattleSize = 2;		//戦闘で戦うのは敵と味方の「2つ」
constexpr int kMaxFleetSize = 2;	//
constexpr size_t kMaxUnitSize = 6;		//艦隊に含まれる最大艦数
constexpr int kFriendSide = 0;
constexpr size_t kEnemySide = 1;	//味方および敵陣営の番号付け

enum class WeaponClass : std::uint64_t;

struct KammusuIndex {
	std::size_t fleet_no;
	std::size_t fleet_i;
};

// 射程
enum Range { kRangeNone, kRangeShort, kRangeMiddle, kRangeLong, kRangeVeryLong };
const wstring kRangeStr[] = { L"無", L"短", L"中", L"長", L"超長" };

// 陣形
enum Formation { kFormationTrail, kFormationSubTrail, kFormationCircle, kFormationEchelon, kFormationAbreast };
const wstring kFormationStr[] = { L"単縦陣", L"複縦陣", L"輪形陣", L"梯形陣", L"単横陣" };

// 値を制限する
template<typename T>
constexpr T limit(const T &val, const T &val_min, const T &val_max) {
	return (val < val_min) ? val_min : (val_max < val) ? val_max : val;
}
constexpr double LimitCap(const double &val, const double &val_cap) {
	return (val < val_cap) ? val : (val_cap + sqrt(val - val_cap));
}

namespace detail {
	template<typename T, std::enable_if_t<std::is_arithmetic<T>::value, std::nullptr_t> = nullptr> struct to_i_helper {};
	template<typename CharType, typename T>
	inline int operator|(const std::basic_string<CharType>& str, to_i_helper<T>) {
		return atithmetic_cvt::from_str<int>(str);
	}
	template<typename CharType, typename T>
	inline T operator|(const CharType* str, to_i_helper<T>) {
		return atithmetic_cvt::from_str<int>(std::basic_string<CharType>(str));
	}
	template<typename T>
	struct limit_helper{
		constexpr limit_helper(const T &val_min, const T &val_max) : min(val_min), max(val_max) {}
		const T& min;
		const T& max;
	};
	template<typename T>
	inline constexpr T operator|(const T &val, const limit_helper<T>& info) {
		return (val < info.min) ? info.min : (info.max < val) ? info.max : val;
	}
}
inline detail::to_i_helper<int> to_i() noexcept { return{}; }
inline detail::to_i_helper<size_t> to_sz() noexcept { return{}; }

template<typename T>
inline constexpr detail::limit_helper<T> limit(const T &val_min, const T &val_max) noexcept { return{ val_min, val_max }; }

#endif //KCS_KAI_INC_BASE_HPP_
