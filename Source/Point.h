#ifndef _POINT_H_
#define _POINT_H_

#include <cstdint>
#include <string>

class Point final {
public:
	Point(int32_t x = 0, int32_t y = 0);
	Point(const int32_t p[2]);
	Point(uint64_t packedPoint);
	Point(const Point & point);
	Point & operator = (const Point & point);
	~Point();

	void setPoint(int32_t xPos, int32_t yPos);
	void setPoint(const int32_t point[2]);
	void setPoint(uint64_t packedPoint);
	void setPoint(const Point & point);

	uint64_t pack() const;
	static Point unpack(uint64_t packedPoint);

	float distanceBetween(const Point & point) const;

	std::string toString() const;

	bool operator == (const Point & point) const;
	bool operator != (const Point & point) const;

	static const Point Zero;

	int32_t operator [] (size_t index) const;

	union {
		struct {
			int32_t x, y;
		};

		int32_t p[2];
	};
};

#endif // _POINT_H_
