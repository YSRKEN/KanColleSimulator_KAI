#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#pragma warning( disable : 4592)

using std::cout;
using std::fill;
using std::ifstream;
using std::move;
using std::stoi;
using std::string;
using std::to_string;
using std::tuple;
using std::unordered_map;
using std::vector;

#define TIME_ELAPSED(...) time_elapsed_lambda([&]{__VA_ARGS__;})
template <typename inner_loop>
void time_elapsed_lambda(inner_loop body)
{
	auto t0 = std::chrono::high_resolution_clock::now();
	body();
	auto t1 = std::chrono::high_resolution_clock::now();
	cout << "処理時間：" << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << "[ms]\n";
}

const uint_fast8_t kBattleSize = 2;	//戦闘で戦うのは敵と味方の「2つ」

// 射程
enum Range { kRangeNone, kRangeShort, kRangeMiddle, kRangeLong, kRangeVeryLong };
const string kRangeStr[] = { "無", "短", "中", "長", "超長" };

// 陣形
enum Formation { kFormationTrail, kFormationSubTrail, kFormationCircle, kFormationEchelon, kFormationAbreast };
const string kFormationStr[] = { "単縦陣", "複縦陣", "輪形陣", "梯形陣", "単横陣" };

// 値を制限する
template<typename T>
constexpr T limit(const T &val, const T &val_min, const T &val_max) {
	return (val < val_min) ? val_min : (val_max < val) ? val_max : val;
}
