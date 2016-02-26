#pragma once

#include <algorithm>
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
using std::tuple;
using std::unordered_map;
using std::vector;

const uint_fast8_t kBattleSize = 2;	//í“¬‚Åí‚¤‚Ì‚Í“G‚Æ–¡•û‚Ìu2‚Âv

// Ë’ö
enum Range { kRangeNone, kRangeShort, kRangeMiddle, kRangeLong, kRangeVeryLong };
const string kRangeStr[] = { "–³", "’Z", "’†", "’·", "’´’·" };
