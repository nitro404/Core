#include "Point.h"

#include "Utilities/StringUtilities.h"

#include <cmath>
#include <regex>

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

std::string Point::toString() const {
	return std::to_string(x) + ", " + std::to_string(y);
}

Point Point::parseFrom(const std::string & data, bool * error) {
	static const std::regex     nonIntegerRegExp("[^-0-9]+");
	static const std::regex nonIntegerTrimRegExp("(^[^-0-9]+)|([^-0-9]+$)");

	std::string trimmedData;
	std::regex_replace(std::back_inserter(trimmedData), data.begin(), data.end(), nonIntegerTrimRegExp, "");

	std::string formattedData;
	std::regex_replace(std::back_inserter(formattedData), trimmedData.begin(), trimmedData.end(), nonIntegerRegExp, " ");

	size_t index = 0, start = -1, end = -1;
	std::string part;
	bool success = false;
	Point newPoint;
	for(int i = 0; i < formattedData.length();i++) {
		if(formattedData[i] == ' ' || i == formattedData.length() - 1) {
			if(index > 1) {
				if(error != nullptr) { *error = true; }
				return Zero;
			}

			start = end + 1;
			end = i == formattedData.length() - 1 ? i + 1 : i;

			part = Utilities::substring(formattedData, start, end);

			newPoint.p[index++] = Utilities::parseInteger(part, &success);

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

	return newPoint;
}

int32_t Point::operator [] (size_t index) const {
	if(index < 0 || index > 1) {
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
