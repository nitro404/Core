#ifndef _VECTOR4_H_
#define _VECTOR4_H_

#include <string>

class Vector2;
class Vector3;

class Vector4 final {
public:
	Vector4              (float a = 0.0f, float b = 0.0f, float c = 0.0f, float d = 0.0f);
	Vector4              (const float v[4]);
	Vector4              (const Vector2 & v);
	Vector4              (const Vector3 & v);
	Vector4              (const Vector4 & v);
	Vector4 & operator = (const Vector2 & v);
	Vector4 & operator = (const Vector3 & v);
	Vector4 & operator = (const Vector4 & v);
	~Vector4             ();

	Vector4 operator   + (float c)           const;
	Vector4 operator   + (const Vector2 & v) const;
	Vector4 operator   + (const Vector3 & v) const;
	Vector4 operator   + (const Vector4 & v) const;

	void operator     += (float c);
	void operator     += (const Vector2 & v);
	void operator     += (const Vector3 & v);
	void operator     += (const Vector4 & v);

	Vector4 operator   - ()                  const;

	Vector4 operator   - (float c)           const;
	Vector4 operator   - (const Vector2 & v) const;
	Vector4 operator   - (const Vector3 & v) const;
	Vector4 operator   - (const Vector4 & v) const;

	void operator     -= (float c);
	void operator     -= (const Vector2 & v);
	void operator     -= (const Vector3 & v);
	void operator     -= (const Vector4 & v);

	Vector4 operator   * (float c)           const;
	Vector4 operator   * (const Vector2 & v) const;
	Vector4 operator   * (const Vector3 & v) const;
	Vector4 operator   * (const Vector4 & v) const;

	void operator     *= (float c);
	void operator     *= (const Vector2 & v);
	void operator     *= (const Vector3 & v);
	void operator     *= (const Vector4 & v);

	Vector4 operator   / (float c)           const;
	Vector4 operator   / (const Vector2 & v) const;
	Vector4 operator   / (const Vector3 & v) const;
	Vector4 operator   / (const Vector4 & v) const;

	void operator     /= (float c);
	void operator     /= (const Vector2 & v);
	void operator     /= (const Vector3 & v);
	void operator     /= (const Vector4 & v);

	float operator    [] (size_t index)      const;

	bool operator     == (const Vector4 & v) const;
	bool operator     != (const Vector4 & v) const;

	void setVector       (float a, float b, float c, float d);
	void setVector       (const float vector[4]);
	void setVector       (const Vector2 & vector);
	void setVector       (const Vector3 & vector);
	void setVector       (const Vector4 & vector);

	float length         ()                  const;
	float dot            (const Vector4 & v) const;
	void normalize       ();
	Vector4 normalized   ()                  const;

	std::string toString() const;

	static const Vector4 Zero;
	static const Vector4 One;
	static const Vector4 Forward;
	static const Vector4 Backward;
	static const Vector4 Left;
	static const Vector4 Right;
	static const Vector4 Up;
	static const Vector4 Down;

	union {
		struct {
			float x, y, z, w;
		};

		float v[4];
	};
};

#endif // _VECTOR4_H_
