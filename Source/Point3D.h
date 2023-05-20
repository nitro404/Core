#ifndef _POINT_3D_H_
#define _POINT_3D_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <optional>
#include <string>

class ByteBuffer;
class Point2D;

class Point3D final {
public:
	Point3D();
	Point3D(int32_t a, int32_t b, int32_t c);
	Point3D(int32_t p[3]);
	Point3D(const Point3D & p);
	Point3D & operator = (const Point3D & p);
	~Point3D();

	Point3D operator   + (int32_t c)         const;
	Point3D operator   + (const Point2D & p) const;
	Point3D operator   + (const Point3D & p) const;

	void operator     += (int32_t c);
	void operator     += (const Point2D & p);
	void operator     += (const Point3D & p);

	Point3D operator   - ()                  const;

	Point3D operator   - (int32_t c)         const;
	Point3D operator   - (const Point2D & p) const;
	Point3D operator   - (const Point3D & p) const;

	void operator     -= (int32_t c);
	void operator     -= (const Point2D & p);
	void operator     -= (const Point3D & p);

	Point3D operator   * (double c)          const;
	Point3D operator   * (const Point2D & p) const;
	Point3D operator   * (const Point3D & p) const;

	void operator     *= (double c);
	void operator     *= (const Point2D & p);
	void operator     *= (const Point3D & p);

	Point3D operator   / (double c)          const;
	Point3D operator   / (const Point2D & p) const;
	Point3D operator   / (const Point3D & p) const;

	void operator     /= (double c);
	void operator     /= (const Point2D & p);
	void operator     /= (const Point3D & p);

	int32_t operator  [] (size_t index)      const;

	bool operator     == (const Point3D & p) const;
	bool operator     != (const Point3D & p) const;

	void setPoint(int32_t a, int32_t b, int32_t c);
	void setPoint(const int32_t p[3]);

	double distanceBetween(const Point3D & p) const;

	static Point3D getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error);
	static std::optional<Point3D> getFrom(const ByteBuffer & byteBuffer, size_t offset);
	static Point3D readFrom(const ByteBuffer & byteBuffer, bool * error);
	static std::optional<Point3D> readFrom(const ByteBuffer & byteBuffer);
	bool putIn(ByteBuffer & byteBuffer, size_t offset) const;
	bool insertIn(ByteBuffer & byteBuffer, size_t offset) const;
	bool writeTo(ByteBuffer & byteBuffer) const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static Point3D parseFrom(const rapidjson::Value & pointValue, bool * error);
	static std::optional<Point3D> parseFrom(const rapidjson::Value & pointValue);
	std::string toString() const;

	static const Point3D ZERO;

	union {
		struct {
			int32_t x, y, z;
		};

		int32_t p[3];
	};
};

#endif // _POINT_3D_H_
