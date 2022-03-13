#include "Matrix3x3.h"

#include "Vector3.h"

#include <fmt/core.h>

const Matrix3x3 Matrix3x3::Zero(
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f
);

const Matrix3x3 Matrix3x3::One(
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f
);

const Matrix3x3 Matrix3x3::Identity(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f
);

Matrix3x3::Matrix3x3(float matrix11, float matrix12, float matrix13,
                     float matrix21, float matrix22, float matrix23,
                     float matrix31, float matrix32, float matrix33)
	: m11(matrix11), m12(matrix12), m13(matrix13)
	, m21(matrix21), m22(matrix22), m23(matrix23)
	, m31(matrix31), m32(matrix32), m33(matrix33) { }

Matrix3x3::Matrix3x3(const float matrix[9])
	: m11(matrix[0]), m12(matrix[1]), m13(matrix[2])
	, m21(matrix[3]), m22(matrix[4]), m23(matrix[5])
	, m31(matrix[6]), m32(matrix[7]), m33(matrix[8]) { }

Matrix3x3::Matrix3x3(const Matrix3x3 & matrix)
	: m11(matrix.m11), m12(matrix.m12), m13(matrix.m13)
	, m21(matrix.m21), m22(matrix.m22), m23(matrix.m23)
	, m31(matrix.m31), m32(matrix.m32), m33(matrix.m33) { }

Matrix3x3 & Matrix3x3::operator = (const Matrix3x3 & matrix) {
	memcpy(m, matrix.m, 9);

	return *this;
}

Matrix3x3::~Matrix3x3() = default;

Matrix3x3 Matrix3x3::operator + (float c)  const {
	if(c == 0.0f) { return *this; }

	return Matrix3x3(
		m11 + c, m12 + c, m13 + c,
		m21 + c, m22 + c, m23 + c,
		m31 + c, m32 + c, m33 + c
	);
}

Matrix3x3 Matrix3x3::operator + (const Matrix3x3 & matrix) const {
	return Matrix3x3(
		m11 + matrix.m11, m12 + matrix.m12, m13 + matrix.m13,
		m21 + matrix.m21, m22 + matrix.m22, m23 + matrix.m23,
		m31 + matrix.m31, m32 + matrix.m32, m33 + matrix.m33
	);
}

void Matrix3x3::operator += (float c) {
	if(c == 0.0f) { return; }

	m11 += c; m12 += c; m13 += c;
	m21 += c; m22 += c; m23 += c;
	m31 += c; m32 += c; m33 += c;
}

void Matrix3x3::operator += (const Matrix3x3 & matrix) {
	m11 += matrix.m11; m12 += matrix.m12; m13 += matrix.m13;
	m21 += matrix.m21; m22 += matrix.m22; m23 += matrix.m23;
	m31 += matrix.m31; m32 += matrix.m32; m33 += matrix.m33;
}

Matrix3x3 Matrix3x3::operator - () const {
	return Matrix3x3(
		-m11, -m12, -m13,
		-m21, -m22, -m23,
		-m31, -m32, -m33
	);
}

Matrix3x3 Matrix3x3::operator - (float c) const {
	if(c == 0.0f) { return *this; }

	return Matrix3x3(
		m11 - c, m12 - c, m13 - c,
		m21 - c, m22 - c, m23 - c,
		m31 - c, m32 - c, m33 - c
	);
}

Matrix3x3 Matrix3x3::operator - (const Matrix3x3 & matrix) const {
	return Matrix3x3(
		m11 - matrix.m11, m12 - matrix.m12, m13 - matrix.m13,
		m21 - matrix.m21, m22 - matrix.m22, m23 - matrix.m23,
		m31 - matrix.m31, m32 - matrix.m32, m33 - matrix.m33
	);
}

void Matrix3x3::operator -= (float c) {
	if(c == 0.0f) { return; }

	m11 -= c; m12 -= c; m13 -= c;
	m21 -= c; m22 -= c; m23 -= c;
	m31 -= c; m32 -= c; m33 -= c;
}

void Matrix3x3::operator -= (const Matrix3x3 & matrix) {
	m11 -= matrix.m11; m12 -= matrix.m12; m13 -= matrix.m13;
	m21 -= matrix.m21; m22 -= matrix.m22; m23 -= matrix.m23;
	m31 -= matrix.m31; m32 -= matrix.m32; m33 -= matrix.m33;
}

Matrix3x3 Matrix3x3::operator * (float c) const {
		 if(c == 0.0f) { return Zero; }
	else if(c == 1.0f) { return *this; }

	return Matrix3x3(
		m11 * c, m12 * c, m13 * c,
		m21 * c, m22 * c, m23 * c,
		m31 * c, m32 * c, m33 * c
	);
}

Matrix3x3 Matrix3x3::operator * (const Matrix3x3 & matrix) const {
	return Matrix3x3(
		m11 * matrix.m11 + m12 * matrix.m21 + m13 * matrix.m31,
		m11 * matrix.m12 + m12 * matrix.m22 + m13 * matrix.m32,
		m11 * matrix.m13 + m12 * matrix.m23 + m13 * matrix.m33,

		m21 * matrix.m11 + m22 * matrix.m21 + m23 * matrix.m31,
		m21 * matrix.m12 + m22 * matrix.m22 + m23 * matrix.m32,
		m21 * matrix.m13 + m22 * matrix.m23 + m23 * matrix.m33,

		m31 * matrix.m11 + m32 * matrix.m21 + m33 * matrix.m31,
		m31 * matrix.m12 + m32 * matrix.m22 + m33 * matrix.m32,
		m31 * matrix.m13 + m32 * matrix.m23 + m33 * matrix.m33
	);
}

void Matrix3x3::operator *= (float c) {
	if(c == 1.0f) { return; }

	m11 *= c; m12 *= c; m13 *= c;
	m21 *= c; m22 *= c; m23 *= c;
	m31 *= c; m32 *= c; m33 *= c;
}

void Matrix3x3::operator *= (const Matrix3x3 & matrix) {
	float product[9];

	product[0] = m11 * matrix.m11 + m12 * matrix.m21 + m13 * matrix.m31;
	product[1] = m11 * matrix.m12 + m12 * matrix.m22 + m13 * matrix.m32;
	product[2] = m11 * matrix.m13 + m12 * matrix.m23 + m13 * matrix.m33;

	product[3] = m21 * matrix.m11 + m22 * matrix.m21 + m23 * matrix.m31;
	product[4] = m21 * matrix.m12 + m22 * matrix.m22 + m23 * matrix.m32;
	product[5] = m21 * matrix.m13 + m22 * matrix.m23 + m23 * matrix.m33;

	product[6] = m31 * matrix.m11 + m32 * matrix.m21 + m33 * matrix.m31;
	product[7] = m31 * matrix.m12 + m32 * matrix.m22 + m33 * matrix.m32;
	product[8] = m31 * matrix.m13 + m32 * matrix.m23 + m33 * matrix.m33;

	memcpy(m, product, 9);
}

Matrix3x3 Matrix3x3::operator / (float c) const {
		 if(c == 0.0f) { return Zero; }
	else if(c == 1.0f) { return *this; }

	return Matrix3x3(
		m11 / c, m12 / c, m13 / c,
		m21 / c, m22 / c, m23 / c,
		m31 / c, m32 / c, m33 / c
	);
}

Matrix3x3 Matrix3x3::operator / (const Matrix3x3 & matrix) const {
	return *this * matrix.inverse();
}

void Matrix3x3::operator /= (float c) {
	if(c == 0.0f) {
		m11 = m12 = m13 = 0.0f;
		m21 = m22 = m23 = 0.0f;
		m31 = m32 = m33 = 0.0f;
	}
	else if(c == 1.0f) {
		return;
	}

	m11 /= c; m12 /= c; m13 /= c;
	m21 /= c; m22 /= c; m23 /= c;
	m31 /= c; m32 /= c; m33 /= c;
}

void Matrix3x3::operator /= (const Matrix3x3 & matrix) {
	return operator *= (matrix.inverse());
}

float Matrix3x3::operator [] (size_t index) const {
	if(index < 0 || index > 8) { return 0.0f; }

	return m[index];
}

bool Matrix3x3::operator == (const Matrix3x3 & matrix) const {
	return m11 == matrix.m11 && m12 == matrix.m12 && m13 == matrix.m13 &&
		   m21 == matrix.m21 && m22 == matrix.m22 && m23 == matrix.m23 &&
		   m31 == matrix.m31 && m32 == matrix.m32 && m33 == matrix.m33;
}

bool Matrix3x3::operator != (const Matrix3x3 & matrix) const {
	return !operator == (matrix);
}

void Matrix3x3::setMatrix(float matrix11, float matrix12, float matrix13,
                          float matrix21, float matrix22, float matrix23,
                          float matrix31, float matrix32, float matrix33) {
	m11 = matrix11; m12 = matrix12; m13 = matrix13;
	m21 = matrix21; m22 = matrix22; m23 = matrix23;
	m31 = matrix31; m32 = matrix32; m33 = matrix33;
}

void Matrix3x3::setMatrix(const float matrix[9]) {
	memcpy(m, matrix, 9);
}

void Matrix3x3::setMatrix(const Matrix3x3 & matrix) {
	memcpy(m, matrix.m, 9);
}

float Matrix3x3::determinant() const {
	return m11 * (m22 * m33 - m32 * m23) -
		   m12 * (m21 * m33 - m23 * m31) +
		   m13 * (m21 * m32 - m22 * m31);
}

void Matrix3x3::transpose() {
	float temp;

	temp = m12;
	m12  = m21;
	m21  = temp;

	temp = m13;
	m13  = m31;
	m31  = m13;

	temp = m23;
	m23  = m32;
	m32  = m23;
}

Matrix3x3 Matrix3x3::transposed() const {
	return Matrix3x3(m11, m21, m31,
					 m12, m22, m32,
					 m13, m23, m33);
}

void Matrix3x3::invert() {
	float determinant = m11 * (m22 * m33 - m32 * m23) -
						m12 * (m21 * m33 - m23 * m31) +
						m13 * (m21 * m32 - m22 * m31);

	if(determinant == 0.0f) {
		m11 = m12 = m13 = 0.0f;
		m21 = m22 = m23 = 0.0f;
		m31 = m32 = m33 = 0.0f;
	}
	else {
		float inverse[9];

		float inverseDeterminant = 1.0f / determinant;

		inverse[0] = (m22 * m33 - m32 * m23) * inverseDeterminant;
		inverse[1] = (m13 * m32 - m12 * m33) * inverseDeterminant;
		inverse[2] = (m12 * m23 - m13 * m22) * inverseDeterminant;
		inverse[3] = (m23 * m31 - m21 * m33) * inverseDeterminant;
		inverse[4] = (m11 * m33 - m13 * m31) * inverseDeterminant;
		inverse[5] = (m21 * m13 - m11 * m23) * inverseDeterminant;
		inverse[6] = (m21 * m32 - m31 * m22) * inverseDeterminant;
		inverse[7] = (m31 * m12 - m11 * m32) * inverseDeterminant;
		inverse[8] = (m11 * m22 - m21 * m12) * inverseDeterminant;

		m11 = inverse[0]; m12 = inverse[1]; m13 = inverse[2];
		m21 = inverse[3]; m22 = inverse[4]; m23 = inverse[5];
		m31 = inverse[6]; m32 = inverse[7]; m33 = inverse[8];
	}
}

Matrix3x3 Matrix3x3::inverse() const {
	float determinant = m11 * (m22 * m33 - m32 * m23) -
						m12 * (m21 * m33 - m23 * m31) +
						m13 * (m21 * m32 - m22 * m31);

	if(determinant == 0.0f) {
		return Zero;
	}
	else {
		float inverseDeterminant = 1.0f / determinant;

		return Matrix3x3(
			(m22 * m33 - m32 * m23) * inverseDeterminant,
			(m13 * m32 - m12 * m33) * inverseDeterminant,
			(m12 * m23 - m13 * m22) * inverseDeterminant,
			(m23 * m31 - m21 * m33) * inverseDeterminant,
			(m11 * m33 - m13 * m31) * inverseDeterminant,
			(m21 * m13 - m11 * m23) * inverseDeterminant,
			(m21 * m32 - m31 * m22) * inverseDeterminant,
			(m31 * m12 - m11 * m32) * inverseDeterminant,
			(m11 * m22 - m21 * m12) * inverseDeterminant
		);
	}
}

std::string Matrix3x3::toString() const {
	return fmt::format("{}, {}, {}, {}, {}, {}, {}, {}, {}", m11, m12, m13, m21, m22, m23, m31, m32, m33);
}
