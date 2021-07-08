#ifndef _MATRIX3X3_H_
#define _MATRIX3X3_H_

#include <string>

class Matrix3x3 {
public:
	Matrix3x3              (float matrix11 = 1.0f, float matrix12 = 0.0f, float matrix13 = 0.0f,
                            float matrix21 = 0.0f, float matrix22 = 1.0f, float matrix23 = 0.0f,
                            float matrix31 = 0.0f, float matrix32 = 0.0f, float matrix33 = 1.0f);
	Matrix3x3              (const float matrix[9]);
	Matrix3x3              (const Matrix3x3 & matrix);
	Matrix3x3 & operator = (const Matrix3x3 & matrix);
	virtual ~Matrix3x3     ();

	Matrix3x3 operator   + (float c)                  const;
	Matrix3x3 operator   + (const Matrix3x3 & matrix) const;

	void operator       += (float c);
	void operator       += (const Matrix3x3 & matrix);

	Matrix3x3 operator   - ()                         const;

	Matrix3x3 operator   - (float c)                  const;
	Matrix3x3 operator   - (const Matrix3x3 & matrix) const;

	void operator       -= (float c);
	void operator       -= (const Matrix3x3 & matrix);

	Matrix3x3 operator   * (float c)                  const;
	Matrix3x3 operator   * (const Matrix3x3 & matrix) const;

	void operator       *= (float c);
	void operator       *= (const Matrix3x3 & matrix);

	Matrix3x3 operator   / (float c)                  const;
	Matrix3x3 operator   / (const Matrix3x3 & matrix) const;

	void operator       /= (float c);
	void operator       /= (const Matrix3x3 & matrix);

	float operator      [] (size_t index)             const;

	bool operator       == (const Matrix3x3 & matrix) const;
	bool operator       != (const Matrix3x3 & matrix) const;

	void setMatrix(float matrix11, float matrix12, float matrix13,
                   float matrix21, float matrix22, float matrix23,
                   float matrix31, float matrix32, float matrix33);
	void setMatrix(const float matrix[9]);
	void setMatrix(const Matrix3x3 & matrix);

	float determinant() const;

	void transpose();
	Matrix3x3 transposed() const;

	void invert();
	Matrix3x3 inverse() const;

	std::string toString() const;

	static Matrix3x3 parseFrom(const std::string & data, bool * error = nullptr);

	static const Matrix3x3 Zero;
	static const Matrix3x3 One;
	static const Matrix3x3 Identity;

	union {
		struct {
			float m11, m12, m13,
				  m21, m22, m23,
				  m31, m32, m33;
		};

		float m[9];
	};
};

#endif // _MATRIX3X3_H_
