#ifndef _VECTOR2_H_
#define _VECTOR2_H_

#include <string>

class Vector3;
class Vector4;

class Vector2 final {
public:
	Vector2              (float a = 0.0f, float b = 0.0f);
	Vector2              (const float v[2]);
	Vector2              (const Vector2 & v);
	Vector2              (const Vector3 & v);
	Vector2              (const Vector4 & v);
	Vector2 & operator = (const Vector2 & v);
	Vector2 & operator = (const Vector3 & v);
	Vector2 & operator = (const Vector4 & v);
	~Vector2             ();

	Vector2 operator   + (float c)           const;
	Vector2 operator   + (const Vector2 & v) const;
	Vector2 operator   + (const Vector3 & v) const;
	Vector2 operator   + (const Vector4 & v) const;

	void operator     += (float c);
	void operator     += (const Vector2 & v);
	void operator     += (const Vector3 & v);
	void operator     += (const Vector4 & v);

	Vector2 operator   - ()                  const;

	Vector2 operator   - (float c)           const;
	Vector2 operator   - (const Vector2 & v) const;
	Vector2 operator   - (const Vector3 & v) const;
	Vector2 operator   - (const Vector4 & v) const;

	void operator     -= (float c);
	void operator     -= (const Vector2 & v);
	void operator     -= (const Vector3 & v);
	void operator     -= (const Vector4 & v);

	Vector2 operator   * (float c)           const;
	Vector2 operator   * (const Vector2 & v) const;
	Vector2 operator   * (const Vector3 & v) const;
	Vector2 operator   * (const Vector4 & v) const;

	void operator     *= (float c);
	void operator     *= (const Vector2 & v);
	void operator     *= (const Vector3 & v);
	void operator     *= (const Vector4 & v);

	Vector2 operator   / (float c)           const;
	Vector2 operator   / (const Vector2 & v) const;
	Vector2 operator   / (const Vector3 & v) const;
	Vector2 operator   / (const Vector4 & v) const;

	void operator     /= (float c);
	void operator     /= (const Vector2 & v);
	void operator     /= (const Vector3 & v);
	void operator     /= (const Vector4 & v);

	float operator    [] (size_t index)      const;

	bool operator     == (const Vector2 & v) const;
	bool operator     != (const Vector2 & v) const;

	void setVector       (float a, float b);
	void setVector       (const float vector[2]);
	void setVector       (const Vector2 & vector);
	void setVector       (const Vector3 & vector);
	void setVector       (const Vector4 & vector);

	float length         ()                  const;
	float dot            (const Vector2 & v) const;
	void normalize       ();
	Vector2 normalized   ()                  const;

	std::string toString() const;

	static const Vector2 Zero;
	static const Vector2 One;
	static const Vector2 Left;
	static const Vector2 Right;
	static const Vector2 Up;
	static const Vector2 Down;

	union {
		struct {
			float x, y;
		};

		float v[2];
	};
};

#endif // _VECTOR2_H_
