﻿#pragma once

#include <algorithm>
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
#pragma warning( disable : 4592)

#define KCS_DEBUG_MODE

using std::cout;
using std::bitset;
using std::endl;
using std::fill;
using std::get;
using std::ifstream;
using std::move;
using std::stoi;
using std::string;
using std::wstring;
using std::to_string;
using std::tuple;
using std::unordered_map;
using std::vector;
using std::wcout;

const int kBattleSize = 2;		//戦闘で戦うのは敵と味方の「2つ」
const int kMaxFleetSize = 2;	//
const int kMaxUnitSize = 6;		//艦隊に含まれる最大艦数
const int kFriendSide = 0, kEnemySide = 1;	//味方および敵陣営の番号付け

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

namespace detail {
	struct to_i_helper {};
	inline int operator|(const std::string& str, to_i_helper) {
		return std::stoi(str);
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
inline detail::to_i_helper to_i() { return{}; }
template<typename T>
inline constexpr detail::limit_helper<T> limit(const T &val_min, const T &val_max) { return{ val_min, val_max }; }
