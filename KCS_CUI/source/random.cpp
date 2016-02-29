#if defined(_WIN32) || defined(_WIN64)
#if (!defined(_MSC_VER) || _MSC_VER >= 1400) && (!defined(DISABE_CRT_RAND_S))//Visual Studio 2005よりrand_sは実装された
#define _CRT_RAND_S
#endif //_MSC_VER >= 1400
#define MY_ARC_FOR_WINDWOS 1
#include <Windows.h>
#endif // defined(_WIN32) || defined(_WIN64)
#include "random.hpp"
#include <stdlib.h> //rand_s, malloc
#include <algorithm>//std::generate
#include <ctime>//clock(), time()
#include <functional>//std::ref in gcc
#include <chrono>
#if defined(_MSC_VER) && !defined(__clang__)
#ifndef __INTEL_COMPILER
#include <immintrin.h>
#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#else
#include <x86intrin.h>
#endif //defined(_WIN32) || defined(_WIN64)
#ifdef __GNUC__
#include <cpuid.h>
#endif //__GNUC__
#endif //__INTEL_COMPILER
#include <cstring>
// Defines the bit mask used to examine the ecx register returned by cpuid.
// (The 30th bit is set.)
using std::uint32_t;
namespace intrin {
	constexpr uint32_t RDRAND_MASK = 0x40000000;

	typedef struct { uint32_t EAX, EBX, ECX, EDX; } regs_t;
	regs_t get_cpuid(unsigned int level) {
		regs_t re = { 0 };
		static_assert(sizeof(re) == (sizeof(uint32_t) * 4), "illegal size of struct regs_t ");
#if ( defined(__INTEL_COMPILER) || defined(_MSC_VER) )
		__cpuid(reinterpret_cast<int*>(&re), static_cast<int>(level));
#elif defined(__GNUC__)
		__get_cpuid(level, &re.EAX, &re.EBX, &re.ECX, &re.EDX);//error: '__get_cpuid' was not declared in this scope
#endif
		return re;
	}
	bool IsIntelCPU() {
		const auto id = get_cpuid(0);
		uint32_t vender[4] = { id.EAX, id.ECX, id.EBX , 0 };
		return (0 == std::strcmp(reinterpret_cast<char*>(vender), "GenuineIntel"));
	}
	bool IsRDRANDsupport() {
		if (!IsIntelCPU()) return false;
		const auto reg = get_cpuid(1);
		return (RDRAND_MASK == (reg.ECX & RDRAND_MASK));
	}
}
#endif//defined(_MSC_VER) && !defined(__clang__)
using seed_v_t = std::vector<std::uint_least32_t>;
seed_v_t create_seed_v() {
	const auto begin_time = std::chrono::high_resolution_clock::now();
	std::random_device rnd;// ランダムデバイス
	seed_v_t sed_v(10);// 初期化用ベクター
	std::generate(sed_v.begin(), sed_v.end(), std::ref(rnd));// ベクタの初期化
#if defined(_MSC_VER) && !defined(__clang__)
	if (intrin::IsRDRANDsupport()) {//RDRAND命令の結果もベクターに追加
		for (std::size_t i = 0; i < 4; i++) {
			unsigned int rdrand_value = 0;
#ifndef __GNUC__
			_rdrand32_step(&rdrand_value);
#else
			__builtin_ia32_rdrand32_step(&rdrand_value);
#endif
			if (0 != rdrand_value) {
				sed_v.push_back(rdrand_value & i);
			}
		}
	}
#endif
#ifdef _CRT_RAND_S
	unsigned int rand_s_value = 0;
	rand_s(&rand_s_value);
	if (0 != rand_s_value) {
		sed_v.push_back(rand_s_value);
	}
#endif //_CRT_RAND_S
#ifdef MY_ARC_FOR_WINDWOS
	POINT point;
	GetCursorPos(&point);
	sed_v.push_back(point.x);
	sed_v.push_back(point.y);
#endif //MY_ARC_FOR_WINDWOS
	sed_v.push_back(static_cast<std::uint_least32_t>(clock()));//clock関数の結果もベクターに追加
	sed_v.push_back(static_cast<std::uint_least32_t>(time(nullptr)));//time関数の結果もベクターに追加
	//ヒープ領域のアドレスもベクターに追加
	auto heap = std::make_unique<char>();
	const auto address = reinterpret_cast<std::uint_least64_t>(heap.get());
	sed_v.push_back(static_cast<std::uint_least32_t>(address));
	sed_v.push_back(static_cast<std::uint_least32_t>(address >> 32));
	const auto end_time = std::chrono::high_resolution_clock::now();
	sed_v.push_back(static_cast<std::uint_least32_t>((end_time - begin_time).count()));
	return sed_v;
}
