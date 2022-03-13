#ifndef _MATRIX2X2_H_
#define _MATRIX2X2_H_

#include <string>

class Vector2;

class Matrix2x2 final {
public:
	Matrix2x2              (float matrix11 = 1.0f, float matrix12 = 0.0f,
                            float matrix21 = 0.0f, float matrix22 = 1.0f);
	Matrix2x2              (const float matrix[4]);
	Matrix2x2              (const Matrix2x2 & matrix);
	Matrix2x2 & operator = (const Matrix2x2 & matrix);
	~Matrix2x2             ();

	Matrix2x2 operator   + (float c)                  const;
	Matrix2x2 operator   + (const Matrix2x2 & matrix) const;

	void operator       += (float c);
	void operator       += (const Matrix2x2 & matrix);

	Matrix2x2 operator   - ()                         const;

	Matrix2x2 operator   - (float c)                  const;
	Matrix2x2 operator   - (const Matrix2x2 & matrix) const;

	void operator       -= (float c);
	void operator       -= (const Matrix2x2 & matrix);

	Matrix2x2 operator   * (float c)                  const;
	Matrix2x2 operator   * (const Matrix2x2 & matrix) const;

	void operator       *= (float c);
	void operator       *= (const Matrix2x2 & matrix);

	Matrix2x2 operator   / (float c)                  const;
	Matrix2x2 operator   / (const Matrix2x2 & matrix) const;

	void operator       /= (float c);
	void operator       /= (const Matrix2x2 & matrix);

	float operator      [] (size_t index)             const;

	bool operator       == (const Matrix2x2 & matrix) const;
	bool operator       != (const Matrix2x2 & matrix) const;

	void setMatrix(float matrix11, float matrix12,
                   float matrix21, float matrix22);
	void setMatrix(const float matrix[4]);
	void setMatrix(const Matrix2x2 & matrix);

	void rotate(float degrees);
	Matrix2x2 rotated(float degrees) const;
	static Matrix2x2 createRotation(float degrees);

	void scale(float amount);
	void scale(float x, float y);
	void scale(const Vector2 & amount);
	Matrix2x2 scaled(float amount) const;
	Matrix2x2 scaled(float x, float y) const;
	Matrix2x2 scaled(const Vector2 & amount) const;
	static Matrix2x2 createScale(float amount);
	static Matrix2x2 createScale(float x, float y);
	static Matrix2x2 createScale(const Vector2 & amount);

	float determinant() const;

	void transpose();
	Matrix2x2 transposed() const;

	void invert();
	Matrix2x2 inverse() const;

	std::string toString() const;

	static const Matrix2x2 Zero;
	static const Matrix2x2 One;
	static const Matrix2x2 Identity;

	union {
		struct {
			float m11, m12,
				  m21, m22;
		};

		float m[4];
	};
};

#endif // _MATRIX2X2_H_
