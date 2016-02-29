#pragma once
#include <random>
#include <memory>
#include <stdexcept>
#include <type_traits>
class missing_rand_generator : public std::runtime_error {
public:
	explicit missing_rand_generator() : std::runtime_error("missing_rand_generator") {}
	explicit missing_rand_generator(const std::string& what_arg) : std::runtime_error("missing_rand_generator : " + what_arg) {}
	explicit missing_rand_generator(const char* what_arg) : std::runtime_error(std::string("missing_rand_generator : ") + what_arg) {}
};

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
	bool is_generatable() const noexcept {
		return generator_.use_count() && generator_.get();
	}
	explicit operator bool() const noexcept {
		return this->is_generatable();
	}
	bool operator!() const noexcept {
		return this->is_generatable();
	}
	double RandReal() {
		if (this->is_generatable()) {
			return this->defalut_dist_(*this->generator_);
		}
		else {
			throw missing_rand_generator();
		}
	}
	double operator()() {
		return this->RandReal();
	}
	template<typename RandType>
	RandType generate(const RandType min, const RandType max) {
		if (this->is_generatable()) {
			return distribution_t<RandType>(min, max)(*this->generator_);
		}
		else {
			throw missing_rand_generator();
		}
	}
	template<typename RandType>
	RandType operator()(const RandType min, const RandType max) {
		return this->generate(min, max);
	}
	//[a, b]の一様整数乱数を取り出す
	int RandInt(const int min, const int max) {
		return this->generate(min, max);
	}
	//[0, n - 1]の一様整数乱数を取り出す
	int RandInt(const int n) { 
		return this->RandInt(0, n -1); 
	}
	double RandReal(const double min, const double max) {
		return this->generate(min, max);
	}
	bool RandBool(double rate = 0.5) {
		if (this->is_generatable()) {
			return std::bernoulli_distribution(rate)(*this->generator_);
		}
		else {
			throw missing_rand_generator();
		}
	}
};
