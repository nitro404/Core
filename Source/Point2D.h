#ifndef _POINT_H_
#define _POINT_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <optional>
#include <string>

class Point2D final {
public:
	Point2D(int32_t x = 0, int32_t y = 0);
	Point2D(const int32_t p[2]);
	Point2D(uint64_t packedPoint);
	Point2D(const Point2D & point);
	Point2D & operator = (const Point2D & point);
	~Point2D();

	void setPoint(int32_t xPos, int32_t yPos);
	void setPoint(const int32_t point[2]);
	void setPoint(uint64_t packedPoint);
	void setPoint(const Point2D & point);

	uint64_t pack() const;
	static Point2D unpack(uint64_t packedPoint);

	float distanceBetween(const Point2D & point) const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static Point2D parseFrom(const rapidjson::Value & pointValue, bool * error);
	static std::optional<Point2D> parseFrom(const rapidjson::Value & pointValue);
	std::string toString() const;

	bool operator == (const Point2D & point) const;
	bool operator != (const Point2D & point) const;

	static const Point2D Zero;

	int32_t operator [] (size_t index) const;

	union {
		struct {
			int32_t x, y;
		};

		int32_t p[2];
	};
};

#endif // _POINT_H_
