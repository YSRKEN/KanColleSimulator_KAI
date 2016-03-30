#include "random.hpp"

thread_local std::mt19937 SharedRand::generator_{ std::random_device{}() };
