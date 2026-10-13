#pragma once
#include <random>

class Random {
public:
	static void Initialize();

	static float GetFloat(float min, float max);

	static int GetInt(int min, int max);

private:
	static std::mt19937 engine_;
};
