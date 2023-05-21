#include "Point2D.h"

#include "ByteBuffer.h"
#include "Point3D.h"
#include "Utilities/RapidJSONUtilities.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <cmath>

static constexpr const char * JSON_X_POSITION_PROPERTY_NAME = "x";
static constexpr const char * JSON_Y_POSITION_PROPERTY_NAME = "y";

const Point2D Point2D::ZERO(0, 0);

Point2D::Point2D()
	: x(0)
	, y(0) { }

Point2D::Point2D(int32_t a, int32_t b)
	: x(a)
	, y(b) { }

Point2D::Point2D(const int32_t p[2])
	: x(p[0])
	, y(p[1]) { }

Point2D::Point2D(uint64_t packedPoint)
	: x(static_cast<int32_t>((packedPoint >> 32) & 0xffff))
	, y(static_cast<int32_t>( packedPoint        & 0xffff)) { }

Point2D::Point2D(const Point2D & point)
	: x(point.x)
	, y(point.y) { }

Point2D & Point2D::operator = (const Point2D & point) {
	x = point.x;
	y = point.y;

	return *this;
}

Point2D::~Point2D() = default;

Point2D Point2D::operator  + (int32_t c)         const { return Point2D(x + c,   y + c); }
Point2D Point2D::operator  + (const Point2D & p) const { return Point2D(x + p.x, y + p.y); }
Point2D Point2D::operator  + (const Point3D & p) const { return Point2D(x + p.x, y + p.y); }

void Point2D::operator    += (int32_t c)               { x += c;   y += c; }
void Point2D::operator    += (const Point2D & p)       { x += p.x; y += p.y; }
void Point2D::operator    += (const Point3D & p)       { x += p.x; y += p.y; }

Point2D Point2D::operator  - ()                  const { return Point2D(-x, -y); }

Point2D Point2D::operator  - (int32_t c)         const { return Point2D(x - c,   y - c); }
Point2D Point2D::operator  - (const Point2D & p) const { return Point2D(x - p.x, y - p.y); }
Point2D Point2D::operator  - (const Point3D & p) const { return Point2D(x - p.x, y - p.y); }

void Point2D::operator    -= (int32_t c)               { x -= c;   y -= c; }
void Point2D::operator    -= (const Point2D & p)       { x -= p.x; y -= p.y; }
void Point2D::operator    -= (const Point3D & p)       { x -= p.x; y -= p.y; }

Point2D Point2D::operator  * (double c)          const { return Point2D(x * c,   y * c); }
Point2D Point2D::operator  * (const Point2D & p) const { return Point2D(x * p.x, y * p.y); }
Point2D Point2D::operator  * (const Point3D & p) const { return Point2D(x * p.x, y * p.y); }

void Point2D::operator    *= (double c)                { x *= c;   y *= c; }
void Point2D::operator    *= (const Point2D & p)       { x *= p.x; y *= p.y; }
void Point2D::operator    *= (const Point3D & p)       { x *= p.x; y *= p.y; }

Point2D Point2D::operator  / (double c)          const { return Point2D(  c == 0.0 ? 0.0 : x / c,     c == 0.0 ? 0.0 : y / c); }
Point2D Point2D::operator  / (const Point2D & p) const { return Point2D(p.x == 0   ? 0   : x / p.x, p.y == 0   ? 0   : y / p.y); }
Point2D Point2D::operator  / (const Point3D & p) const { return Point2D(p.x == 0   ? 0   : x / p.x, p.y == 0   ? 0   : y / p.y); }

void Point2D::operator    /= (double c)                { x =   c == 0.0 ? 0.0 : x / c;   y =   c == 0.0 ? 0.0 : y / c; }
void Point2D::operator    /= (const Point2D & p)       { x = p.x == 0   ? 0   : x / p.x; y = p.y == 0   ? 0   : y / p.y; }
void Point2D::operator    /= (const Point3D & p)       { x = p.x == 0   ? 0   : x / p.x; y = p.y == 0   ? 0   : y / p.y; }

int32_t Point2D::operator [] (size_t index) const {
	if(index > 1) {
		return 0;
	}

	return p[index];
}

bool Point2D::operator == (const Point2D & point) const {
	return x == point.x &&
		   y == point.y;
}

bool Point2D::operator != (const Point2D & point) const {
	return !operator == (point);
}

void Point2D::setPoint(int32_t xPos, int32_t yPos) {
	x = xPos;
	y = yPos;
}

void Point2D::setPoint(const int32_t point[2]) {
	x = point[0];
	y = point[1];
}

void Point2D::setPoint(uint64_t packedPoint) {
	x = static_cast<int32_t>((packedPoint >> 32) & 0xffff);
	y = static_cast<int32_t>( packedPoint        & 0xffff);
}

void Point2D::setPoint(const Point2D & point) {
	x = point.x;
	y = point.y;
}

uint64_t Point2D::pack() const {
	return static_cast<uint64_t>(x) << 32 | static_cast<uint64_t>(y);
}

Point2D Point2D::unpack(uint64_t packedPoint) {
	return Point2D(packedPoint);
}

float Point2D::distanceBetween(const Point2D & point) const {
	return sqrt(pow(point.x - x, 2) + pow(point.y - y, 2));
}

Point2D Point2D::getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error) {
	if(offset + (sizeof(int32_t) * 2) > byteBuffer.getSize()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool internalError = false;

	Point2D value;

	for(size_t i = 0; i < 2; i++) {
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

std::optional<Point2D> Point2D::getFrom(const ByteBuffer & byteBuffer, size_t offset) {
	bool error = false;

	Point2D value = getFrom(byteBuffer, offset, &error);

	if(error) {
		return {};
	}

	return value;
}

Point2D Point2D::readFrom(const ByteBuffer & byteBuffer, bool * error) {
	bool internalError = false;

	Point2D value(getFrom(byteBuffer, byteBuffer.getReadOffset(), error));

	if(internalError) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		byteBuffer.skipReadBytes(sizeof(int32_t) * 2);
	}

	return value;
}

std::optional<Point2D> Point2D::readFrom(const ByteBuffer & byteBuffer) {
	bool error = false;

	Point2D value(readFrom(byteBuffer, &error));

	if(error) {
		return {};
	}

	return value;
}

bool Point2D::putIn(ByteBuffer & byteBuffer, size_t offset) const {
	for(size_t i = 0; i < 2; i++) {
		if(!byteBuffer.putInteger(p[i], offset + (sizeof(int32_t) * i))) {
			return false;
		}
	}

	return true;
}

bool Point2D::insertIn(ByteBuffer & byteBuffer, size_t offset) const {
	for(size_t i = 0; i < 2; i++) {
		if(!byteBuffer.insertInteger(p[i], offset + (sizeof(int32_t) * i))) {
			return false;
		}
	}

	return true;
}

bool Point2D::writeTo(ByteBuffer & byteBuffer) const {
	for(size_t i = 0; i < 2; i++) {
		if(!byteBuffer.putUnsignedLong(p[i], byteBuffer.getWriteOffset())) {
			return false;
		}

		byteBuffer.skipWriteBytes(sizeof(int32_t));
	}

	return true;
}

rapidjson::Value Point2D::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value pointValue(rapidjson::kObjectType);

	pointValue.AddMember(rapidjson::StringRef(JSON_X_POSITION_PROPERTY_NAME), rapidjson::Value(x), allocator);
	pointValue.AddMember(rapidjson::StringRef(JSON_Y_POSITION_PROPERTY_NAME), rapidjson::Value(y), allocator);

	return pointValue;
}

Point2D Point2D::parseFrom(const rapidjson::Value & pointValue, bool * error) {
	if(!pointValue.IsObject()) {
		spdlog::error("Invalid point type: '{}', expected 'object'.", Utilities::typeToString(pointValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse x position
	if(!pointValue.HasMember(JSON_X_POSITION_PROPERTY_NAME)) {
		spdlog::error("Point2D is missing '{}' property'.", JSON_X_POSITION_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & xPositionValue = pointValue[JSON_X_POSITION_PROPERTY_NAME];

	if(!xPositionValue.IsInt()) {
		spdlog::error("Point2D has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_X_POSITION_PROPERTY_NAME, Utilities::typeToString(xPositionValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse y position
	if(!pointValue.HasMember(JSON_Y_POSITION_PROPERTY_NAME)) {
		spdlog::error("Point2D is missing '{}' property'.", JSON_Y_POSITION_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & yPositionValue = pointValue[JSON_Y_POSITION_PROPERTY_NAME];

	if(!yPositionValue.IsInt()) {
		spdlog::error("Point2D has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_Y_POSITION_PROPERTY_NAME, Utilities::typeToString(yPositionValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return Point2D(xPositionValue.GetInt(), yPositionValue.GetInt());
}

std::optional<Point2D> Point2D::parseFrom(const rapidjson::Value & pointValue) {
	bool error = false;

	Point2D point(parseFrom(pointValue, &error));

	if(error) {
		return {};
	}

	return point;
}

std::string Point2D::toString() const {
	return fmt::format("{}, {}", x, y);
}
