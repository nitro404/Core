#ifndef _POINT_H_
#define _POINT_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <optional>
#include <string>

class ByteBuffer;
class Point3D;

class Point2D final {
public:
	Point2D();
	Point2D(int32_t a, int32_t b);
	Point2D(const int32_t p[2]);
	Point2D(uint64_t packedPoint);
	Point2D(const Point2D & point);
	Point2D & operator = (const Point2D & point);
	~Point2D();

	Point2D operator   + (int32_t c)         const;
	Point2D operator   + (const Point2D & p) const;
	Point2D operator   + (const Point3D & p) const;

	void operator     += (int32_t c);
	void operator     += (const Point2D & p);
	void operator     += (const Point3D & p);

	Point2D operator   - ()                  const;

	Point2D operator   - (int32_t c)         const;
	Point2D operator   - (const Point2D & p) const;
	Point2D operator   - (const Point3D & p) const;

	void operator     -= (int32_t c);
	void operator     -= (const Point2D & p);
	void operator     -= (const Point3D & p);

	Point2D operator   * (double c)          const;
	Point2D operator   * (const Point2D & p) const;
	Point2D operator   * (const Point3D & p) const;

	void operator     *= (double c);
	void operator     *= (const Point2D & p);
	void operator     *= (const Point3D & p);

	Point2D operator   / (double c)          const;
	Point2D operator   / (const Point2D & p) const;
	Point2D operator   / (const Point3D & p) const;

	void operator     /= (double c);
	void operator     /= (const Point2D & p);
	void operator     /= (const Point3D & p);

	int32_t operator  [] (size_t index)      const;

	bool operator     == (const Point2D & p) const;
	bool operator     != (const Point2D & p) const;

	void setPoint(int32_t xPos, int32_t yPos);
	void setPoint(const int32_t point[2]);
	void setPoint(uint64_t packedPoint);
	void setPoint(const Point2D & point);

	uint64_t pack() const;
	static Point2D unpack(uint64_t packedPoint);

	float distanceBetween(const Point2D & point) const;

	static Point2D getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error);
	static std::optional<Point2D> getFrom(const ByteBuffer & byteBuffer, size_t offset);
	static Point2D readFrom(const ByteBuffer & byteBuffer, bool * error);
	static std::optional<Point2D> readFrom(const ByteBuffer & byteBuffer);
	bool putIn(ByteBuffer & byteBuffer, size_t offset) const;
	bool insertIn(ByteBuffer & byteBuffer, size_t offset) const;
	bool writeTo(ByteBuffer & byteBuffer) const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static Point2D parseFrom(const rapidjson::Value & pointValue, bool * error);
	static std::optional<Point2D> parseFrom(const rapidjson::Value & pointValue);
	std::string toString() const;

	static constexpr size_t SIZE_BYTES = (sizeof(int32_t) * 2);
	static const Point2D ZERO;

	union {
		struct {
			int32_t x, y;
		};

		int32_t p[2];
	};
};

#endif // _POINT_H_
