#include "Vector2.h"

#include "Utilities/StringUtilities.h"
#include "Vector3.h"
#include "Vector4.h"

#include <regex>

const Vector2 Vector2::Zero   ( 0.0f,  0.0f);
const Vector2 Vector2::One    ( 1.0f,  1.0f);
const Vector2 Vector2::Left   (-1.0f,  0.0f);
const Vector2 Vector2::Right  ( 1.0f,  0.0f);
const Vector2 Vector2::Up     ( 0.0f,  1.0f);
const Vector2 Vector2::Down	  ( 0.0f, -1.0f);

Vector2::Vector2              (float a, float b)        : x(a),    y(b)    { }
Vector2::Vector2              (const float v[2])        : x(v[0]), y(v[1]) { }
Vector2::Vector2              (const Vector2 & v)       : x(v.x),  y(v.y)  { }
Vector2::Vector2              (const Vector3 & v)       : x(v.x),  y(v.y)  { }
Vector2::Vector2              (const Vector4 & v)       : x(v.x),  y(v.y)  { }
Vector2 & Vector2::operator = (const Vector2 & v)       { x = v.x; y = v.y; return *this; }
Vector2 & Vector2::operator = (const Vector3 & v)       { x = v.x; y = v.y; return *this; }
Vector2 & Vector2::operator = (const Vector4 & v)       { x = v.x; y = v.y; return *this; }
Vector2::~Vector2             ()                        { }

Vector2 Vector2::operator   + (float c)           const { return Vector2(x + c,   y + c); }
Vector2 Vector2::operator   + (const Vector2 & v) const { return Vector2(x + v.x, y + v.y); }
Vector2 Vector2::operator   + (const Vector3 & v) const { return Vector2(x + v.x, y + v.y); }
Vector2 Vector2::operator   + (const Vector4 & v) const { return Vector2(x + v.x, y + v.y); }

void Vector2::operator     += (float c)                 { x += c;   y += c; }
void Vector2::operator     += (const Vector2 & v)       { x += v.x; y += v.y; }
void Vector2::operator     += (const Vector3 & v)       { x += v.x; y += v.y; }
void Vector2::operator     += (const Vector4 & v)       { x += v.x; y += v.y; }

Vector2 Vector2::operator   - ()                  const { return Vector2(-x, -y); }

Vector2 Vector2::operator   - (float c)           const { return Vector2(x - c,   y - c); }
Vector2 Vector2::operator   - (const Vector2 & v) const { return Vector2(x - v.x, y - v.y); }
Vector2 Vector2::operator   - (const Vector3 & v) const { return Vector2(x - v.x, y - v.y); }
Vector2 Vector2::operator   - (const Vector4 & v) const { return Vector2(x - v.x, y - v.y); }

void Vector2::operator     -= (float c)                 { x -= c;   y -= c; }
void Vector2::operator     -= (const Vector2 & v)       { x -= v.x; y -= v.y; }
void Vector2::operator     -= (const Vector3 & v)       { x -= v.x; y -= v.y; }
void Vector2::operator     -= (const Vector4 & v)       { x -= v.x; y -= v.y; }

Vector2 Vector2::operator   * (float c)           const { return Vector2(x * c,   y * c); }
Vector2 Vector2::operator   * (const Vector2 & v) const { return Vector2(x * v.x, y * v.y); }
Vector2 Vector2::operator   * (const Vector3 & v) const { return Vector2(x * v.x, y * v.y); }
Vector2 Vector2::operator   * (const Vector4 & v) const { return Vector2(x * v.x, y * v.y); }

void Vector2::operator     *= (float c)                 { x *= c;   y *= c; }
void Vector2::operator     *= (const Vector2 & v)       { x *= v.x; y *= v.y; }
void Vector2::operator     *= (const Vector3 & v)       { x *= v.x; y *= v.y; }
void Vector2::operator     *= (const Vector4 & v)       { x *= v.x; y *= v.y; }

Vector2 Vector2::operator   / (float c)           const { return Vector2(  c == 0.0f ? 0.0f : x / c,     c == 0.0f ? 0.0f : y / c); }
Vector2 Vector2::operator   / (const Vector2 & v) const { return Vector2(v.x == 0.0f ? 0.0f : x / v.x, v.y == 0.0f ? 0.0f : y / v.y); }
Vector2 Vector2::operator   / (const Vector3 & v) const { return Vector2(v.x == 0.0f ? 0.0f : x / v.x, v.y == 0.0f ? 0.0f : y / v.y); }
Vector2 Vector2::operator   / (const Vector4 & v) const { return Vector2(v.x == 0.0f ? 0.0f : x / v.x, v.y == 0.0f ? 0.0f : y / v.y); }

void Vector2::operator     /= (float c)                 { x =   c == 0.0f ? 0.0f : x / c;   y =   c == 0.0f ? 0.0f : y / c; }
void Vector2::operator     /= (const Vector2 & v)       { x = v.x == 0.0f ? 0.0f : x / v.x; y = v.y == 0.0f ? 0.0f : y / v.y; }
void Vector2::operator     /= (const Vector3 & v)       { x = v.x == 0.0f ? 0.0f : x / v.x; y = v.y == 0.0f ? 0.0f : y / v.y; }
void Vector2::operator     /= (const Vector4 & v)       { x = v.x == 0.0f ? 0.0f : x / v.x; y = v.y == 0.0f ? 0.0f : y / v.y; }

float Vector2::operator    [] (size_t index)      const { return index < 0 || index > 1 ? 0 : v[index]; }

bool Vector2::operator     == (const Vector2 & v) const { return x == v.x && y == v.y; }
bool Vector2::operator     != (const Vector2 & v) const { return x != v.x || y != v.y; }

void Vector2::setVector       (float a, float b)        { x = a; y = b; }
void Vector2::setVector       (const float vector[2])   { x = v[0]; y = v[1]; }
void Vector2::setVector       (const Vector2 & vector)  { x = vector.x; y = vector.y; }
void Vector2::setVector       (const Vector3 & vector)  { x = vector.x; y = vector.y; }
void Vector2::setVector       (const Vector4 & vector)  { x = vector.x; y = vector.y; }

float Vector2::length         ()                  const { return sqrt((x * x) + (y * y)); }
float Vector2::dot            (const Vector2 & v) const { return (x * v.x) + (y * v.y); }
void Vector2::normalize       ()                        { float l = length(); if(l == 0.0f) { x = y = 0.0f; } else { x /= l; y /= l; } }
Vector2 Vector2::normalized   ()                  const { float l = length(); if(l == 0.0f) { return Zero; } else { return Vector2(x / l, y / l); } }

std::string Vector2::toString() const {
	return std::to_string(x) + ", " + std::to_string(y);
}

Vector2 Vector2::parseFrom(const std::string & data, bool * error) {
	static const std::regex     nonFloatRegExp("[^-0-9.]+");
	static const std::regex nonFloatTrimRegExp("(^[^-0-9.]+)|([^-0-9.]+$)");

	std::string trimmedData;
	std::regex_replace(std::back_inserter(trimmedData), data.begin(), data.end(), nonFloatTrimRegExp, "");

	std::string formattedData;
	std::regex_replace(std::back_inserter(formattedData), trimmedData.begin(), trimmedData.end(), nonFloatRegExp, " ");

	size_t index = 0, start = -1, end = -1;
	std::string part;
	bool success = false;
	Vector2 newVector;
	for(size_t i = 0; i < formattedData.length(); i++) {
		if(formattedData[i] == ' ' || i == formattedData.length() - 1) {
			if(index > 1) {
				if(error != nullptr) { *error = true; }
				return Zero;
			}

			start = end + 1;
			end = i == formattedData.length() - 1 ? i + 1 : i;

			part = Utilities::substring(formattedData, start, end);

			newVector.v[index++] = Utilities::parseFloat(part, &success);

			if(!success) {
				if(error != nullptr) { *error = true; }
				return Zero;
			}
		}
	}

	if(index != 2) {
		if(error != nullptr) { *error = true; }
		return Zero;
	}

	return newVector;
}
