#ifndef NOMINMAX
#define NOMINMAX
#endif
#if defined(_WIN32) || defined(_WIN64)
#	define MY_ARC_FOR_WINDWOS 1
#	include <Windows.h>
#endif // defined(_WIN32) || defined(_WIN64)
#include "random.hpp"
#include <stdlib.h> //rand_s, malloc
#include <algorithm>//std::generate
#include <ctime>//clock(), time()
#include <functional>//std::ref in gcc
#include <chrono>
#if !defined(_MSC_VER) || !defined(__clang__)
#	ifndef __INTEL_COMPILER
#		include <immintrin.h>
#		if defined(_WIN32) || defined(_WIN64)
#		include <intrin.h>
#		else
#			include <x86intrin.h>
#		endif //defined(_WIN32) || defined(_WIN64)
#		ifdef __GNUC__
#			include <cpuid.h>
#		endif //__GNUC__
#	endif //__INTEL_COMPILER
#	include <cstring>
using std::uint32_t;
namespace intrin {
	constexpr uint32_t RDRAND_MASK = 1U << 30U;
	constexpr uint32_t RDSEED_MASK = 1U << 18U;

	struct regs_t { uint32_t EAX, EBX, ECX, EDX; };
	union register_str_cvt {
		uint32_t u32[4];
		char str[16];
	};
	regs_t get_cpuid(unsigned int level) {
		regs_t re = { 0 };
		static_assert(sizeof(re) == (sizeof(uint32_t) * 4), "illegal size of struct regs_t ");
#	if ( defined(__INTEL_COMPILER) || defined(_MSC_VER) )
		__cpuid(reinterpret_cast<int*>(&re), static_cast<int>(level));
#	elif defined(__GNUC__)
		__get_cpuid(level, &re.EAX, &re.EBX, &re.ECX, &re.EDX);
#	endif
		return re;
	}
	bool IsIntelCPU() {
		const auto id = get_cpuid(0);
		register_str_cvt vender = { id.EBX, id.EDX, id.ECX , 0 };
		return (0 == std::strcmp(vender.str, "GenuineIntel"));
	}
	bool IsAMDCPU() {
		const auto id = get_cpuid(0);
		register_str_cvt vender = { id.EBX, id.EDX, id.ECX , 0 };
		return (0 == std::strcmp(vender.str, "AuthenticAMD"));
	}
	bool IsRDRANDsupport() {
		if (!IsIntelCPU() && !IsAMDCPU()) return false;
		const auto reg = get_cpuid(1);//If RDRAND is supported, the bit 30 of the ECX register is set after calling CPUID standard function 01H.
		return (RDRAND_MASK == (reg.ECX & RDRAND_MASK));
	}
	bool IsRDSEEDsupport() {
		if (!IsIntelCPU()) return false;
		const auto reg = get_cpuid(7);//If RDSEED is supported, the bit 18 of the EBX register is set after calling CPUID standard function 07H.
		return (RDSEED_MASK == (reg.EBX & RDSEED_MASK));
	}
}
#endif//!defined(_MSC_VER) || !defined(__clang__)
using seed_v_t = std::vector<unsigned int>;
seed_v_t create_seed_v() {
	const auto begin_time = std::chrono::high_resolution_clock::now();
	std::random_device rnd;// ランダムデバイス
	seed_v_t sed_v(9);// 初期化用ベクター
	std::generate(sed_v.begin(), sed_v.end(), std::ref(rnd));// ベクタの初期化
#if !defined(_MSC_VER) || !defined(__clang__)//Clang with Microsoft CodeGenはasmに対応していない
	if (intrin::IsRDRANDsupport()) {//RDRAND命令の結果もベクターに追加
		for (unsigned int i = 0; i < 4; i++) {
			unsigned int rdrand_value = 0;
#	if defined(_MSC_VER) || defined(__INTEL_COMPILER)
			_rdrand32_step(&rdrand_value);
#	else//defined(_MSC_VER) || defined(__INTEL_COMPILER)
			__builtin_ia32_rdrand32_step(&rdrand_value);
#	endif//defined(_MSC_VER) || defined(__INTEL_COMPILER)
			if (0 != rdrand_value) {
				sed_v.push_back((rdrand_value < std::numeric_limits<decltype(rdrand_value)>::max() - i) ? rdrand_value + i : rdrand_value);
			}
		}
	}
	if (intrin::IsRDSEEDsupport()) {
		for (unsigned int i = 0; i < 5; i++) {
			unsigned int rdseed_value = 0;
#	if defined(_MSC_VER) || defined(__INTEL_COMPILER)
			_rdseed32_step(&rdseed_value);
#	else//defined(_MSC_VER) || defined(__INTEL_COMPILER)
			__builtin_ia32_rdseed32_step(&rdrand_value);
#	endif//defined(_MSC_VER) || defined(__INTEL_COMPILER)
			if (0 != rdseed_value) {
				sed_v.push_back((rdseed_value < std::numeric_limits<decltype(rdseed_value)>::max() - i) ? rdseed_value + i : rdseed_value);
			}
		}
	}
#endif//!defined(_MSC_VER) || !defined(__clang__)
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
	if(sizeof(void*) > 4)sed_v.push_back(static_cast<std::uint_least32_t>(address >> 32));
	const auto end_time = std::chrono::high_resolution_clock::now();
	sed_v.push_back(static_cast<std::uint_least32_t>((end_time - begin_time).count()));
	return sed_v;
}

SharedRand make_SharedRand()
{
	const auto sed_v = create_seed_v();
	std::seed_seq seq(sed_v.begin(), sed_v.end());
	return SharedRand(std::mt19937(seq));
}
