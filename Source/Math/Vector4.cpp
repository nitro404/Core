#include "Vector4.h"

#include "Vector2.h"
#include "Vector3.h"

#include <fmt/core.h>

const Vector4 Vector4::Zero    ( 0.0f,  0.0f,  0.0f,  0.0f);
const Vector4 Vector4::One     ( 1.0f,  1.0f,  1.0f,  1.0f);
const Vector4 Vector4::Forward ( 0.0f,  0.0f, -1.0f,  0.0f);
const Vector4 Vector4::Backward( 0.0f,  0.0f,  1.0f,  0.0f);
const Vector4 Vector4::Left    (-1.0f,  0.0f,  0.0f,  0.0f);
const Vector4 Vector4::Right   ( 1.0f,  0.0f,  0.0f,  0.0f);
const Vector4 Vector4::Up      ( 0.0f,  1.0f,  0.0f,  0.0f);
const Vector4 Vector4::Down    ( 0.0f, -1.0f,  0.0f,  0.0f);

Vector4::Vector4              (float a, float b, float c, float d) : x(a), y(b), z(c), w(d)  { }
Vector4::Vector4              (const float v[4])        : x(v[0]), y(v[1]), z(v[2]), w(v[3]) { }
Vector4::Vector4              (const Vector2 & v)       : x(v.x),  y(v.y),  z(0.0f), w(0.0f) { }
Vector4::Vector4              (const Vector3 & v)       : x(v.x),  y(v.y),  z(v.z),  w(0.0f) { }
Vector4::Vector4              (const Vector4 & v)       : x(v.x),  y(v.y),  z(v.z),  w(v.w)  { }
Vector4 & Vector4::operator = (const Vector2 & v)       { x = v.x; y = v.y; z = 0.0f; w = 0.0f; return *this; }
Vector4 & Vector4::operator = (const Vector3 & v)       { x = v.x; y = v.y; z = v.z;  w = 0.0f; return *this; }
Vector4 & Vector4::operator = (const Vector4 & v)       { x = v.x; y = v.y; z = v.z;  w = v.w;  return *this; }
Vector4::~Vector4             ()                        { }

Vector4 Vector4::operator   + (float c)           const { return Vector4(x + c,   y + c,   z + c,   w + c); }
Vector4 Vector4::operator   + (const Vector2 & v) const { return Vector4(x + v.x, y + v.y, z,       w); }
Vector4 Vector4::operator   + (const Vector3 & v) const { return Vector4(x + v.x, y + v.y, z + v.z, w); }
Vector4 Vector4::operator   + (const Vector4 & v) const { return Vector4(x + v.x, y + v.y, z + v.z, w + v.w); }

void Vector4::operator     += (float c)                 { x += c;   y += c;   z += c;   w += c; }
void Vector4::operator     += (const Vector2 & v)       { x += v.x; y += v.y; }
void Vector4::operator     += (const Vector3 & v)       { x += v.x; y += v.y; z += v.z; }
void Vector4::operator     += (const Vector4 & v)       { x += v.x; y += v.y; z += v.z; w += v.w; }

Vector4 Vector4::operator   - ()                  const { return Vector4(-x, -y, -z, -w); }

Vector4 Vector4::operator   - (float c)           const { return Vector4(x - c,   y - c,   z - c,   w - c); }
Vector4 Vector4::operator   - (const Vector2 & v) const { return Vector4(x - v.x, y - v.y, z,       w); }
Vector4 Vector4::operator   - (const Vector3 & v) const { return Vector4(x - v.x, y - v.y, z - v.z, w); }
Vector4 Vector4::operator   - (const Vector4 & v) const { return Vector4(x - v.x, y - v.y, z - v.z, w - v.w); }

void Vector4::operator     -= (float c)                 { x -= c;   y -= c;   z -= c;   w -= c; }
void Vector4::operator     -= (const Vector2 & v)       { x -= v.x; y -= v.y; }
void Vector4::operator     -= (const Vector3 & v)       { x -= v.x; y -= v.y; z -= v.z; }
void Vector4::operator     -= (const Vector4 & v)       { x -= v.x; y -= v.y; z -= v.z; w -= v.w; }

Vector4 Vector4::operator   * (float c)           const { return Vector4(x * c,   y * c,   z * c,   w * c); }
Vector4 Vector4::operator   * (const Vector2 & v) const { return Vector4(x * v.x, y * v.y, z,       w); }
Vector4 Vector4::operator   * (const Vector3 & v) const { return Vector4(x * v.x, y * v.y, z * v.z, w); }
Vector4 Vector4::operator   * (const Vector4 & v) const { return Vector4(x * v.x, y * v.y, z * v.z, w * v.w); }

void Vector4::operator     *= (float c)                 { x *= c;   y *= c;   z *= c;   w *= c; }
void Vector4::operator     *= (const Vector2 & v)       { x *= v.x; y *= v.y; }
void Vector4::operator     *= (const Vector3 & v)       { x *= v.x; y *= v.y; z *= v.z; }
void Vector4::operator     *= (const Vector4 & v)       { x *= v.x; y *= v.y; z *= v.z; w *= v.w; }

Vector4 Vector4::operator   / (float c)           const { return Vector4(  c == 0.0f ? 0.0f : x / c,     c == 0.0f ? 0.0f : y / c,     c == 0.0f ? 0.0f : z / c,     c == 0.0f ? 0.0f : w / c); }
Vector4 Vector4::operator   / (const Vector2 & v) const { return Vector4(v.x == 0.0f ? 0.0f : x / v.x, v.y == 0.0f ? 0.0f : y / v.y,                      z,                            w); }
Vector4 Vector4::operator   / (const Vector3 & v) const { return Vector4(v.x == 0.0f ? 0.0f : x / v.x, v.y == 0.0f ? 0.0f : y / v.y, v.z == 0.0f ? 0.0f : z / v.z,                      w); }
Vector4 Vector4::operator   / (const Vector4 & v) const { return Vector4(v.x == 0.0f ? 0.0f : x / v.x, v.y == 0.0f ? 0.0f : y / v.y, v.z == 0.0f ? 0.0f : z / v.z, v.w == 0.0f ? 0.0f : w / v.w); }

void Vector4::operator     /= (float c)                 { x =   c == 0.0f ? 0.0f : x / c;   y =   c == 0.0f ? 0.0f : y / c;   z =   c == 0.0f ? 0.0f : z / c;   w =   c == 0.0f ? 0.0f : w / c; }
void Vector4::operator     /= (const Vector2 & v)       { x = v.x == 0.0f ? 0.0f : x / v.x; y = v.y == 0.0f ? 0.0f : y / v.y; }
void Vector4::operator     /= (const Vector3 & v)       { x = v.x == 0.0f ? 0.0f : x / v.x; y = v.y == 0.0f ? 0.0f : y / v.y; z = v.z == 0.0f ? 0.0f : z / v.z; }
void Vector4::operator     /= (const Vector4 & v)       { x = v.x == 0.0f ? 0.0f : x / v.x; y = v.y == 0.0f ? 0.0f : y / v.y; z = v.z == 0.0f ? 0.0f : z / v.z; w = v.w == 0.0f ? 0.0f : w / v.w; }

float Vector4::operator    [] (size_t index)      const { return index < 0 || index > 3 ? 0 : v[index]; }

bool Vector4::operator     == (const Vector4 & v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
bool Vector4::operator     != (const Vector4 & v) const { return x != v.x || y != v.y || z != v.z || w != v.w; }

void Vector4::setVector       (float a, float b, float c, float d) { x = a; y = b; z = c; w = d; }
void Vector4::setVector       (const float vector[4])   { x = v[0]; y = v[1]; z = v[2]; w = v[3]; }
void Vector4::setVector       (const Vector2 & vector)  { x = vector.x; y = vector.y; z = 0.0f; w = 0.0f; }
void Vector4::setVector       (const Vector3 & vector)  { x = vector.x; y = vector.y; z = vector.z; w = 0.0f; }
void Vector4::setVector       (const Vector4 & vector)  { x = vector.x; y = vector.y; z = vector.z; w = vector.w; }

float Vector4::length         ()                  const { return sqrt((x * x) + (y * y) + (z * z) + (w * w)); }
float Vector4::dot            (const Vector4 & v) const { return (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w); }
void Vector4::normalize       ()                        { float l = length(); if(l == 0.0f) { x = y = z = w = 0.0f; } else { x /= l; y /= l; z /= l; w /= l; } }
Vector4 Vector4::normalized   ()                  const { float l = length(); if(l == 0.0f) { return Zero; } else { return Vector4(x / l, y / l, z / l, w / l); } }

std::string Vector4::toString() const {
	return fmt::format("{}, {}, {}, {}", x, y, z, w);
}
