#include "Colour.h"

#include "ByteBuffer.h"
#include "Math/ExtendedMath.h"

#include <fmt/core.h>

const Colour::ByteOrder Colour::DEFAULT_BYTE_ORDER = Colour::ByteOrder::RGBA;

Colour::Colour(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
	: r(red)
	, g(green)
	, b(blue)
	, a(alpha) { }

Colour::Colour(const uint8_t colour[3], uint8_t alpha, ByteOrder byteOrder)
	: r(colour[0])
	, g(colour[1])
	, b(colour[2])
	, a(alpha) {
	switch(byteOrder) {
		case Colour::ByteOrder::RGBA:
		case Colour::ByteOrder::ARGB: {
			break;
		}

		case Colour::ByteOrder::BGRA:
		case Colour::ByteOrder::ABGR: {
			r = colour[2];
			g = colour[1];
			b = colour[0];
			a = alpha;
			break;
		}
	}
}

Colour::Colour(const uint8_t colour[4], ByteOrder byteOrder)
	: r(colour[0])
	, g(colour[1])
	, b(colour[2])
	, a(colour[3]) {
	switch(byteOrder) {
		case Colour::ByteOrder::RGBA: {
			break;
		}

		case Colour::ByteOrder::BGRA: {
			r = colour[2];
			g = colour[1];
			b = colour[0];
			a = colour[3];
			break;
		}

		case Colour::ByteOrder::ARGB: {
			r = colour[1];
			g = colour[2];
			b = colour[3];
			a = colour[0];
			break;
		}

		case Colour::ByteOrder::ABGR: {
			r = colour[3];
			g = colour[2];
			b = colour[1];
			a = colour[0];
			break;
		}
	}
}

Colour::Colour(uint32_t packedColour, Colour::ByteOrder byteOrder)
	: r(0)
	, g(0)
	, b(0)
	, a(0) {
	setColour(packedColour, byteOrder);
}

Colour::Colour(const Colour & c, uint8_t a)
	: r(c.r)
	, g(c.g)
	, b(c.b)
	, a(a) { }

Colour::Colour(const Colour & c)
	: r(c.r)
	, g(c.g)
	, b(c.b)
	, a(c.a) { }

Colour & Colour::operator = (const Colour & c) {
	setColour(c);

	return *this;
}

Colour::~Colour() = default;

bool Colour::isVisible() const {
	return a != 0;
}

void Colour::setColour(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
	r = red;
	g = green;
	b = blue;
	a = alpha;
}

void Colour::setColour(const uint8_t colour[3], uint8_t alpha) {
	r = colour[0];
	g = colour[1];
	b = colour[2];
	a = alpha;
}

void Colour::setColour(const uint8_t colour[4]) {
	r = colour[0];
	g = colour[1];
	b = colour[2];
	a = colour[3];
}

void Colour::setColour(uint32_t packedColour, ByteOrder byteOrder) {
	switch(byteOrder) {
		case Colour::ByteOrder::RGBA: {
			r = static_cast<uint8_t>((packedColour >> 24) & 0xff);
			g = static_cast<uint8_t>((packedColour >> 16) & 0xff);
			b = static_cast<uint8_t>((packedColour >>  8) & 0xff);
			a = static_cast<uint8_t>((packedColour      ) & 0xff);
			break;
		}

		case Colour::ByteOrder::BGRA: {
			b = static_cast<uint8_t>((packedColour >> 24) & 0xff);
			g = static_cast<uint8_t>((packedColour >> 16) & 0xff);
			r = static_cast<uint8_t>((packedColour >>  8) & 0xff);
			a = static_cast<uint8_t>((packedColour      ) & 0xff);
			break;
		}

		case Colour::ByteOrder::ARGB: {
			a = static_cast<uint8_t>((packedColour >> 24) & 0xff);
			r = static_cast<uint8_t>((packedColour >> 16) & 0xff);
			g = static_cast<uint8_t>((packedColour >>  8) & 0xff);
			b = static_cast<uint8_t>((packedColour      ) & 0xff);
			break;
		}

		case Colour::ByteOrder::ABGR: {
			a = static_cast<uint8_t>((packedColour >> 24) & 0xff);
			b = static_cast<uint8_t>((packedColour >> 16) & 0xff);
			g = static_cast<uint8_t>((packedColour >>  8) & 0xff);
			r = static_cast<uint8_t>((packedColour      ) & 0xff);
			break;
		}
	}
}

void Colour::setColour(const Colour & c, uint8_t alpha) {
	r = c.r;
	g = c.g;
	b = c.b;
	a = alpha;
}

void Colour::setColour(const Colour & c) {
	r = c.r;
	g = c.g;
	b = c.b;
	a = c.a;
}

uint32_t Colour::pack(Colour::ByteOrder byteOrder) const {
	switch(byteOrder) {
		case Colour::ByteOrder::RGBA:
			return r << 24 | g << 16 | b << 8 | a;

		case Colour::ByteOrder::BGRA:
			return b << 24 | g << 16 | r << 8 | a;

		case Colour::ByteOrder::ARGB:
			return a << 24 | r << 16 | g << 8 | b;

		case Colour::ByteOrder::ABGR:
			return a << 24 | b << 16 | g << 8 | r;
	}

	return 0;
}

Colour Colour::unpack(uint32_t packedColour, ByteOrder byteOrder) {
	return Colour(packedColour, byteOrder);
}

void Colour::scale(float factor, bool alpha) {
	     if(factor <  0.0f) { return; }
	else if(factor == 0.0f) {
		r = g = b = 0;

		if(alpha) {
			a = 0;
		}
	}

	float scaledRed   = static_cast<float>(r) * factor;
	float scaledGreen = static_cast<float>(g) * factor;
	float scaledBlue  = static_cast<float>(b) * factor;
	float scaledAlpha = static_cast<float>(a) * factor;

	r = static_cast<uint8_t>(Math::clamp(scaledRed, std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max()));
	g = static_cast<uint8_t>(Math::clamp(scaledGreen, std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max()));
	b = static_cast<uint8_t>(Math::clamp(scaledBlue, std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max()));

	if(alpha) {
		a = static_cast<uint8_t>(Math::clamp(scaledAlpha, 0, std::numeric_limits<uint8_t>::max()));
	}
}

Colour Colour::scaled(float factor, bool alpha) const {
	     if(factor <  0.0f) { return INVISIBLE; }
	else if(factor == 0.0f) {
		return Colour(0, 0, 0, alpha ? 0 : a);
	}

	float scaledRed   = Math::clamp(static_cast<float>(r) * factor, static_cast<float>(std::numeric_limits<uint8_t>::min()), static_cast<float>(std::numeric_limits<uint8_t>::max()));
	float scaledGreen = Math::clamp(static_cast<float>(g) * factor, static_cast<float>(std::numeric_limits<uint8_t>::min()), static_cast<float>(std::numeric_limits<uint8_t>::max()));
	float scaledBlue  = Math::clamp(static_cast<float>(b) * factor, static_cast<float>(std::numeric_limits<uint8_t>::min()), static_cast<float>(std::numeric_limits<uint8_t>::max()));
	float scaledAlpha = Math::clamp(static_cast<float>(a) * factor, static_cast<float>(std::numeric_limits<uint8_t>::min()), static_cast<float>(std::numeric_limits<uint8_t>::max()));

	return Colour(static_cast<uint8_t>(scaledRed),
				  static_cast<uint8_t>(scaledGreen),
				  static_cast<uint8_t>(scaledBlue),
		  alpha ? static_cast<uint8_t>(scaledAlpha) : a);
}

void Colour::invert() {
	r = std::numeric_limits<uint8_t>::max() - r;
	g = std::numeric_limits<uint8_t>::max() - g;
	b = std::numeric_limits<uint8_t>::max() - b;
}

Colour Colour::inverted() const {
	return Colour(std::numeric_limits<uint8_t>::max() - r, std::numeric_limits<uint8_t>::max() - g, std::numeric_limits<uint8_t>::max() - b, a);
}

Colour Colour::lerp(const Colour & c, const Colour & d, float amount) {
	float lerpedRed   = Math::clamp(Math::lerp(static_cast<float>(c.r), static_cast<float>(d.r), amount), static_cast<float>(std::numeric_limits<uint8_t>::min()), static_cast<float>(std::numeric_limits<uint8_t>::max()));
	float lerpedGreen = Math::clamp(Math::lerp(static_cast<float>(c.g), static_cast<float>(d.g), amount), static_cast<float>(std::numeric_limits<uint8_t>::min()), static_cast<float>(std::numeric_limits<uint8_t>::max()));
	float lerpedBlue  = Math::clamp(Math::lerp(static_cast<float>(c.b), static_cast<float>(d.b), amount), static_cast<float>(std::numeric_limits<uint8_t>::min()), static_cast<float>(std::numeric_limits<uint8_t>::max()));
	float lerpedAlpha = Math::clamp(Math::lerp(static_cast<float>(c.a), static_cast<float>(d.a), amount), static_cast<float>(std::numeric_limits<uint8_t>::min()), static_cast<float>(std::numeric_limits<uint8_t>::max()));

	return Colour(static_cast<uint8_t>(lerpedRed),
				  static_cast<uint8_t>(lerpedGreen),
				  static_cast<uint8_t>(lerpedBlue),
				  static_cast<uint8_t>(lerpedAlpha));
}

Colour Colour::getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error) {
	return getFrom(byteBuffer, offset, true, DEFAULT_BYTE_ORDER, error);
}

Colour Colour::getFrom(const ByteBuffer & byteBuffer, size_t offset, bool alpha, bool * error) {
	return getFrom(byteBuffer, offset, alpha, DEFAULT_BYTE_ORDER, error);
}

Colour Colour::getFrom(const ByteBuffer & byteBuffer, size_t offset, bool alpha, ByteOrder byteOrder, bool * error) {
	uint8_t numberOfBytes = alpha ? 4 : 3;

	if(offset + numberOfBytes > byteBuffer.getSize()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	uint8_t colourData[4];
	colourData[3] = std::numeric_limits<uint8_t>::max();
	std::memcpy(colourData, byteBuffer.getRawData() + offset, numberOfBytes);

	return Colour(colourData, byteOrder);
}

std::optional<Colour> Colour::getFrom(const ByteBuffer & byteBuffer, size_t offset, bool alpha, ByteOrder byteOrder) {
	bool error = false;

	Colour value(getFrom(byteBuffer, offset, &error));

	if(error) {
		return {};
	}

	return value;
}

Colour Colour::readFrom(const ByteBuffer & byteBuffer, bool * error) {
	return readFrom(byteBuffer, true, DEFAULT_BYTE_ORDER, error);
}

Colour Colour::readFrom(const ByteBuffer & byteBuffer, bool alpha, bool * error) {
	return readFrom(byteBuffer, alpha, DEFAULT_BYTE_ORDER, error);
}

Colour Colour::readFrom(const ByteBuffer & byteBuffer, bool alpha, ByteOrder byteOrder, bool * error) {
	bool internalError = false;

	Colour colour(getFrom(byteBuffer, byteBuffer.getReadOffset(), alpha, byteOrder, error));

	if(internalError) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		byteBuffer.skipReadBytes(alpha ? 4 : 3);
	}

	return colour;
}

std::optional<Colour> Colour::readFrom(const ByteBuffer & byteBuffer, bool alpha, ByteOrder byteOrder) {
	bool error = false;

	Colour value(readFrom(byteBuffer, &error));

	if(error) {
		return {};
	}

	return value;
}

bool Colour::putIn(ByteBuffer & byteBuffer, size_t offset, bool alpha, ByteOrder byteOrder) const {
	switch(byteOrder) {
		case Colour::ByteOrder::RGBA: {
			uint8_t numberOfBytes = alpha ? 4 : 3;

			for(uint8_t i = 0; i < numberOfBytes; i++) {
				if(!byteBuffer.putByte(c[i], offset + i)) {
					return false;
				}
			}

			break;
		}

		case Colour::ByteOrder::BGRA: {
			if(!byteBuffer.putByte(b, offset + 1)) {
				return false;
			}

			if(!byteBuffer.putByte(g, offset + 1)) {
				return false;
			}

			if(!byteBuffer.putByte(r, offset + 2)) {
				return false;
			}

			if(alpha) {
				if(!byteBuffer.putByte(a, offset + 3)) {
					return false;
				}
			}

			break;
		}

		case Colour::ByteOrder::ARGB: {
			uint8_t newOffset = offset;

			if(alpha) {
				if(!byteBuffer.putByte(a, newOffset)) {
					return false;
				}

				newOffset++;
			}

			if(!byteBuffer.putByte(r, newOffset)) {
				return false;
			}

			newOffset++;

			if(!byteBuffer.putByte(g, newOffset)) {
				return false;
			}

			newOffset++;

			if(!byteBuffer.putByte(b, newOffset)) {
				return false;
			}

			break;
		}

		case Colour::ByteOrder::ABGR: {
			uint8_t newOffset = offset;

			if(alpha) {
				if(!byteBuffer.putByte(a, newOffset)) {
					return false;
				}

				newOffset++;
			}

			if(!byteBuffer.putByte(b, newOffset)) {
				return false;
			}

			newOffset++;

			if(!byteBuffer.putByte(g, newOffset)) {
				return false;
			}

			newOffset++;

			if(!byteBuffer.putByte(r, newOffset)) {
				return false;
			}

			break;
		}
	}

	return true;
}

bool Colour::insertIn(ByteBuffer & byteBuffer, size_t offset, bool alpha, ByteOrder byteOrder) const {
	switch(byteOrder) {
		case Colour::ByteOrder::RGBA: {
			uint8_t numberOfBytes = alpha ? 4 : 3;

			for(uint8_t i = 0; i < numberOfBytes; i++) {
				if(!byteBuffer.insertByte(c[i], offset + i)) {
					return false;
				}
			}

			break;
		}

		case Colour::ByteOrder::BGRA: {
			if(!byteBuffer.insertByte(b, offset + 1)) {
				return false;
			}

			if(!byteBuffer.insertByte(g, offset + 1)) {
				return false;
			}

			if(!byteBuffer.insertByte(r, offset + 2)) {
				return false;
			}

			if(alpha) {
				if(!byteBuffer.insertByte(a, offset + 3)) {
					return false;
				}
			}

			break;
		}

		case Colour::ByteOrder::ARGB: {
			uint8_t newOffset = offset;

			if(alpha) {
				if(!byteBuffer.insertByte(a, newOffset)) {
					return false;
				}

				newOffset++;
			}

			if(!byteBuffer.insertByte(r, newOffset)) {
				return false;
			}

			newOffset++;

			if(!byteBuffer.insertByte(g, newOffset)) {
				return false;
			}

			newOffset++;

			if(!byteBuffer.insertByte(b, newOffset)) {
				return false;
			}

			break;
		}

		case Colour::ByteOrder::ABGR: {
			uint8_t newOffset = offset;

			if(alpha) {
				if(!byteBuffer.insertByte(a, newOffset)) {
					return false;
				}

				newOffset++;
			}

			if(!byteBuffer.insertByte(b, newOffset)) {
				return false;
			}

			newOffset++;

			if(!byteBuffer.insertByte(g, newOffset)) {
				return false;
			}

			newOffset++;

			if(!byteBuffer.insertByte(r, newOffset)) {
				return false;
			}

			break;
		}
	}

	return true;
}

bool Colour::writeTo(ByteBuffer & byteBuffer, bool alpha, ByteOrder byteOrder) const {
	switch(byteOrder) {
		case Colour::ByteOrder::RGBA: {
			uint8_t colourCount = alpha ? 4 : 3;

			for(uint8_t i = 0; i < colourCount; i++) {
				if(!byteBuffer.writeByte(c[i])) {
					return false;
				}
			}

			break;
		}

		case Colour::ByteOrder::BGRA: {
			if(!byteBuffer.writeByte(b)) {
				return false;
			}

			if(!byteBuffer.writeByte(g)) {
				return false;
			}

			if(!byteBuffer.writeByte(r)) {
				return false;
			}

			if(alpha) {
				if(!byteBuffer.writeByte(a)) {
					return false;
				}
			}

			break;
		}

		case Colour::ByteOrder::ARGB: {
			if(alpha) {
				if(!byteBuffer.writeByte(a)) {
					return false;
				}
			}

			if(!byteBuffer.writeByte(r)) {
				return false;
			}

			if(!byteBuffer.writeByte(g)) {
				return false;
			}

			if(!byteBuffer.writeByte(b)) {
				return false;
			}

			break;
		}

		case Colour::ByteOrder::ABGR: {
			if(alpha) {
				if(!byteBuffer.writeByte(a)) {
					return false;
				}
			}

			if(!byteBuffer.writeByte(b)) {
				return false;
			}

			if(!byteBuffer.writeByte(g)) {
				return false;
			}

			if(!byteBuffer.writeByte(r)) {
				return false;
			}

			break;
		}
	}

	return true;
}

std::string Colour::toString() const {
	return fmt::format("{}, {}, {}, {}", r, g, b, a);
}

uint8_t Colour::operator [] (size_t index) const {
	if(index > 3) {
		return 0;
	}

	return c[index];
}

bool Colour::operator == (const Colour & c) const {
	return r == c.r &&
		   g == c.g &&
		   b == c.b &&
		   a == c.a;
}

bool Colour::operator != (const Colour & c) const {
	return !operator == (c);
}
