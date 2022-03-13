#include "Dimension.h"

#include <fmt/core.h>

const Dimension Dimension::Zero(0, 0);

Dimension::Dimension(uint32_t width, uint32_t height)
	: w(width)
	, h(height) { }

Dimension::Dimension(const uint32_t d[2])
	: w(d[0])
	, h(d[1]) { }

Dimension::Dimension(uint64_t packedDimension)
	: w(static_cast<uint32_t>((packedDimension >> 32) & 0xffff))
	, h(static_cast<uint32_t>( packedDimension        & 0xffff)) { }

Dimension::Dimension(const Dimension & dimension)
	: w(dimension.w)
	, h(dimension.h) { }

Dimension & Dimension::operator = (const Dimension & dimension) {
	w = dimension.w;
	h = dimension.h;

	return *this;
}

Dimension::~Dimension() = default;

bool Dimension::isEmpty() const {
	return w == 0 || h == 0;
}

bool Dimension::isNotEmpty() const {
	return w != 0 && h != 0;
}

uint64_t Dimension::getPerimeter() const {
	return 2 * (w + h);
}

uint64_t Dimension::getArea() const {
	return w * h;
}

void Dimension::setDimension(uint32_t width, uint32_t height) {
	w = width;
	h = height;
}

void Dimension::setDimension(const uint32_t dimension[2]) {
	w = dimension[0];
	h = dimension[1];
}

void Dimension::setDimension(uint64_t packedDimension) {
	w = static_cast<uint32_t>((packedDimension >> 32) & 0xffff);
	h = static_cast<uint32_t>( packedDimension        & 0xffff);
}

void Dimension::setDimension(const Dimension & dimension) {
	w = dimension.w;
	h = dimension.h;
}

uint64_t Dimension::pack() const {
	return static_cast<uint64_t>(w) << 32 | static_cast<uint64_t>(h);
}

Dimension Dimension::unpack(uint64_t packedDimension) {
	return Dimension(packedDimension);
}

std::string Dimension::toString() const {
	return fmt::format("{}, {}", w, h);
}

uint32_t Dimension::operator [] (size_t index) const {
	if(index > 1) {
		return 0;
	}

	return d[index];
}

bool Dimension::operator == (const Dimension & dimension) const {
	return w == dimension.w &&
		   h == dimension.h;
}

bool Dimension::operator != (const Dimension & dimension) const {
	return !operator == (dimension);
}
