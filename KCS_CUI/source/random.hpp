#ifndef KCS_KAI_INC_RANDOM_HPP_
#define KCS_KAI_INC_RANDOM_HPP_
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <random>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <unordered_set>
#include <functional>
#include <limits>
#include <numeric>
#include "exception.hpp"
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

using seed_v_t = std::vector<std::uint_least32_t>;
seed_v_t create_seed_v();
namespace detail {
	template<typename T, std::enable_if_t<std::is_integral<T>::value, std::nullptr_t> = nullptr> 
	auto diff(T n1, T n2) -> std::make_unsigned_t<T> {
		if (n1 < n2) std::swap(n1, n2);
		return static_cast<std::make_unsigned_t<T>>(n1 - n2);
	}
}

class SharedRand {
private:
	std::shared_ptr<std::mt19937> generator_;
	std::uniform_real_distribution<double> defalut_dist_;
public:
	template<typename RandType, std::enable_if_t<std::is_arithmetic<RandType>::value, std::nullptr_t> = nullptr>
	using distribution_t = std::conditional_t<std::is_integral<RandType>::value, std::uniform_int_distribution<RandType>, std::uniform_real_distribution<RandType>>;
	SharedRand() : generator_(), defalut_dist_(0.0, 1.0) {}
	explicit SharedRand(unsigned int seed) : generator_(new std::mt19937(seed)), defalut_dist_(0.0, 1.0) {}
	explicit SharedRand(std::seed_seq& seed) : generator_(new std::mt19937(seed)), defalut_dist_(0.0, 1.0) {}
	explicit SharedRand(std::mt19937&& engine) : generator_(new std::mt19937(std::move(engine))), defalut_dist_(0.0, 1.0) {}
	SharedRand(const SharedRand& o) noexcept : generator_(o.generator_), defalut_dist_(o.defalut_dist_) {}
	SharedRand(SharedRand&& o) noexcept : generator_(std::move(o.generator_)), defalut_dist_(o.defalut_dist_) {}
	SharedRand& operator=(const SharedRand& o) {
		this->defalut_dist_ = o.defalut_dist_;
		this->generator_ = o.generator_;
		return *this;
	}
	SharedRand& operator=(SharedRand&& o) {
		this->defalut_dist_ = std::move(o.defalut_dist_);
		this->generator_ = std::move(o.generator_);
		return *this;
	}

	//utility
	bool is_generatable() const noexcept {
		return generator_.use_count() && generator_.get();
	}
	explicit operator bool() const noexcept {
		return this->is_generatable();
	}
	bool operator!() const noexcept {
		return this->is_generatable();
	}
	std::mt19937& get(std::nothrow_t) const noexcept {
		return *this->generator_;
	}
	std::mt19937& get() const {
		MISSING_RAND_GENERATOR_THROW_WITH_MESSAGE_IF(!this->is_generatable(), "rand generator is not initialized.")
		else {
			return *this->generator_;
		}
	}
	std::mt19937& operator*() const {
		return this->get();
	}

	//generater
	double RandReal() const {
		return this->defalut_dist_(this->get());
	}
	double operator()() const {
		return this->RandReal();
	}
	template<typename RandType>
	RandType generate(const RandType min, const RandType max) const {
		return distribution_t<RandType>(min, max)(this->get());
	}
	template<typename RandType>
	RandType operator()(const RandType min, const RandType max) const {
		return this->generate(min, max);
	}
	//[a, b]の一様整数乱数を取り出す
	int RandInt(const int min, const int max) const {
		return this->generate(min, max);
	}
	//[0, n - 1]の一様整数乱数を取り出す
	int RandInt(const int n) const {
		return this->RandInt(0, n -1); 
	}
	double RandReal(const double min, const double max) const {
		return this->generate(min, max);
	}
	bool RandBool(double rate = 0.5) const {
		return std::bernoulli_distribution(rate)(this->get());
	}
	template<typename T> const T& select_random_in_range(const std::vector<T>& v) const {
		return v[this->generate<size_t>(0, v.size() - 1)];
	}
	template<typename T, size_t N> const T& select_random_in_range(const std::array<T, N>& v) const {
		return v[this->generate<size_t>(0, v.size() - 1)];
	}
	template<typename T, size_t N> const T& select_random_in_range(const std::array<T, N>& v, size_t size) const {
		return v[this->generate<size_t>(0, size - 1)];
	}
private:
	template<typename RandType> std::vector<RandType> make_unique_rand_array_unique(const std::size_t size, RandType rand_min, RandType rand_max) const {
		if (rand_min > rand_max) std::swap(rand_min, rand_max);
		const auto max_min_diff_tmp = detail::diff(rand_max, rand_min);
		const auto max_min_diff = max_min_diff_tmp + 1;
		INVAID_ARGUMENT_THROW_WITH_MESSAGE_IF(
			max_min_diff_tmp != std::numeric_limits<decltype(max_min_diff_tmp)>::max() && max_min_diff < size,
			"random generate range(" + std::to_string(rand_min) + "-->" + std::to_string(rand_max) + ") is small to make unique rand array."
		)

		std::vector<RandType> tmp;
		auto& engine = this->get();
		distribution_t<RandType> distribution(rand_min, rand_max);

		const std::size_t make_size = (static_cast<std::uintmax_t>(std::numeric_limits<double>::max()) < size) ? ((std::numeric_limits<std::size_t>::max() - size / 5) < size) ? size : size + size / 5 : static_cast<size_t>(size*1.2);
		tmp.reserve(make_size);
		while (tmp.size() < size) {
			while (tmp.size() < make_size) tmp.push_back(distribution(engine));
			std::sort(tmp.begin(), tmp.end());
			auto unique_end = std::unique(tmp.begin(), tmp.end());

			if (size < static_cast<std::size_t>(std::distance(tmp.begin(), unique_end))) {
				unique_end = std::next(tmp.begin(), size);
			}
			tmp.erase(unique_end, tmp.end());
		}

		std::shuffle(tmp.begin(), tmp.end(), engine);
		return tmp;
	}
	template<typename RandType> std::vector<RandType> make_unique_rand_array_select(const std::size_t size, RandType rand_min, RandType rand_max) const {
		if (rand_min > rand_max) std::swap(rand_min, rand_max);
		const auto max_min_diff_tmp = detail::diff(rand_max, rand_min);
		INVAID_ARGUMENT_THROW_WITH_MESSAGE_IF(max_min_diff_tmp == std::numeric_limits<decltype(max_min_diff_tmp)>::max(), "random generate range(" + std::to_string(rand_min) + "-->" + std::to_string(rand_max) + ") is too big!")
		const auto max_min_diff = max_min_diff_tmp + 1;
		INVAID_ARGUMENT_THROW_WITH_MESSAGE_IF(max_min_diff < size, "random generate range(" + std::to_string(rand_min) + "-->" + std::to_string(rand_max) + ") is small to make unique rand array.")

		std::vector<RandType> tmp(max_min_diff);
		std::iota(tmp.begin(), tmp.end(), rand_min);

		auto& engine = this->get();
		distribution_t<RandType> distribution(rand_min, rand_max);

		for (std::size_t cnt = 0; cnt < size; ++cnt) {
			std::size_t pos = std::uniform_int_distribution<std::size_t>(cnt, tmp.size() - 1)(engine);

			if (cnt != pos) std::swap(tmp[cnt], tmp[pos]);
		}
		tmp.erase(std::next(tmp.begin(), size), tmp.end());

		return tmp;
	}
	template<typename RandType> std::vector<RandType> make_unique_rand_array_just_shuffle(const std::size_t size, RandType rand_min, RandType rand_max) const {
		if (rand_min > rand_max) std::swap(rand_min, rand_max);
		const auto max_min_diff = detail::diff(rand_max, rand_min) + 1;
		INVAID_ARGUMENT_THROW_WITH_MESSAGE_IF(max_min_diff != size, "random generate range(" + std::to_string(rand_min) + "-->" + std::to_string(rand_max) + ")'s size is not same with size.")

		auto& engine = this->get();
		distribution_t<RandType> distribution(rand_min, rand_max);
		std::vector<RandType> re(size);
		std::iota(re.begin(), re.end(), rand_min);
		std::shuffle(re.begin(), re.end(), engine);
		return re;
	}
public:
	template<typename RandType, std::enable_if_t<std::is_integral<RandType>::value/*std::is_arithmetic<RandType>::value*/, std::nullptr_t> = nullptr>
	std::vector<RandType> make_unique_rand_array(const std::size_t size, RandType rand_min = std::numeric_limits<RandType>::min(), RandType rand_max = std::numeric_limits<RandType>::max()) const {
		if (rand_min > rand_max) std::swap(rand_min, rand_max);
		if (1 == size) return{ this->generate(rand_min, rand_max) };
		const auto max_min_diff_tmp = detail::diff(rand_max, rand_min);
		if(max_min_diff_tmp == std::numeric_limits<decltype(max_min_diff_tmp)>::max()) return make_unique_rand_array_unique(size, rand_min, rand_max);
		const auto max_min_diff = max_min_diff_tmp + 1;
		INVAID_ARGUMENT_THROW_WITH_MESSAGE_IF(max_min_diff < size, "random generate range(" + std::to_string(rand_min) + "-->" + std::to_string(rand_max) + ") is small to make unique rand array.")

		if (max_min_diff == size) return make_unique_rand_array_just_shuffle(size, rand_min, rand_max);
		else if (static_cast<std::uintmax_t>(std::numeric_limits<double>::max()) < max_min_diff || size < (max_min_diff * 0.04)) {
			return make_unique_rand_array_unique(size, rand_min, rand_max);
		}
		else {
			return make_unique_rand_array_select(size, rand_min, rand_max);
		}
	}
	template<typename ForwardIte, typename RandType, std::enable_if_t<std::is_arithmetic<RandType>::value, std::nullptr_t> = nullptr>
	void generate(ForwardIte begin, ForwardIte end, RandType rand_min = std::numeric_limits<RandType>::min(), RandType rand_max = std::numeric_limits<RandType>::max()) const {
		if (rand_min > rand_max) std::swap(rand_min, rand_max);
		auto& engine = this->get();
		distribution_t<RandType> distribution(rand_min, rand_max);
		std::generate(begin, end, [&distribution, &engine]() { return distribution(engine); });
	}
	template<typename RandType, std::enable_if_t<std::is_arithmetic<RandType>::value, std::nullptr_t> = nullptr>
	std::vector<RandType> generate_n(const std::size_t size, RandType rand_min = std::numeric_limits<RandType>::min(), RandType rand_max = std::numeric_limits<RandType>::max()) const {
		if (rand_min > rand_max) std::swap(rand_min, rand_max);
		std::vector<RandType> re(size);
		auto& engine = this->get();
		distribution_t<RandType> distribution(rand_min, rand_max);
		for (auto& e : re) e = distribution(engine);
		return re;
	}
	template<std::size_t N, typename RandType, std::enable_if_t<std::is_arithmetic<RandType>::value, std::nullptr_t> = nullptr>
	std::array<RandType, N> generate_n(RandType rand_min = std::numeric_limits<RandType>::min(), RandType rand_max = std::numeric_limits<RandType>::max()) const {
		if (rand_min > rand_max) std::swap(rand_min, rand_max);
		std::array<RandType, N> re;
		auto& engine = this->get();
		distribution_t<RandType> distribution(rand_min, rand_max);
		for (auto& e : re) e = distribution(engine);
		return re;
	}

};
SharedRand make_SharedRand();
#endif //KCS_KAI_INC_RANDOM_HPP_
