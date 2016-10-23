#ifndef KCS_KAI_INC_RANDOM_HPP_
#define KCS_KAI_INC_RANDOM_HPP_
#include <random>
#include "std_future.hpp"
class SharedRand {
private:
	static thread_local std::mt19937 generator_;
	~SharedRand() = delete;
public:
	static std::mt19937& get() {
		return generator_;
	}

	//[a, b]の一様整数乱数を取り出す
	template<class Int>
	static Int RandInt(const Int min, const Int max) {
		return std::uniform_int_distribution<Int>{min, max}(generator_);
	}
	//[0, n - 1]の一様整数乱数を取り出す
	template<class Int>
	static Int RandInt(const Int n) {
		return RandInt<Int>(0, n - 1); 
	}
	static double RandReal(const double min = 0, const double max = 1) {
		return std::uniform_real_distribution<double>{min, max}(generator_);
	}
	static bool RandBool(double rate = 0.5) {
		return std::bernoulli_distribution{ rate }(generator_);
	}
	template<typename T>
	static const auto& select_random_in_range(const T& v, size_t size) {
		return v[RandInt(size)];
	}
	template<typename T>
	static const auto& select_random_in_range(const T& v) {
		INVAID_ARGUMENT_THROW_WITH_MESSAGE_IF(std_future::size(v) < 1, "range is empty.");
		return select_random_in_range(v, std_future::size(v));
	}
};
#endif //KCS_KAI_INC_RANDOM_HPP_
