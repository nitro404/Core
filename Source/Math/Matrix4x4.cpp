#include "Matrix4x4.h"

#include "Utilities/StringUtilities.h"
#include "Vector3.h"
#include "Vector4.h"

#include <regex>

const Matrix4x4 Matrix4x4::Zero(
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
);

const Matrix4x4 Matrix4x4::One(
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f
);

const Matrix4x4 Matrix4x4::Identity(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);

Matrix4x4::Matrix4x4(float matrix11, float matrix12, float matrix13, float matrix14,
                     float matrix21, float matrix22, float matrix23, float matrix24,
                     float matrix31, float matrix32, float matrix33, float matrix34,
                     float matrix41, float matrix42, float matrix43, float matrix44)
	: m11(matrix11), m12(matrix12), m13(matrix13), m14(matrix14)
	, m21(matrix21), m22(matrix22), m23(matrix23), m24(matrix24)
	, m31(matrix31), m32(matrix32), m33(matrix33), m34(matrix34)
	, m41(matrix41), m42(matrix42), m43(matrix43), m44(matrix44) { }

Matrix4x4::Matrix4x4(const float matrix[16])
	: m11(matrix[0]),  m12(matrix[1]),  m13(matrix[2]),  m14(matrix[3])
	, m21(matrix[4]),  m22(matrix[5]),  m23(matrix[6]),  m24(matrix[7])
	, m31(matrix[8]),  m32(matrix[9]),  m33(matrix[10]), m34(matrix[11])
	, m41(matrix[12]), m42(matrix[13]), m43(matrix[14]), m44(matrix[15]) { }

Matrix4x4::Matrix4x4(const Matrix4x4 & matrix)
	: m11(matrix.m11), m12(matrix.m12), m13(matrix.m13), m14(matrix.m14)
	, m21(matrix.m21), m22(matrix.m22), m23(matrix.m23), m24(matrix.m24)
	, m31(matrix.m31), m32(matrix.m32), m33(matrix.m33), m34(matrix.m34)
	, m41(matrix.m41), m42(matrix.m42), m43(matrix.m43), m44(matrix.m44) { }

Matrix4x4 & Matrix4x4::operator = (const Matrix4x4 & matrix) {
	memcpy(m, matrix.m, 16);

	return *this;
}

Matrix4x4::~Matrix4x4() = default;

Matrix4x4 Matrix4x4::operator + (float c)  const {
	if(c == 0.0f) { return *this; }

	return Matrix4x4(
		m11 + c, m12 + c, m13 + c, m14 + c,
		m21 + c, m22 + c, m23 + c, m24 + c,
		m31 + c, m32 + c, m33 + c, m34 + c,
		m41 + c, m42 + c, m43 + c, m44 + c
	);
}

Matrix4x4 Matrix4x4::operator + (const Matrix4x4 & matrix) const {
	return Matrix4x4(
		m11 + matrix.m11, m12 + matrix.m12, m13 + matrix.m13, m14 + matrix.m14,
		m21 + matrix.m21, m22 + matrix.m22, m23 + matrix.m23, m24 + matrix.m24,
		m31 + matrix.m31, m32 + matrix.m32, m33 + matrix.m33, m34 + matrix.m34,
		m41 + matrix.m41, m42 + matrix.m42, m43 + matrix.m43, m44 + matrix.m44
	);
}

void Matrix4x4::operator += (float c) {
	if(c == 0.0f) { return; }

	m11 += c; m12 += c; m13 += c; m14 += c;
	m21 += c; m22 += c; m23 += c; m24 += c;
	m31 += c; m32 += c; m33 += c; m34 += c;
	m41 += c; m42 += c; m43 += c; m44 += c;
}

void Matrix4x4::operator += (const Matrix4x4 & matrix) {
	m11 += matrix.m11; m12 += matrix.m12; m13 += matrix.m13; m14 += matrix.m14;
	m21 += matrix.m21; m22 += matrix.m22; m23 += matrix.m23; m24 += matrix.m24;
	m31 += matrix.m31; m32 += matrix.m32; m33 += matrix.m33; m34 += matrix.m34;
	m41 += matrix.m41; m42 += matrix.m42; m43 += matrix.m43; m44 += matrix.m44;
}

Matrix4x4 Matrix4x4::operator - () const {
	return Matrix4x4(
		-m11, -m12, -m13, -m14,
		-m21, -m22, -m23, -m24,
		-m31, -m32, -m33, -m34,
		-m41, -m42, -m43, -m44
	);
}

Matrix4x4 Matrix4x4::operator - (float c) const {
	if(c == 0.0f) { return *this; }

	return Matrix4x4(
		m11 - c, m12 - c, m13 - c, m14 - c,
		m21 - c, m22 - c, m23 - c, m24 - c,
		m31 - c, m32 - c, m33 - c, m34 - c,
		m41 - c, m42 - c, m43 - c, m44 - c
	);
}

Matrix4x4 Matrix4x4::operator - (const Matrix4x4 & matrix) const {
	return Matrix4x4(
		m11 - matrix.m11, m12 - matrix.m12, m13 - matrix.m13, m14 - matrix.m14,
		m21 - matrix.m21, m22 - matrix.m22, m23 - matrix.m23, m24 - matrix.m24,
		m31 - matrix.m31, m32 - matrix.m32, m33 - matrix.m33, m34 - matrix.m34,
		m41 - matrix.m41, m42 - matrix.m42, m43 - matrix.m43, m44 - matrix.m44
	);
}

void Matrix4x4::operator -= (float c) {
	if(c == 0.0f) { return; }

	m11 -= c; m12 -= c; m13 -= c; m14 -= c;
	m21 -= c; m22 -= c; m23 -= c; m24 -= c;
	m31 -= c; m32 -= c; m33 -= c; m34 -= c;
	m41 -= c; m42 -= c; m43 -= c; m44 -= c;
}

void Matrix4x4::operator -= (const Matrix4x4 & matrix) {
	m11 -= matrix.m11; m12 -= matrix.m12; m13 -= matrix.m13; m14 -= matrix.m14;
	m21 -= matrix.m21; m22 -= matrix.m22; m23 -= matrix.m23; m24 -= matrix.m24;
	m31 -= matrix.m31; m32 -= matrix.m32; m33 -= matrix.m33; m34 -= matrix.m34;
	m41 -= matrix.m41; m42 -= matrix.m42; m43 -= matrix.m43; m44 -= matrix.m44;
}

Matrix4x4 Matrix4x4::operator * (float c) const {
		 if(c == 0.0f) { return Zero; }
	else if(c == 1.0f) { return *this; }

	return Matrix4x4(
		m11 * c, m12 * c, m13 * c, m14 * c,
		m21 * c, m22 * c, m23 * c, m24 * c,
		m31 * c, m32 * c, m33 * c, m34 * c,
		m41 * c, m42 * c, m43 * c, m44 * c
	);
}

Matrix4x4 Matrix4x4::operator * (const Matrix4x4 & matrix) const {
	return Matrix4x4(
		m11 * matrix.m11 + m12 * matrix.m21 + m13 * matrix.m31 + m14 * matrix.m41,
		m11 * matrix.m12 + m12 * matrix.m22 + m13 * matrix.m32 + m14 * matrix.m42,
		m11 * matrix.m13 + m12 * matrix.m23 + m13 * matrix.m33 + m14 * matrix.m43,
		m11 * matrix.m14 + m12 * matrix.m24 + m13 * matrix.m34 + m14 * matrix.m44,

		m21 * matrix.m11 + m22 * matrix.m21 + m23 * matrix.m31 + m24 * matrix.m41,
		m21 * matrix.m12 + m22 * matrix.m22 + m23 * matrix.m32 + m24 * matrix.m42,
		m21 * matrix.m13 + m22 * matrix.m23 + m23 * matrix.m33 + m24 * matrix.m43,
		m21 * matrix.m14 + m22 * matrix.m24 + m23 * matrix.m34 + m24 * matrix.m44,

		m31 * matrix.m11 + m32 * matrix.m21 + m33 * matrix.m31 + m34 * matrix.m41,
		m31 * matrix.m12 + m32 * matrix.m22 + m33 * matrix.m32 + m34 * matrix.m42,
		m31 * matrix.m13 + m32 * matrix.m23 + m33 * matrix.m33 + m34 * matrix.m43,
		m31 * matrix.m14 + m32 * matrix.m24 + m33 * matrix.m34 + m34 * matrix.m44,

		m41 * matrix.m11 + m42 * matrix.m21 + m43 * matrix.m31 + m44 * matrix.m41,
		m41 * matrix.m12 + m42 * matrix.m22 + m43 * matrix.m32 + m44 * matrix.m42,
		m41 * matrix.m13 + m42 * matrix.m23 + m43 * matrix.m33 + m44 * matrix.m43,
		m41 * matrix.m14 + m42 * matrix.m24 + m43 * matrix.m34 + m44 * matrix.m44
	);
}

void Matrix4x4::operator *= (float c) {
	if(c == 1.0f) { return; }

	m11 *= c; m12 *= c; m13 *= c; m14 *= c;
	m21 *= c; m22 *= c; m23 *= c; m24 *= c;
	m31 *= c; m32 *= c; m33 *= c; m34 *= c;
	m41 *= c; m42 *= c; m43 *= c; m44 *= c;
}

void Matrix4x4::operator *= (const Matrix4x4 & matrix) {
	float product[16];

	product[0]  = m11 * matrix.m11 + m12 * matrix.m21 + m13 * matrix.m31 + m14 * matrix.m41;
	product[1]  = m11 * matrix.m12 + m12 * matrix.m22 + m13 * matrix.m32 + m14 * matrix.m42;
	product[2]  = m11 * matrix.m13 + m12 * matrix.m23 + m13 * matrix.m33 + m14 * matrix.m43;
	product[3]  = m11 * matrix.m14 + m12 * matrix.m24 + m13 * matrix.m34 + m14 * matrix.m44;

	product[4]  = m21 * matrix.m11 + m22 * matrix.m21 + m23 * matrix.m31 + m24 * matrix.m41;
	product[5]  = m21 * matrix.m12 + m22 * matrix.m22 + m23 * matrix.m32 + m24 * matrix.m42;
	product[6]  = m21 * matrix.m13 + m22 * matrix.m23 + m23 * matrix.m33 + m24 * matrix.m43;
	product[7]  = m21 * matrix.m14 + m22 * matrix.m24 + m23 * matrix.m34 + m24 * matrix.m44;

	product[8]  = m31 * matrix.m11 + m32 * matrix.m21 + m33 * matrix.m31 + m34 * matrix.m41;
	product[9]  = m31 * matrix.m12 + m32 * matrix.m22 + m33 * matrix.m32 + m34 * matrix.m42;
	product[10] = m31 * matrix.m13 + m32 * matrix.m23 + m33 * matrix.m33 + m34 * matrix.m43;
	product[11] = m31 * matrix.m14 + m32 * matrix.m24 + m33 * matrix.m34 + m34 * matrix.m44;

	product[12] = m41 * matrix.m11 + m42 * matrix.m21 + m43 * matrix.m31 + m44 * matrix.m41;
	product[13] = m41 * matrix.m12 + m42 * matrix.m22 + m43 * matrix.m32 + m44 * matrix.m42;
	product[14] = m41 * matrix.m13 + m42 * matrix.m23 + m43 * matrix.m33 + m44 * matrix.m43;
	product[15] = m41 * matrix.m14 + m42 * matrix.m24 + m43 * matrix.m34 + m44 * matrix.m44;

	memcpy(m, product, 16);
}

Matrix4x4 Matrix4x4::operator / (float c) const {
		 if(c == 0.0f) { return Zero; }
	else if(c == 1.0f) { return *this; }

	return Matrix4x4(
		m11 / c, m12 / c, m13 / c, m14 / c,
		m21 / c, m22 / c, m23 / c, m24 / c,
		m31 / c, m32 / c, m33 / c, m34 / c,
		m41 / c, m42 / c, m43 / c, m44 / c
	);
}

Matrix4x4 Matrix4x4::operator / (const Matrix4x4 & matrix) const {
	return *this * matrix.inverse();
}

void Matrix4x4::operator /= (float c) {
	if(c == 0.0f) {
		m11 = m12 = m13 = m14 = 0.0f;
		m21 = m22 = m23 = m44 = 0.0f;
		m31 = m32 = m33 = m34 = 0.0f;
		m41 = m42 = m43 = m24 = 0.0f;
	}
	else if(c == 1.0f) {
		return;
	}

	m11 /= c; m12 /= c; m13 /= c; m14 /= c;
	m21 /= c; m22 /= c; m23 /= c; m24 /= c;
	m31 /= c; m32 /= c; m33 /= c; m34 /= c;
	m41 /= c; m42 /= c; m43 /= c; m44 /= c;
}

void Matrix4x4::operator /= (const Matrix4x4 & matrix) {
	return operator *= (matrix.inverse());
}

float Matrix4x4::operator [] (size_t index) const {
	if(index < 0 || index > 15) { return 0.0f; }

	return m[index];
}

bool Matrix4x4::operator == (const Matrix4x4 & matrix) const {
	return m11 == matrix.m11 && m12 == matrix.m12 && m13 == matrix.m13 && m14 == matrix.m14 &&
		   m21 == matrix.m21 && m22 == matrix.m22 && m23 == matrix.m23 && m14 == matrix.m24 &&
		   m31 == matrix.m31 && m32 == matrix.m32 && m33 == matrix.m33 && m14 == matrix.m34 &&
		   m41 == matrix.m41 && m42 == matrix.m42 && m43 == matrix.m43 && m14 == matrix.m44;
}

bool Matrix4x4::operator != (const Matrix4x4 & matrix) const {
	return !operator == (matrix);
}

void Matrix4x4::setMatrix(float matrix11, float matrix12, float matrix13, float matrix14,
                          float matrix21, float matrix22, float matrix23, float matrix24,
                          float matrix31, float matrix32, float matrix33, float matrix34,
                          float matrix41, float matrix42, float matrix43, float matrix44) {
	m11 = matrix11; m12 = matrix12; m13 = matrix13; m14 = matrix14;
	m21 = matrix21; m22 = matrix22; m23 = matrix23; m24 = matrix24;
	m31 = matrix31; m32 = matrix32; m33 = matrix33; m34 = matrix34;
	m41 = matrix41; m42 = matrix42; m43 = matrix43; m44 = matrix44;
}

void Matrix4x4::setMatrix(const float matrix[16]) {
	memcpy(m, matrix, 16);
}

void Matrix4x4::setMatrix(const Matrix4x4 & matrix) {
	memcpy(m, matrix.m, 16);
}

float Matrix4x4::determinant() const {
	return m11 * ( m22 * m33 * m44 - m22 * m34 * m43 - m32 * m23 * m44 + m32 * m24 * m43 + m42 * m23 * m34 - m42 * m24 * m33) +
		   m12 * (-m21 * m33 * m44 + m21 * m34 * m43 + m31 * m23 * m44 - m31 * m24 * m43 - m41 * m23 * m34 + m41 * m24 * m33) +
		   m13 * ( m21 * m32 * m44 - m21 * m34 * m42 - m31 * m22 * m44 + m31 * m24 * m42 + m41 * m22 * m34 - m41 * m24 * m32) +
		   m14 * (-m21 * m32 * m43 + m21 * m33 * m42 + m31 * m22 * m43 - m31 * m23 * m42 - m41 * m22 * m33 + m41 * m23 * m32);
}

void Matrix4x4::transpose() {
	float temp;

	temp = m12;
	m12  = m21;
	m21  = temp;

	temp = m13;
	m13  = m31;
	m31  = temp;

	temp = m14;
	m14  = m41;
	m41  = temp;

	temp = m23;
	m23  = m32;
	m32  = temp;

	temp = m24;
	m24  = m42;
	m42  = temp;

	temp = m34;
	m34  = m43;
	m43  = temp;
}

Matrix4x4 Matrix4x4::transposed() const {
	return Matrix4x4(
		m11, m21, m31, m41,
		m12, m22, m32, m42,
		m13, m23, m33, m43,
		m14, m24, m34, m44
	);
}

void Matrix4x4::invert() {
	float inverse[16];

	inverse[0] =   m22 * m33 * m44 -
				   m22 * m34 * m43 -
				   m32 * m23 * m44 +
				   m32 * m24 * m43 +
				   m42 * m23 * m34 -
				   m42 * m24 * m33;

	inverse[4] =  -m21 * m33 * m44 +
				   m21 * m34 * m43 +
				   m31 * m23 * m44 -
				   m31 * m24 * m43 -
				   m41 * m23 * m34 +
				   m41 * m24 * m33;

	inverse[8] =   m21 * m32 * m44 -
				   m21 * m34 * m42 -
				   m31 * m22 * m44 +
				   m31 * m24 * m42 +
				   m41 * m22 * m34 -
				   m41 * m24 * m32;

	inverse[12] = -m21 * m32 * m43 +
				   m21 * m33 * m42 +
				   m31 * m22 * m43 -
				   m31 * m23 * m42 -
				   m41 * m22 * m33 +
				   m41 * m23 * m32;

	inverse[1] =  -m12 * m33 * m44 +
				   m12 * m34 * m43 +
				   m32 * m13 * m44 -
				   m32 * m14 * m43 -
				   m42 * m13 * m34 +
				   m42 * m14 * m33;

	inverse[5] =   m11 * m33 * m44 -
				   m11 * m34 * m43 -
				   m31 * m13 * m44 +
				   m31 * m14 * m43 +
				   m41 * m13 * m34 -
				   m41 * m14 * m33;

	inverse[9] =  -m11 * m32 * m44 +
				   m11 * m34 * m42 +
				   m31 * m12 * m44 -
				   m31 * m14 * m42 -
				   m41 * m12 * m34 +
				   m41 * m14 * m32;

	inverse[13] =  m11 * m32 * m43 -
				   m11 * m33 * m42 -
				   m31 * m12 * m43 +
				   m31 * m13 * m42 +
				   m41 * m12 * m33 -
				   m41 * m13 * m32;

	inverse[2] =   m12 * m23 * m44 -
				   m12 * m24 * m43 -
				   m22 * m13 * m44 +
				   m22 * m14 * m43 +
				   m42 * m13 * m24 -
				   m42 * m14 * m23;

	inverse[6] =  -m11 * m23 * m44 +
				   m11 * m24 * m43 +
				   m21 * m13 * m44 -
				   m21 * m14 * m43 -
				   m41 * m13 * m24 +
				   m41 * m14 * m23;

	inverse[10] =  m11 * m22 * m44 -
				   m11 * m24 * m42 -
				   m21 * m12 * m44 +
				   m21 * m14 * m42 +
				   m41 * m12 * m24 -
				   m41 * m14 * m22;

	inverse[14] = -m11 * m22 * m43 +
				   m11 * m23 * m42 + 
				   m21 * m12 * m43 -
				   m21 * m13 * m42 -
				   m41 * m12 * m23 +
				   m41 * m13 * m22;

	inverse[3] =  -m12 * m23 * m34 +
				   m12 * m24 * m33 +
				   m22 * m13 * m34 -
				   m22 * m14 * m33 -
				   m32 * m13 * m24 +
				   m32 * m14 * m23;

	inverse[7] =   m11 * m23 * m34 -
				   m11 * m24 * m33 -
				   m21 * m13 * m34 +
				   m21 * m14 * m33 +
				   m31 * m13 * m24 -
				   m31 * m14 * m23;

	inverse[11] = -m11 * m22 * m34 +
				   m11 * m24 * m32 +
				   m21 * m12 * m34 -
				   m21 * m14 * m32 -
				   m31 * m12 * m24 +
				   m31 * m14 * m22;

	inverse[15] =  m11 * m22 * m33 -
				   m11 * m23 * m32 -
				   m21 * m12 * m33 +
				   m21 * m13 * m32 +
				   m31 * m12 * m23 -
				   m31 * m13 * m22;

	float determinant = m11 * inverse[0] + m12 * inverse[4] + m13 * inverse[8] + m14 * inverse[12];

	if(determinant == 0.0f) {
		m11 = m12 = m13 = m14 = 0.0f;
		m21 = m22 = m23 = m24 = 0.0f;
		m31 = m32 = m33 = m34 = 0.0f;
		m41 = m42 = m43 = m44 = 0.0f;
	}
	else {
		float inverseDeterminant = 1.0f / determinant;

		m11 = inverse[0]  * inverseDeterminant; m12 = inverse[1]  * inverseDeterminant; m13 = inverse[2]  * inverseDeterminant; m14 = inverse[3]  * inverseDeterminant;
		m21 = inverse[4]  * inverseDeterminant; m22 = inverse[5]  * inverseDeterminant; m23 = inverse[6]  * inverseDeterminant; m24 = inverse[7]  * inverseDeterminant;
		m31 = inverse[8]  * inverseDeterminant; m32 = inverse[9]  * inverseDeterminant; m33 = inverse[10] * inverseDeterminant; m34 = inverse[11] * inverseDeterminant;
		m41 = inverse[12] * inverseDeterminant; m42 = inverse[13] * inverseDeterminant; m43 = inverse[14] * inverseDeterminant; m44 = inverse[15] * inverseDeterminant;
	}
}

Matrix4x4 Matrix4x4::inverse() const {
	float inverse[16];

	inverse[0] =   m22 * m33 * m44 -
				   m22 * m34 * m43 -
				   m32 * m23 * m44 +
				   m32 * m24 * m43 +
				   m42 * m23 * m34 -
				   m42 * m24 * m33;

	inverse[4] =  -m21 * m33 * m44 +
				   m21 * m34 * m43 +
				   m31 * m23 * m44 -
				   m31 * m24 * m43 -
				   m41 * m23 * m34 +
				   m41 * m24 * m33;

	inverse[8] =   m21 * m32 * m44 -
				   m21 * m34 * m42 -
				   m31 * m22 * m44 +
				   m31 * m24 * m42 +
				   m41 * m22 * m34 -
				   m41 * m24 * m32;

	inverse[12] = -m21 * m32 * m43 +
				   m21 * m33 * m42 +
				   m31 * m22 * m43 -
				   m31 * m23 * m42 -
				   m41 * m22 * m33 +
				   m41 * m23 * m32;

	inverse[1] =  -m12 * m33 * m44 +
				   m12 * m34 * m43 +
				   m32 * m13 * m44 -
				   m32 * m14 * m43 -
				   m42 * m13 * m34 +
				   m42 * m14 * m33;

	inverse[5] =   m11 * m33 * m44 -
				   m11 * m34 * m43 -
				   m31 * m13 * m44 +
				   m31 * m14 * m43 +
				   m41 * m13 * m34 -
				   m41 * m14 * m33;

	inverse[9] =  -m11 * m32 * m44 +
				   m11 * m34 * m42 +
				   m31 * m12 * m44 -
				   m31 * m14 * m42 -
				   m41 * m12 * m34 +
				   m41 * m14 * m32;

	inverse[13] =  m11 * m32 * m43 -
				   m11 * m33 * m42 -
				   m31 * m12 * m43 +
				   m31 * m13 * m42 +
				   m41 * m12 * m33 -
				   m41 * m13 * m32;

	inverse[2] =   m12 * m23 * m44 -
				   m12 * m24 * m43 -
				   m22 * m13 * m44 +
				   m22 * m14 * m43 +
				   m42 * m13 * m24 -
				   m42 * m14 * m23;

	inverse[6] =  -m11 * m23 * m44 +
				   m11 * m24 * m43 +
				   m21 * m13 * m44 -
				   m21 * m14 * m43 -
				   m41 * m13 * m24 +
				   m41 * m14 * m23;

	inverse[10] =  m11 * m22 * m44 -
				   m11 * m24 * m42 -
				   m21 * m12 * m44 +
				   m21 * m14 * m42 +
				   m41 * m12 * m24 -
				   m41 * m14 * m22;

	inverse[14] = -m11 * m22 * m43 +
				   m11 * m23 * m42 + 
				   m21 * m12 * m43 -
				   m21 * m13 * m42 -
				   m41 * m12 * m23 +
				   m41 * m13 * m22;

	inverse[3] =  -m12 * m23 * m34 +
				   m12 * m24 * m33 +
				   m22 * m13 * m34 -
				   m22 * m14 * m33 -
				   m32 * m13 * m24 +
				   m32 * m14 * m23;

	inverse[7] =   m11 * m23 * m34 -
				   m11 * m24 * m33 -
				   m21 * m13 * m34 +
				   m21 * m14 * m33 +
				   m31 * m13 * m24 -
				   m31 * m14 * m23;

	inverse[11] = -m11 * m22 * m34 +
				   m11 * m24 * m32 +
				   m21 * m12 * m34 -
				   m21 * m14 * m32 -
				   m31 * m12 * m24 +
				   m31 * m14 * m22;

	inverse[15] =  m11 * m22 * m33 -
				   m11 * m23 * m32 -
				   m21 * m12 * m33 +
				   m21 * m13 * m32 +
				   m31 * m12 * m23 -
				   m31 * m13 * m22;

	float determinant = m11 * inverse[0] + m12 * inverse[4] + m13 * inverse[8] + m14 * inverse[12];

	if(determinant == 0.0f) {
		return Zero;
	}
	else {
		float inverseDeterminant = 1.0f / determinant;

		return Matrix4x4(
			inverse[0]  * inverseDeterminant, inverse[1]  * inverseDeterminant, inverse[2]  * inverseDeterminant, inverse[3]  * inverseDeterminant,
			inverse[4]  * inverseDeterminant, inverse[5]  * inverseDeterminant, inverse[6]  * inverseDeterminant, inverse[7]  * inverseDeterminant,
			inverse[8]  * inverseDeterminant, inverse[9]  * inverseDeterminant, inverse[10] * inverseDeterminant, inverse[11] * inverseDeterminant,
			inverse[12] * inverseDeterminant, inverse[13] * inverseDeterminant, inverse[14] * inverseDeterminant, inverse[15] * inverseDeterminant
		);
	}
}

std::string Matrix4x4::toString() const {
	return std::to_string(m11) + ", " + std::to_string(m12) + ", " + std::to_string(m13) + ", " + std::to_string(m14) + ", " +
		   std::to_string(m21) + ", " + std::to_string(m22) + ", " + std::to_string(m23) + ", " + std::to_string(m24) + ", " +
		   std::to_string(m31) + ", " + std::to_string(m32) + ", " + std::to_string(m33) + ", " + std::to_string(m34) + ", " +
		   std::to_string(m41) + ", " + std::to_string(m42) + ", " + std::to_string(m43) + ", " + std::to_string(m44);
}

Matrix4x4 Matrix4x4::parseFrom(const std::string & data, bool * error) {
	static const std::regex      nonFloatRegExp("[^-0-9.]+");
	static const std::regex  nonFloatTrimRegExp("(^[^-0-9.]+)|([^-0-9.]+$)");

	std::string trimmedData;
	std::regex_replace(std::back_inserter(trimmedData), data.begin(), data.end(), nonFloatTrimRegExp, "");

	std::string formattedData;
	std::regex_replace(std::back_inserter(formattedData), trimmedData.begin(), trimmedData.end(), nonFloatRegExp, " ");

	size_t index = 0, start = -1, end = -1;
	std::string part;
	bool success = false;
	Matrix4x4 newMatrix;
	for(size_t i = 0; i < formattedData.length(); i++) {
		if(formattedData[i] == ' ' || i == formattedData.length() - 1) {
			if(index > 15) {
				if(error != nullptr) { *error = true; }
				return Zero;
			}

			start = end + 1;
			end = i == formattedData.length() - 1 ? i + 1 : i;

			part = Utilities::substring(formattedData, start, end);

			newMatrix.m[index++] = Utilities::parseFloat(part, &success);

			if(!success) {
				if(error != nullptr) { *error = true; }
				return Zero;
			}
		}
	}

	if(index != 16) {
		if(error != nullptr) { *error = true; }
		return Zero;
	}

	return newMatrix;
}
