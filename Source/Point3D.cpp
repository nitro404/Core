#include "Point3D.h"

#include "ByteBuffer.h"
#include "Point2D.h"
#include "Utilities/RapidJSONUtilities.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <cmath>

static constexpr const char * JSON_X_POSITION_PROPERTY_NAME = "x";
static constexpr const char * JSON_Y_POSITION_PROPERTY_NAME = "y";
static constexpr const char * JSON_Z_POSITION_PROPERTY_NAME = "z";

const Point3D Point3D::ZERO(0, 0, 0);

Point3D::Point3D()
	: x(0)
	, y(0)
	, z(0) { }

Point3D::Point3D(int32_t a, int32_t b, int32_t c)
	: x(a)
	, y(b)
	, z(c) { }

Point3D::Point3D(int32_t p[3])
	: x(p[0])
	, y(p[1])
	, z(p[2]) { }

Point3D::Point3D(const Point3D & p)
	: x(p.x)
	, y(p.y)
	, z(p.z) { }

Point3D & Point3D::operator = (const Point3D & p) {
	x = p.x;
	y = p.y;

	return *this;
}

Point3D::~Point3D() = default;

Point3D Point3D::operator  + (int32_t c)         const { return Point3D(x + c,   y + c,   z + c); }
Point3D Point3D::operator  + (const Point2D & p) const { return Point3D(x + p.x, y + p.y, 0); }
Point3D Point3D::operator  + (const Point3D & p) const { return Point3D(x + p.x, y + p.y, z + p.z); }

void Point3D::operator    += (int32_t c)               { x += c;   y += c;   z += c; }
void Point3D::operator    += (const Point2D & p)       { x += p.x; y += p.y; }
void Point3D::operator    += (const Point3D & p)       { x += p.x; y += p.y; z += p.z; }

Point3D Point3D::operator  - ()                  const { return Point3D(-x, -y, -z); }

Point3D Point3D::operator  - (int32_t c)         const { return Point3D(x - c,   y - c,   z - c); }
Point3D Point3D::operator  - (const Point2D & p) const { return Point3D(x - p.x, y - p.y, 0); }
Point3D Point3D::operator  - (const Point3D & p) const { return Point3D(x - p.x, y - p.y, z - p.z); }

void Point3D::operator    -= (int32_t c)               { x -= c;   y -= c;   z -= c; }
void Point3D::operator    -= (const Point2D & p)       { x -= p.x; y -= p.y; }
void Point3D::operator    -= (const Point3D & p)       { x -= p.x; y -= p.y; z -= p.z; }

Point3D Point3D::operator  * (double c)          const { return Point3D(x * c,   y * c,   z * c); }
Point3D Point3D::operator  * (const Point2D & p) const { return Point3D(x * p.x, y * p.y, 0); }
Point3D Point3D::operator  * (const Point3D & p) const { return Point3D(x * p.x, y * p.y, z * p.z); }

void Point3D::operator    *= (double c)                { x *= c;   y *= c;   z *= c; }
void Point3D::operator    *= (const Point2D & p)       { x *= p.x; y *= p.y; }
void Point3D::operator    *= (const Point3D & p)       { x *= p.x; y *= p.y; z *= p.z; }

Point3D Point3D::operator  / (double c)          const { return Point3D(  c == 0.0 ? 0.0 : x / c,     c == 0.0 ? 0.0 : y / c,     c == 0.0 ? 0.0 : z / c); }
Point3D Point3D::operator  / (const Point2D & p) const { return Point3D(p.x == 0   ? 0   : x / p.x, p.y == 0   ? 0   : y / p.y, 0); }
Point3D Point3D::operator  / (const Point3D & p) const { return Point3D(p.x == 0   ? 0   : x / p.x, p.y == 0   ? 0   : y / p.y, p.z == 0   ? 0   : z / p.z); }

void Point3D::operator    /= (double c)                { x =   c == 0.0 ? 0.0 : x / c;   y =   c == 0.0 ? 0.0 : y / c;   z =   c == 0.0 ? 0.0 : z / c; }
void Point3D::operator    /= (const Point2D & p)       { x = p.x == 0   ? 0   : x / p.x; y = p.y == 0   ? 0   : y / p.y; }
void Point3D::operator    /= (const Point3D & p)       { x = p.x == 0   ? 0   : x / p.x; y = p.y == 0   ? 0   : y / p.y; z = p.z == 0   ? 0   : z / p.z; }

int32_t Point3D::operator [] (size_t index) const {
	if(index > 2) {
		return 0;
	}

	return p[index];
}

bool Point3D::operator == (const Point3D & p) const {
	return x == p.x &&
		   y == p.y &&
		   z == p.z;
}

bool Point3D::operator != (const Point3D & p) const {
	return !operator == (p);
}

void Point3D::setPoint(int32_t a, int32_t b, int32_t c) {
	x = a;
	y = b;
	z = c;
}

void Point3D::setPoint(const int32_t p[3]) {
	x = p[0];
	y = p[1];
	z = p[2];
}

double Point3D::distanceBetween(const Point3D & p) const {
	return sqrt(pow(p.x - x, 2) + pow(p.y - y, 2) + pow(p.z - z, 2));
}

Point3D Point3D::getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error) {
	if(offset + SIZE_BYTES > byteBuffer.getSize()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool internalError = false;

	Point3D value;

	for(size_t i = 0; i < 3; i++) {
		value.p[i] = byteBuffer.getInteger(offset + (sizeof(int32_t) * i), &internalError);

		if(internalError) {
			if(error != nullptr) {
				*error = true;
			}

			return {};
		}
	}

	return value;
}

std::optional<Point3D> Point3D::getFrom(const ByteBuffer & byteBuffer, size_t offset) {
	bool error = false;

	Point3D value(getFrom(byteBuffer, offset, &error));

	if(error) {
		return {};
	}

	return value;
}

Point3D Point3D::readFrom(const ByteBuffer & byteBuffer, bool * error) {
	bool internalError = false;

	Point3D value(getFrom(byteBuffer, byteBuffer.getReadOffset(), error));

	if(internalError) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		byteBuffer.skipReadBytes(SIZE_BYTES);
	}

	return value;
}

std::optional<Point3D> Point3D::readFrom(const ByteBuffer & byteBuffer) {
	bool error = false;

	Point3D value(readFrom(byteBuffer, &error));

	if(error) {
		return {};
	}

	return value;
}

bool Point3D::putIn(ByteBuffer & byteBuffer, size_t offset) const {
	for(size_t i = 0; i < 3; i++) {
		if(!byteBuffer.putInteger(p[i], offset + (sizeof(int32_t) * i))) {
			return false;
		}
	}

	return true;
}

bool Point3D::insertIn(ByteBuffer & byteBuffer, size_t offset) const {
	for(size_t i = 0; i < 3; i++) {
		if(!byteBuffer.insertInteger(p[i], offset + (sizeof(int32_t) * i))) {
			return false;
		}
	}

	return true;
}

bool Point3D::writeTo(ByteBuffer & byteBuffer) const {
	for(size_t i = 0; i < 3; i++) {
		if(!byteBuffer.putUnsignedLong(p[i], byteBuffer.getWriteOffset())) {
			return false;
		}

		byteBuffer.skipWriteBytes(sizeof(int32_t));
	}

	return true;
}

rapidjson::Value Point3D::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value pointValue(rapidjson::kObjectType);

	pointValue.AddMember(rapidjson::StringRef(JSON_X_POSITION_PROPERTY_NAME), rapidjson::Value(x), allocator);
	pointValue.AddMember(rapidjson::StringRef(JSON_Y_POSITION_PROPERTY_NAME), rapidjson::Value(y), allocator);
	pointValue.AddMember(rapidjson::StringRef(JSON_Z_POSITION_PROPERTY_NAME), rapidjson::Value(z), allocator);

	return pointValue;
}

Point3D Point3D::parseFrom(const rapidjson::Value & pointValue, bool * error) {
	if(!pointValue.IsObject()) {
		spdlog::error("Invalid point type: '{}', expected 'object'.", Utilities::typeToString(pointValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse x position
	if(!pointValue.HasMember(JSON_X_POSITION_PROPERTY_NAME)) {
		spdlog::error("Point is missing '{}' property.", JSON_X_POSITION_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & xPositionValue = pointValue[JSON_X_POSITION_PROPERTY_NAME];

	if(!xPositionValue.IsInt()) {
		spdlog::error("Point has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_X_POSITION_PROPERTY_NAME, Utilities::typeToString(xPositionValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse y position
	if(!pointValue.HasMember(JSON_Y_POSITION_PROPERTY_NAME)) {
		spdlog::error("Point is missing '{}' property.", JSON_Y_POSITION_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & yPositionValue = pointValue[JSON_Y_POSITION_PROPERTY_NAME];

	if(!yPositionValue.IsInt()) {
		spdlog::error("Point has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_Y_POSITION_PROPERTY_NAME, Utilities::typeToString(yPositionValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse z position
	if(!pointValue.HasMember(JSON_Z_POSITION_PROPERTY_NAME)) {
		spdlog::error("Point is missing '{}' property.", JSON_Z_POSITION_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & zPositionValue = pointValue[JSON_Z_POSITION_PROPERTY_NAME];

	if(!zPositionValue.IsInt()) {
		spdlog::error("Point has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_Z_POSITION_PROPERTY_NAME, Utilities::typeToString(zPositionValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return Point3D(xPositionValue.GetInt(), yPositionValue.GetInt(), zPositionValue.GetInt());
}

std::optional<Point3D> Point3D::parseFrom(const rapidjson::Value & pointValue) {
	bool error = false;

	Point3D point(parseFrom(pointValue, &error));

	if(error) {
		return {};
	}

	return point;
}

std::string Point3D::toString() const {
	return fmt::format("{}, {}, {}", x, y, z);
}
