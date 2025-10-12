#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

#include "Dimension.h"
#include "Point2D.h"

#include <rapidjson/document.h>

#include <cstdint>
#include <optional>
#include <string>

class Vector2;

class Rect final {
public:
	Rect(int32_t xPos = 0, int32_t yPos = 0, uint32_t width = 0, uint32_t height = 0);
	Rect(const int32_t p[2], const uint32_t d[2]);
	Rect(int32_t xPos, int32_t yPos, const Dimension & dimensions);
	Rect(const Point2D & position, uint32_t width = 0, uint32_t height = 0);
	Rect(uint64_t packedPosition, uint64_t packedDimension);
	Rect(const Rect & rectangle);
	Rect & operator = (const Rect & rectangle);
	~Rect();

	bool isEmpty() const;
	bool isNotEmpty() const;
	uint64_t getPerimeter() const;
	uint64_t getArea() const;

	Point2D getPosition() const;
	Dimension getDimensions() const;

	Point2D getCenter() const;

	Point2D getLeft() const;
	Point2D getRight() const;
	Point2D getTop() const;
	Point2D getBottom() const;

	Point2D getTopLeft() const;
	Point2D getTopRight() const;
	Point2D getBottomLeft() const;
	Point2D getBottomRight() const;

	bool contains(const Point2D & point) const;
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
			int32_t x;
			int32_t y;
			uint32_t w;
			uint32_t h;
		};

		struct {
			int32_t p[2];
			uint32_t d[2];
		};
	};
};

#endif // _RECTANGLE_H_
