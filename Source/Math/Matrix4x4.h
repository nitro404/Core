#ifndef _MATRIX4X4_H_
#define _MATRIX4X4_H_

#include <string>

class Matrix4x4 {
public:
	Matrix4x4              (float matrix11 = 1.0f, float matrix12 = 0.0f, float matrix13 = 0.0f, float matrix14 = 0.0f,
                            float matrix21 = 0.0f, float matrix22 = 1.0f, float matrix23 = 0.0f, float matrix24 = 0.0f,
                            float matrix31 = 0.0f, float matrix32 = 0.0f, float matrix33 = 1.0f, float matrix34 = 0.0f,
                            float matrix41 = 0.0f, float matrix42 = 0.0f, float matrix43 = 0.0f, float matrix44 = 1.0f);
	Matrix4x4              (const float matrix[16]);
	Matrix4x4              (const Matrix4x4 & matrix);
	Matrix4x4 & operator = (const Matrix4x4 & matrix);
	virtual ~Matrix4x4     ();

	Matrix4x4 operator   + (float c)                  const;
	Matrix4x4 operator   + (const Matrix4x4 & matrix) const;

	void operator       += (float c);
	void operator       += (const Matrix4x4 & matrix);

	Matrix4x4 operator   - ()                         const;

	Matrix4x4 operator   - (float c)                  const;
	Matrix4x4 operator   - (const Matrix4x4 & matrix) const;

	void operator       -= (float c);
	void operator       -= (const Matrix4x4 & matrix);

	Matrix4x4 operator   * (float c)                  const;
	Matrix4x4 operator   * (const Matrix4x4 & matrix) const;

	void operator       *= (float c);
	void operator       *= (const Matrix4x4 & matrix);

	Matrix4x4 operator   / (float c)                  const;
	Matrix4x4 operator   / (const Matrix4x4 & matrix) const;

	void operator       /= (float c);
	void operator       /= (const Matrix4x4 & matrix);

	float operator      [] (size_t index)             const;

	bool operator       == (const Matrix4x4 & matrix) const;
	bool operator       != (const Matrix4x4 & matrix) const;

	void setMatrix(float matrix11, float matrix12, float matrix13, float matrix14,
                   float matrix21, float matrix22, float matrix23, float matrix24,
                   float matrix31, float matrix32, float matrix33, float matrix34,
                   float matrix41, float matrix42, float matrix43, float matrix44);
	void setMatrix(const float matrix[16]);
	void setMatrix(const Matrix4x4 & matrix);

	float determinant() const;

	void transpose();
	Matrix4x4 transposed() const;

	void invert();
	Matrix4x4 inverse() const;

	std::string toString() const;

	static Matrix4x4 parseFrom(const std::string & data, bool * error = nullptr);

	static const Matrix4x4 Zero;
	static const Matrix4x4 One;
	static const Matrix4x4 Identity;

	union {
		struct {
			float m11, m12, m13, m14,
				  m21, m22, m23, m24,
				  m31, m32, m33, m34,
				  m41, m42, m43, m44;
		};

		float m[16];
	};
};

#endif // _MATRIX4X4_H_
