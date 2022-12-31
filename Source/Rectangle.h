#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

#include "Dimension.h"
#include "Point.h"

#include <rapidjson/document.h>

#include <cstdint>
#include <optional>
#include <string>

class Point;
class Dimension;
class Vector2;

class Rect final {
public:
	Rect(int32_t xPos = 0, int32_t yPos = 0, uint32_t width = 0, uint32_t height = 0);
	Rect(const int32_t p[2], const uint32_t d[2]);
	Rect(int32_t xPos, int32_t yPos, const Dimension & dimensions);
	Rect(const Point & position, uint32_t width = 0, uint32_t height = 0);
	Rect(uint64_t packedPosition, uint64_t packedDimension);
	Rect(const Rect & rectangle);
	Rect & operator = (const Rect & rectangle);
	~Rect();

	bool isEmpty() const;
	bool isNotEmpty() const;
	uint64_t getPerimeter() const;
	uint64_t getArea() const;

	Point getPosition() const;
	Dimension getDimensions() const;

	Point getCenter() const;

	Point getLeft() const;
	Point getRight() const;
	Point getTop() const;
	Point getBottom() const;

	Point getTopLeft() const;
	Point getTopRight() const;
	Point getBottomLeft() const;
	Point getBottomRight() const;

	bool contains(const Point & point) const;
	bool contains(const Vector2 & point) const;
	bool contains(const Rect & rectangle) const;

	bool intersects(const Rect & rectangle) const;
	Rect intersection(const Rect & rectangle) const;

	Rect unioned(const Rect & rectangle) const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static Rect parseFrom(const rapidjson::Value & rectangleValue, bool * error);
	static std::optional<Rect> parseFrom(const rapidjson::Value & rectangleValue);
	std::string toString() const;

	bool operator == (const Rect & rectangle) const;
	bool operator != (const Rect & rectangle) const;

	static const Rect Zero;

	union {
		struct {
			Point position;
			Dimension size;
		};

		struct {
			int32_t x, y;
			uint32_t w, h;
		};

		struct {
			int32_t p[2];
			uint32_t d[2];
		};
	};
};

#endif // _RECTANGLE_H_
