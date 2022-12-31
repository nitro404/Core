#include "Point.h"

#include "Utilities/RapidJSONUtilities.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <cmath>

static constexpr const char * JSON_X_POSITION_PROPERTY_NAME = "x";
static constexpr const char * JSON_Y_POSITION_PROPERTY_NAME = "y";

const Point Point::Zero(0, 0);

Point::Point(int32_t xPos, int32_t yPos)
	: x(xPos)
	, y(yPos) { }

Point::Point(const int32_t p[2])
	: x(p[0])
	, y(p[1]) { }

Point::Point(uint64_t packedPoint)
	: x(static_cast<int32_t>((packedPoint >> 32) & 0xffff))
	, y(static_cast<int32_t>( packedPoint        & 0xffff)) { }

Point::Point(const Point & point)
	: x(point.x)
	, y(point.y) { }

Point & Point::operator = (const Point & point) {
	x = point.x;
	y = point.y;

	return *this;
}

Point::~Point() = default;

void Point::setPoint(int32_t xPos, int32_t yPos) {
	x = xPos;
	y = yPos;
}

void Point::setPoint(const int32_t point[2]) {
	x = point[0];
	y = point[1];
}

void Point::setPoint(uint64_t packedPoint) {
	x = static_cast<int32_t>((packedPoint >> 32) & 0xffff);
	y = static_cast<int32_t>( packedPoint        & 0xffff);
}

void Point::setPoint(const Point & point) {
	x = point.x;
	y = point.y;
}

uint64_t Point::pack() const {
	return static_cast<uint64_t>(x) << 32 | static_cast<uint64_t>(y);
}

Point Point::unpack(uint64_t packedPoint) {
	return Point(packedPoint);
}

float Point::distanceBetween(const Point & point) const {
	return sqrt(pow(point.x - x, 2) + pow(point.y - y, 2));
}

rapidjson::Value Point::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value pointValue(rapidjson::kObjectType);

	pointValue.AddMember(rapidjson::StringRef(JSON_X_POSITION_PROPERTY_NAME), rapidjson::Value(x), allocator);
	pointValue.AddMember(rapidjson::StringRef(JSON_Y_POSITION_PROPERTY_NAME), rapidjson::Value(y), allocator);

	return pointValue;
}

Point Point::parseFrom(const rapidjson::Value & pointValue, bool * error) {
	if(!pointValue.IsObject()) {
		spdlog::error("Invalid point type: '{}', expected 'object'.", Utilities::typeToString(pointValue.GetType()));
		return {};
	}

	// parse x position
	if(!pointValue.HasMember(JSON_X_POSITION_PROPERTY_NAME)) {
		spdlog::error("Point is missing '{}' property'.", JSON_X_POSITION_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & xPositionValue = pointValue[JSON_X_POSITION_PROPERTY_NAME];

	if(!xPositionValue.IsInt()) {
		spdlog::error("Point has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_X_POSITION_PROPERTY_NAME, Utilities::typeToString(xPositionValue.GetType()));
		return {};
	}

	// parse y position
	if(!pointValue.HasMember(JSON_Y_POSITION_PROPERTY_NAME)) {
		spdlog::error("Point is missing '{}' property'.", JSON_Y_POSITION_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & yPositionValue = pointValue[JSON_Y_POSITION_PROPERTY_NAME];

	if(!yPositionValue.IsInt()) {
		spdlog::error("Point has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_Y_POSITION_PROPERTY_NAME, Utilities::typeToString(yPositionValue.GetType()));
		return {};
	}

	return Point(xPositionValue.GetInt(), yPositionValue.GetInt());
}

std::optional<Point> Point::parseFrom(const rapidjson::Value & pointValue) {
	bool error = false;

	Point point(parseFrom(pointValue, &error));

	if(error) {
		return {};
	}

	return point;
}

std::string Point::toString() const {
	return fmt::format("{}, {}", x, y);
}

int32_t Point::operator [] (size_t index) const {
	if(index > 1) {
		return 0;
	}

	return p[index];
}

bool Point::operator == (const Point & point) const {
	return x == point.x &&
		   y == point.y;
}

bool Point::operator != (const Point & point) const {
	return !operator == (point);
}
