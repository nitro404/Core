#ifndef _EXTENDED_MATH_H_
#define _EXTENDED_MATH_H_

namespace Math {

	extern const float PI;
	extern const float HalfPI;
	extern const float QuarterPI;
	extern const float TwoPI;
	extern const float E;
	extern const float Log10E;
	extern const float Log2E;

	float min(float a, float b);
	float max(float a, float b);
	float clamp(float value, float min, float max);
	float abs(float v);
	float distance(float a, float b);
	float radiansToDegrees(float r);
	float degreesToRadians(float d);
	int compareAngles(float a, float b);
	float lerp(float a, float b, float amount);
	float normalize(float value, float min, float max);

}

#endif // _EXTENDED_MATH_H_
