#pragma once
#include <fstream>
#include "char_convert.hpp"
inline void skip_utf8_bom(std::ifstream& fs, char_cvt::char_enc& enc) {
	int dst[3];
	for (auto& i : dst) i = fs.get();
	constexpr int utf8[] = { 0xEF, 0xBB, 0xBF };
	if (!std::equal(std::begin(dst), std::end(dst), utf8)) fs.seekg(0);
	else enc = char_cvt::char_enc::utf8;
}
