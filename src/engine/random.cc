#include "random.h"

RandGenerator::RandGenerator() {
	eng = std::default_random_engine(rd());
}

void RandGenerator::setrange(float min, float max) {
	float_distr = std::uniform_real_distribution<float>(min, max);
}

void RandGenerator::setrange(int min, int max) {
	int_distr = std::uniform_int_distribution<int>(min, max);
}

int RandGenerator::randint() {
	return int_distr(eng);
}

float RandGenerator::randfloat() {
	return float_distr(eng);
}
