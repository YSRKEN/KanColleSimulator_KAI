#include <random>
#include "random.hpp"

XorShiftPlus& GetRandomEngine() {
	static thread_local XorShiftPlus instance{ std::random_device{}() };
	return instance;
}
