#ifndef _DIMENSION_H_
#define _DIMENSION_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <optional>
#include <string>

class Dimension final {
public:
	Dimension(uint32_t w = 0, uint32_t h = 0);
	Dimension(const uint32_t d[2]);
	Dimension(uint64_t packedDimension);
	Dimension(const Dimension & dimension);
	Dimension & operator = (const Dimension & dimension);
	~Dimension();

	bool isEmpty() const;
	bool isNotEmpty() const;
	uint64_t getPerimeter() const;
	uint64_t getArea() const;

	void setDimension(uint32_t width, uint32_t height);
	void setDimension(const uint32_t dimension[2]);
	void setDimension(uint64_t packedDimension);
	void setDimension(const Dimension & dimension);

	uint64_t pack() const;
	static Dimension unpack(uint64_t packedDimension);

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static Dimension parseFrom(const rapidjson::Value & dimensionValue, bool * error);
	static std::optional<Dimension> parseFrom(const rapidjson::Value & dimensionValue);
	std::string toString() const;

	uint32_t operator [] (size_t index) const;

	bool operator == (const Dimension & dimension) const;
	bool operator != (const Dimension & dimension) const;

	static const Dimension Zero;

	union {
		struct {
			uint32_t w, h;
		};

		uint32_t d[2];
	};
};

#endif // _DIMENSION_H_
