#include "Dimension.h"

#include "Utilities/StringUtilities.h"

#include <regex>

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
	return std::to_string(w) + ", " + std::to_string(h);
}

Dimension Dimension::parseFrom(const std::string & data, bool * error) {
	static const std::regex     nonIntegerRegExp("[^-0-9]+");
	static const std::regex nonIntegerTrimRegExp("(^[^-0-9]+)|([^-0-9]+$)");

	std::string trimmedData;
	std::regex_replace(std::back_inserter(trimmedData), data.begin(), data.end(), nonIntegerTrimRegExp, "");

	std::string formattedData;
	std::regex_replace(std::back_inserter(formattedData), trimmedData.begin(), trimmedData.end(), nonIntegerRegExp, " ");

	size_t index = 0, start = -1, end = -1;
	std::string part;
	bool success = false;
	Dimension newDimension;
	for(size_t i = 0; i < formattedData.length(); i++) {
		if(formattedData[i] == ' ' || i == formattedData.length() - 1) {
			if(index > 1) {
				if(error != nullptr) { *error = true; }
				return Zero;
			}

			start = end + 1;
			end = i == formattedData.length() - 1 ? i + 1 : i;

			part = Utilities::substring(formattedData, start, end);

			newDimension.d[index++] = Utilities::parseInteger(part, &success);

			if(!success) {
				if(error != nullptr) { *error = true; }
				return Zero;
			}
		}
	}

	if(index != 2) {
		if(error != nullptr) { *error = false; }
		return Zero;
	}

	return newDimension;
}

uint32_t Dimension::operator [] (size_t index) const {
	if(index < 0 || index > 1) {
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
