#ifndef KCS_KAI_INC_RANDOM_HPP_
#define KCS_KAI_INC_RANDOM_HPP_
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

// based on XORSHIFT-ADD version 1.2 (MIT License)
// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/XSADD/index-jp.html
// https://github.com/MersenneTwister-Lab/XSadd
class XorShiftPlus {
public:
	typedef uint32_t result_type;
	static constexpr result_type min() { return 0; }
	static constexpr result_type max() { return (result_type)-1; }
private:
	const int LOOP = 8;
	result_type state[4];
	void period_certificate() noexcept {
		if (state[0] == 0 && state[1] == 0 && state[2] == 0 && state[3] == 0) {
			state[0] = 'X';
			state[1] = 'S';
			state[2] = 'A';
			state[3] = 'D';
		}
	}
	void next_state() noexcept {
		const int sh1 = 15;
		const int sh2 = 18;
		const int sh3 = 11;
		result_type t = state[0];
		t ^= t << sh1;
		t ^= t >> sh2;
		t ^= state[3] << sh3;
		state[0] = state[1];
		state[1] = state[2];
		state[2] = state[3];
		state[3] = t;
	}
public:
	XorShiftPlus(result_type seed) noexcept : state{ seed, 0, 0, 0 } {
		for (int i = 1; i < LOOP; i++)
			state[i & 3] ^= i + UINT32_C(1812433253) * (state[(i - 1) & 3] ^ (state[(i - 1) & 3] >> 30));
		period_certificate();
		for (int i = 0; i < LOOP; i++)
			next_state();
	}
	result_type operator()() noexcept {
		next_state();
		return state[3] + state[2];
	}
};

XorShiftPlus& GetRandomEngine();

inline double RandReal() {
	static std::uniform_real_distribution<double> dist;
	return dist(GetRandomEngine());
}

inline double RandReal(double min, double max) {
	std::uniform_real_distribution<double> dist(min, max);
	return dist(GetRandomEngine());
}

template<class Int = int>
inline Int RandInt(Int max) {
	std::uniform_int_distribution<Int> dist(0, max);
	return dist(GetRandomEngine());
}

inline bool RandBool(double rate = 0.5) {
	std::bernoulli_distribution dist(rate);
	return dist(GetRandomEngine());
}

template<class InputIterator>
inline InputIterator select_random_in_range(InputIterator first, InputIterator last) {
	return first + RandInt<size_t>(last - first - 1);
}

#endif //KCS_KAI_INC_RANDOM_HPP_
