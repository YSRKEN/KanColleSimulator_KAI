#include <random>
#include "random.hpp"

XorShiftAdd& GetRandomEngine() {
	static thread_local XorShiftAdd instance{ std::random_device{}() };
	return instance;
}
