#include "Ease.h"
#include <algorithm>

float EaseOut(float start, float end, float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	t = 1.0f - (1.0f - t) * (1.0f - t); // EaseOutQuad
	return start + (end - start) * t;
}