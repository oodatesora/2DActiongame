#include "Random.h"

std::mt19937 Random::engine_;

void Random::Initialize() {
	std::random_device seed;
	engine_.seed(seed());
}

float Random::GetFloat(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(engine_);
}

int Random::GetInt(int min, int max) {
	std::uniform_int_distribution<int> dist(min, max);
	return dist(engine_);
}