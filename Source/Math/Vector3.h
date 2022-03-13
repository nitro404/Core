#ifndef _VECTOR3_H_
#define _VECTOR3_H_

#include <string>

class Vector2;
class Vector4;

class Vector3 final {
public:
	Vector3              (float a = 0.0f, float b = 0.0f, float c = 0.0f);
	Vector3              (const float v[3]);
	Vector3              (const Vector2 & v);
	Vector3              (const Vector3 & v);
	Vector3              (const Vector4 & v);
	Vector3 & operator = (const Vector2 & v);
	Vector3 & operator = (const Vector3 & v);
	Vector3 & operator = (const Vector4 & v);
	~Vector3             ();

	Vector3 operator   + (float c)           const;
	Vector3 operator   + (const Vector2 & v) const;
	Vector3 operator   + (const Vector3 & v) const;
	Vector3 operator   + (const Vector4 & v) const;

	void operator     += (float c);
	void operator     += (const Vector2 & v);
	void operator     += (const Vector3 & v);
	void operator     += (const Vector4 & v);

	Vector3 operator   - ()                  const;

	Vector3 operator   - (float c)           const;
	Vector3 operator   - (const Vector2 & v) const;
	Vector3 operator   - (const Vector3 & v) const;
	Vector3 operator   - (const Vector4 & v) const;

	void operator     -= (float c);
	void operator     -= (const Vector2 & v);
	void operator     -= (const Vector3 & v);
	void operator     -= (const Vector4 & v);

	Vector3 operator   * (float c)           const;
	Vector3 operator   * (const Vector2 & v) const;
	Vector3 operator   * (const Vector3 & v) const;
	Vector3 operator   * (const Vector4 & v) const;

	void operator     *= (float c);
	void operator     *= (const Vector2 & v);
	void operator     *= (const Vector3 & v);
	void operator     *= (const Vector4 & v);

	Vector3 operator   / (float c)           const;
	Vector3 operator   / (const Vector2 & v) const;
	Vector3 operator   / (const Vector3 & v) const;
	Vector3 operator   / (const Vector4 & v) const;

	void operator     /= (float c);
	void operator     /= (const Vector2 & v);
	void operator     /= (const Vector3 & v);
	void operator     /= (const Vector4 & v);

	float operator    [] (size_t index)      const;

	bool operator     == (const Vector3 & v) const;
	bool operator     != (const Vector3 & v) const;

	void setVector       (float a, float b, float c);
	void setVector       (const float vector[3]);
	void setVector       (const Vector2 & vector);
	void setVector       (const Vector3 & vector);
	void setVector       (const Vector4 & vector);

	float length         ()                  const;
	float dot            (const Vector3 & v) const;
	Vector3 cross        (const Vector3 & v) const;
	void normalize       ();
	Vector3 normalized   ()                  const;

	std::string toString() const;

	static const Vector3 Zero;
	static const Vector3 One;
	static const Vector3 Forward;
	static const Vector3 Backward;
	static const Vector3 Left;
	static const Vector3 Right;
	static const Vector3 Up;
	static const Vector3 Down;

	union {
		struct {
			float x, y, z;
		};

		float v[3];
	};
};

#endif // _VECTOR3_H_
