#include "Matrix2x2.h"

#include "ExtendedMath.h"
#include "Vector2.h"

#include <fmt/core.h>

#include <cmath>

const Matrix2x2 Matrix2x2::Zero(
	0.0f, 0.0f,
	0.0f, 0.0f
);

const Matrix2x2 Matrix2x2::One(
	1.0f, 1.0f,
	1.0f, 1.0f
);

const Matrix2x2 Matrix2x2::Identity(
	1.0f, 0.0f,
	0.0f, 1.0f
);

Matrix2x2::Matrix2x2(float matrix11, float matrix12,
                     float matrix21, float matrix22)
	: m11(matrix11), m12(matrix12)
	, m21(matrix21), m22(matrix22) { }

Matrix2x2::Matrix2x2(const float matrix[4])
	: m11(matrix[0]), m12(matrix[1])
	, m21(matrix[2]), m22(matrix[3]) { }

Matrix2x2::Matrix2x2(const Matrix2x2 & matrix)
	: m11(matrix.m11), m12(matrix.m12)
	, m21(matrix.m21), m22(matrix.m22) { }

Matrix2x2 & Matrix2x2::operator = (const Matrix2x2 & matrix) {
	memcpy(m, matrix.m, 4);

	return *this;
}

Matrix2x2::~Matrix2x2() = default;

Matrix2x2 Matrix2x2::operator + (float c)  const {
	if(c == 0.0f) { return *this; }

	return Matrix2x2(
		m11 + c, m12 + c,
		m21 + c, m22 + c
	);
}

Matrix2x2 Matrix2x2::operator + (const Matrix2x2 & matrix) const {
	return Matrix2x2(
		m11 + matrix.m11, m12 + matrix.m12,
		m21 + matrix.m21, m22 + matrix.m22
	);
}

void Matrix2x2::operator += (float c) {
	if(c == 0.0f) { return; }

	m11 += c; m12 += c;
	m21 += c; m22 += c;
}

void Matrix2x2::operator += (const Matrix2x2 & matrix) {
	m11 += matrix.m11; m12 += matrix.m12;
	m21 += matrix.m21; m22 += matrix.m22;
}

Matrix2x2 Matrix2x2::operator - () const {
	return Matrix2x2(
		-m11, -m12,
		-m21, -m22
	);
}

Matrix2x2 Matrix2x2::operator - (float c) const {
	if(c == 0.0f) { return *this; }

	return Matrix2x2(
		m11 - c, m12 - c,
		m21 - c, m22 - c
	);
}

Matrix2x2 Matrix2x2::operator - (const Matrix2x2 & matrix) const {
	return Matrix2x2(
		m11 - matrix.m11, m12 - matrix.m12,
		m21 - matrix.m21, m22 - matrix.m22
	);
}

void Matrix2x2::operator -= (float c) {
	if(c == 0.0f) { return; }

	m11 -= c; m12 -= c;
	m21 -= c; m22 -= c;
}

void Matrix2x2::operator -= (const Matrix2x2 & matrix) {
	m11 -= matrix.m11; m12 -= matrix.m12;
	m21 -= matrix.m21; m22 -= matrix.m22;
}

Matrix2x2 Matrix2x2::operator * (float c) const {
		 if(c == 0.0f) { return Zero; }
	else if(c == 1.0f) { return *this; }

	return Matrix2x2(
		m11 * c, m12 * c,
		m21 * c, m22 * c
	);
}

Matrix2x2 Matrix2x2::operator * (const Matrix2x2 & matrix) const {
	return Matrix2x2(
		m11 * matrix.m11 + m12 * matrix.m21,
		m11 * matrix.m12 + m12 * matrix.m22,

		m21 * matrix.m11 + m22 * matrix.m21,
		m21 * matrix.m12 + m22 * matrix.m22
	);
}

void Matrix2x2::operator *= (float c) {
	if(c == 1.0f) { return; }

	m11 *= c; m12 *= c;
	m21 *= c; m22 *= c;
}

void Matrix2x2::operator *= (const Matrix2x2 & matrix) {
	float product[4];

	product[0] = m11 * matrix.m11 + m12 * matrix.m21;
	product[1] = m11 * matrix.m12 + m12 * matrix.m22;

	product[2] = m21 * matrix.m11 + m22 * matrix.m21;
	product[3] = m21 * matrix.m12 + m22 * matrix.m22;

	memcpy(m, product, 4);
}

Matrix2x2 Matrix2x2::operator / (float c) const {
		 if(c == 0.0f) { return Zero; }
	else if(c == 1.0f) { return *this; }

	return Matrix2x2(
		m11 / c, m12 / c,
		m21 / c, m22 / c
	);
}

Matrix2x2 Matrix2x2::operator / (const Matrix2x2 & matrix) const {
	return *this * matrix.inverse();
}

void Matrix2x2::operator /= (float c) {
	if(c == 0.0f) {
		m11 = m12 = 0.0f;
		m21 = m22 = 0.0f;
	}
	else if(c == 1.0f) {
		return;
	}

	m11 /= c; m12 /= c;
	m21 /= c; m22 /= c;
}

void Matrix2x2::operator /= (const Matrix2x2 & matrix) {
	return operator *= (matrix.inverse());
}

float Matrix2x2::operator [] (size_t index) const {
	if(index < 0 || index > 3) { return 0.0f; }

	return m[index];
}

bool Matrix2x2::operator == (const Matrix2x2 & matrix) const {
	return m11 == matrix.m11 && m12 == matrix.m12 &&
		   m21 == matrix.m21 && m22 == matrix.m22;
}

bool Matrix2x2::operator != (const Matrix2x2 & matrix) const {
	return !operator == (matrix);
}

void Matrix2x2::setMatrix(float matrix11, float matrix12,
                          float matrix21, float matrix22) {
	m11 = matrix11; m12 = matrix12;
	m21 = matrix21; m22 = matrix22;
}

void Matrix2x2::setMatrix(const float matrix[4]) {
	memcpy(m, matrix, 4);
}

void Matrix2x2::setMatrix(const Matrix2x2 & matrix) {
	memcpy(m, matrix.m, 4);
}

void Matrix2x2::rotate(float degrees) {
	float radians = Math::degreesToRadians(degrees);

	float s = sin(radians);
	float c = cos(radians);

	float rotation[4];

	rotation[0] = c * m11 - s * m12;
	rotation[1] = s * m11 + c * m12;

	rotation[2] = c * m21 - s * m22;
	rotation[3] = s * m21 + c * m22;

	m11 = rotation[0]; m12 = rotation[1];
	m21 = rotation[2]; m22 = rotation[3];
}

Matrix2x2 Matrix2x2::rotated(float degrees) const {
	return createRotation(degrees) * (*this);
}

Matrix2x2 Matrix2x2::createRotation(float degrees) {
	float radians = Math::degreesToRadians(degrees);

	return Matrix2x2(
		 cos(radians), sin(radians),
		-sin(radians), cos(radians)
	);
}

void Matrix2x2::scale(float amount) {
	m11 *= amount; m12 *= amount;
	m21 *= amount; m22 *= amount;
}

void Matrix2x2::scale(float x, float y) {
	m11 *= x; m12 *= y;
	m21 *= x; m22 *= y;
}

void Matrix2x2::scale(const Vector2 & amount) {
	m11 *= amount.x; m12 *= amount.y;
	m21 *= amount.x; m22 *= amount.y;
}

Matrix2x2 Matrix2x2::scaled(float amount) const {
	return createScale(amount) * (*this);
}

Matrix2x2 Matrix2x2::scaled(float x, float y) const {
	return createScale(x, y) * (*this);
}

Matrix2x2 Matrix2x2::scaled(const Vector2 & amount) const {
	return createScale(amount) * (*this);
}

Matrix2x2 Matrix2x2::createScale(float amount) {
	return Matrix2x2(
		amount, 0.0f,
		0.0f,   amount
	);
}

Matrix2x2 Matrix2x2::createScale(float x, float y) {
	return Matrix2x2(
		x,    0.0f,
		0.0f, y
	);
}

Matrix2x2 Matrix2x2::createScale(const Vector2 & amount) {
	return Matrix2x2(
		amount.x, 0.0f,
		0.0f,     amount.y
	);
}

float Matrix2x2::determinant() const {
	return (m11 * m22) - (m12 * m21);
}

void Matrix2x2::transpose() {
	float temp;

	temp = m12;
	m12  = m21;
	m21  = temp;
}

Matrix2x2 Matrix2x2::transposed() const {
	return Matrix2x2(
		m11, m21,
		m12, m22
	);
}

void Matrix2x2::invert() {
	float determinant = (m11 * m22) - (m12 * m21);

	if(determinant == 0.0f) {
		m11 = m12 = 0.0f;
		m21 = m22 = 0.0f;
	}
	else {
		float inverse[4];

		float inverseDeterminant = 1.0f / determinant;

		inverse[0] =  m22 * inverseDeterminant;
		inverse[1] = -m12 * inverseDeterminant;
		inverse[2] = -m21 * inverseDeterminant;
		inverse[3] =  m11 * inverseDeterminant;

		m11 = inverse[0]; m12 = inverse[1];
		m21 = inverse[2]; m22 = inverse[3];
	}
}

Matrix2x2 Matrix2x2::inverse() const {
	float determinant = (m11 * m22) - (m12 * m21);

	if(determinant == 0.0f) {
		return Zero;
	}
	else {
		float inverseDeterminant = 1.0f / determinant;

		return Matrix2x2(
			 m22 * inverseDeterminant, -m12 * inverseDeterminant
			-m21 * inverseDeterminant,  m11 * inverseDeterminant
		);
	}
}

std::string Matrix2x2::toString() const {
	return fmt::format("{}, {}, {}, {}", m11, m12, m21, m22);
}
