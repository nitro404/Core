#include "Math/ExtendedMath.h"

#include <cmath>

const float Math::PI        = 3.14159265358979323846f;
const float Math::HalfPI    = 1.57079632679489661923f;
const float Math::QuarterPI = 0.78539816339744830962f;
const float Math::TwoPI     = 6.28318530717958647693f;
const float Math::E         = 2.71828182845904523536f;
const float Math::Log10E    = 6.25907521676639521102f;
const float Math::Log2E     = 1.8841693853637201099f;

float Math::min(float a, float b) {
	return a < b ? a : b;
}

float Math::max(float a, float b) {
	return a > b ? a : b;
}

float Math::clamp(float value, float min, float max) {
	return value < min ? min : value > max ? max : value;
}

float Math::abs(float v) {
	return v < 0.0f ? 0.0f - v : v;
}

float Math::distance(float a, float b) {
	return abs(b - a);
}

float Math::radiansToDegrees(float r) {
	return r * (180.0f / PI);
}

float Math::degreesToRadians(float d) {
	return d * (PI / 180.0f);
}

int Math::compareAngles(float a, float b) {
	if(a == b) { return 0; }

	float c = fmod(a, 360.0f);
	float d = fmod(b, 360.0f);

	if(c < 0.0f) { c += 360.0f; }
	if(d < 0.0f) { d += 360.0f; }
	if(c == d) { return 0; }

	double angleComparison = cos(degreesToRadians(a - b) + (PI / 2.0f));

	return angleComparison < 0.0 ? -1 : angleComparison > 0.0 ? 1 : 0;
}

float Math::lerp(float a, float b, float amount) {
		 if(amount == 0.0f) { return a; }
	else if(amount == 1.0f) { return b; }

	return a + (b - a) * amount;
}

float Math::normalize(float value, float min, float max) {
	return (value - min) / (max - min);
}
