#include "Colour.h"

#include "Math/ExtendedMath.h"

#include <fmt/core.h>

const Colour::ByteOrder Colour::DEFAULT_BYTE_ORDER = Colour::ByteOrder::RGBA;

const Colour Colour::AliceBlue           (240, 248, 255, 255);
const Colour Colour::AntiqueWhite        (250, 235, 215, 255);
const Colour Colour::Aqua                (  0, 255, 255, 255);
const Colour Colour::Aquamarine          (127, 255, 212, 255);
const Colour Colour::Azure               (240, 255, 255, 255);
const Colour Colour::Beige               (245, 245, 220, 255);
const Colour Colour::Bisque              (255, 228, 196, 255);
const Colour Colour::Black               (  0,   0,   0, 255);
const Colour Colour::BlanchedAlmond      (255, 235, 205, 255);
const Colour Colour::Blue                (  0,   0, 255, 255);
const Colour Colour::BlueViolet          (138,  43, 226, 255);
const Colour Colour::Brown               (165,  42,  42, 255);
const Colour Colour::BurlyWood           (222, 184, 135, 255);
const Colour Colour::CadetBlue           ( 95, 158, 160, 255);
const Colour Colour::Chartreuse          (127, 255,   0, 255);
const Colour Colour::Chocolate           (210, 105,  30, 255);
const Colour Colour::Coral               (255, 127,  80, 255);
const Colour Colour::CornflowerBlue      (100, 149, 237, 255);
const Colour Colour::Cornsilk            (255, 248, 220, 255);
const Colour Colour::Crimson             (220,  20,  60, 255);
const Colour Colour::Cyan                (  0, 255, 255, 255);
const Colour Colour::DarkBlue            (  0,   0, 139, 255);
const Colour Colour::DarkBrown           (101,  67,  33, 255);
const Colour Colour::DarkCyan            (  0, 139, 139, 255);
const Colour Colour::DarkGoldenrod       (184, 134,  11, 255);
const Colour Colour::DarkGray            (169, 169, 169, 255);
const Colour Colour::DarkGreen           (  0, 100,   0, 255);
const Colour Colour::DarkKhaki           (189, 183, 107, 255);
const Colour Colour::DarkMagenta         (139,   0, 139, 255);
const Colour Colour::DarkOliveGreen      ( 85, 107,  47, 255);
const Colour Colour::DarkOrange          (255, 140,   0, 255);
const Colour Colour::DarkOrchid          (153,  50, 204, 255);
const Colour Colour::DarkRed             (139,   0,   0, 255);
const Colour Colour::DarkSalmon          (233, 150, 122, 255);
const Colour Colour::DarkSeaGreen        (143, 188, 139, 255);
const Colour Colour::DarkSlateBlue       ( 72,  61, 139, 255);
const Colour Colour::DarkSlateGray       ( 47,  79,  79, 255);
const Colour Colour::DarkTurquoise       (  0, 206, 209, 255);
const Colour Colour::DarkViolet          (148,   0, 211, 255);
const Colour Colour::DeepPink            (255,  20, 147, 255);
const Colour Colour::DeepSkyBlue         (  0, 191, 255, 255);
const Colour Colour::DimGray             (105, 105, 105, 255);
const Colour Colour::DodgerBlue          ( 30, 144, 255, 255);
const Colour Colour::Firebrick           (178,  34,  34, 255);
const Colour Colour::FloralWhite         (255, 250, 240, 255);
const Colour Colour::ForestGreen         ( 34, 139,  34, 255);
const Colour Colour::Fuchsia             (255,   0, 255, 255);
const Colour Colour::Gainsboro           (220, 220, 220, 255);
const Colour Colour::GhostWhite          (248, 248, 255, 255);
const Colour Colour::Gold                (255, 215,   0, 255);
const Colour Colour::Goldenrod           (218, 165,  32, 255);
const Colour Colour::Gray                (128, 128, 128, 255);
const Colour Colour::Green               (  0, 128,   0, 255);
const Colour Colour::GreenYellow         (173, 255,  47, 255);
const Colour Colour::Honeydew            (240, 255, 240, 255);
const Colour Colour::HotPink             (255, 105, 180, 255);
const Colour Colour::IndianRed           (205,  92,  92, 255);
const Colour Colour::Indigo              ( 75,   0, 130, 255);
const Colour Colour::Ivory               (255, 255, 240, 255);
const Colour Colour::Khaki               (240, 230, 140, 255);
const Colour Colour::Lavender            (230, 230, 250, 255);
const Colour Colour::LavenderBlush       (255, 240, 245, 255);
const Colour Colour::LawnGreen           (124, 252,   0, 255);
const Colour Colour::LemonChiffon        (255, 250, 205, 255);
const Colour Colour::LightBlue           (173, 216, 230, 255);
const Colour Colour::LightBrown          (152, 118,  84, 255);
const Colour Colour::LightCoral          (240, 128, 128, 255);
const Colour Colour::LightCyan           (224, 255, 255, 255);
const Colour Colour::LightGoldenrodYellow(250, 250, 210, 255);
const Colour Colour::LightGray           (211, 211, 211, 255);
const Colour Colour::LightGreen          (144, 238, 144, 255);
const Colour Colour::LightPink           (255, 182, 193, 255);
const Colour Colour::LightRed            (255, 102, 102, 255);
const Colour Colour::LightSalmon         (255, 160, 122, 255);
const Colour Colour::LightSeaGreen       ( 32, 178, 170, 255);
const Colour Colour::LightSkyBlue        (135, 206, 250, 255);
const Colour Colour::LightSlateGray      (119, 136, 153, 255);
const Colour Colour::LightSteelBlue      (176, 196, 222, 255);
const Colour Colour::LightYellow         (255, 255, 224, 255);
const Colour Colour::Lime                (  0, 255,   0, 255);
const Colour Colour::LimeGreen           ( 50, 205,  50, 255);
const Colour Colour::Linen               (250, 240, 230, 255);
const Colour Colour::Magenta             (255,   0, 255, 255);
const Colour Colour::Maroon              (128,   0,   0, 255);
const Colour Colour::MediumAquamarine    (102, 205, 170, 255);
const Colour Colour::MediumBlue          (  0,   0, 205, 255);
const Colour Colour::MediumOrchid        (186,  85, 211, 255);
const Colour Colour::MediumPurple        (147, 112, 219, 255);
const Colour Colour::MediumSeaGreen      ( 60, 179, 113, 255);
const Colour Colour::MediumSlateBlue     (123, 104, 238, 255);
const Colour Colour::MediumSpringGreen   (  0, 250, 154, 255);
const Colour Colour::MediumTurquoise     ( 72, 209, 204, 255);
const Colour Colour::MediumVioletRed     (199,  21, 133, 255);
const Colour Colour::MidnightBlue        ( 25,  25, 112, 255);
const Colour Colour::MintCream           (245, 255, 250, 255);
const Colour Colour::MistyRose           (255, 228, 225, 255);
const Colour Colour::Moccasin            (255, 228, 181, 255);
const Colour Colour::NavajoWhite         (255, 222, 173, 255);
const Colour Colour::Navy                (  0,   0, 128, 255);
const Colour Colour::OldLace             (253, 245, 230, 255);
const Colour Colour::Olive               (128, 128,   0, 255);
const Colour Colour::OliveDrab           (107, 142,  35, 255);
const Colour Colour::Orange              (255, 165,   0, 255);
const Colour Colour::OrangeRed           (255,  69,   0, 255);
const Colour Colour::Orchid              (218, 112, 214, 255);
const Colour Colour::PaleGoldenrod       (238, 232, 170, 255);
const Colour Colour::PaleGreen           (152, 251, 152, 255);
const Colour Colour::PaleTurquoise       (175, 238, 238, 255);
const Colour Colour::PaleVioletRed       (219, 112, 147, 255);
const Colour Colour::PapayaWhip          (255, 239, 213, 255);
const Colour Colour::PeachPuff           (255, 218, 185, 255);
const Colour Colour::Peru                (205, 133,  63, 255);
const Colour Colour::Pink                (255, 192, 203, 255);
const Colour Colour::Plum                (221, 160, 221, 255);
const Colour Colour::PowderBlue          (176, 224, 230, 255);
const Colour Colour::Purple              (128,   0, 128, 255);
const Colour Colour::Red                 (255,   0,   0, 255);
const Colour Colour::RosyBrown           (188, 143, 143, 255);
const Colour Colour::RoyalBlue           ( 65, 105, 225, 255);
const Colour Colour::SaddleBrown         (139,  69,  19, 255);
const Colour Colour::Salmon              (250, 128, 114, 255);
const Colour Colour::SandyBrown          (244, 164,  96, 255);
const Colour Colour::SeaGreen            ( 46, 139,  87, 255);
const Colour Colour::SeaShell            (255, 245, 238, 255);
const Colour Colour::Sienna              (160,  82,  45, 255);
const Colour Colour::Silver              (192, 192, 192, 255);
const Colour Colour::SkyBlue             (135, 206, 235, 255);
const Colour Colour::SlateBlue           (106,  90, 205, 255);
const Colour Colour::SlateGray           (112, 128, 144, 255);
const Colour Colour::Snow                (255, 250, 250, 255);
const Colour Colour::SpringGreen         (  0, 255, 127, 255);
const Colour Colour::SteelBlue           ( 70, 130, 180, 255);
const Colour Colour::Tan                 (210, 180, 140, 255);
const Colour Colour::Teal                (  0, 128, 128, 255);
const Colour Colour::Thistle             (216, 191, 216, 255);
const Colour Colour::Tomato              (255,  99,  71, 255);
const Colour Colour::Transparent         (  0,   0,   0,   0);
const Colour Colour::Turquoise           ( 64, 224, 208, 255);
const Colour Colour::Violet              (238, 130, 238, 255);
const Colour Colour::Wheat               (245, 222, 179, 255);
const Colour Colour::White               (255, 255, 255, 255);
const Colour Colour::WhiteSmoke          (245, 245, 245, 255);
const Colour Colour::Yellow              (255, 255,   0, 255);
const Colour Colour::YellowGreen         (154, 205,  50, 255);

Colour::Colour(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
	: r(red)
	, g(green)
	, b(blue)
	, a(alpha) { }

Colour::Colour(const uint8_t colour[3], uint8_t alpha)
	: r(colour[0])
	, g(colour[1])
	, b(colour[2])
	, a(alpha) { }

Colour::Colour(const uint8_t colour[4])
	: r(colour[0])
	, g(colour[1])
	, b(colour[2])
	, a(colour[3]) { }

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

	r = static_cast<uint8_t>(Math::clamp(scaledRed, 0, 255));
	g = static_cast<uint8_t>(Math::clamp(scaledGreen, 0, 255));
	b = static_cast<uint8_t>(Math::clamp(scaledBlue, 0, 255));

	if(alpha) {
		a = static_cast<uint8_t>(Math::clamp(scaledAlpha, 0, 255));
	}
}

Colour Colour::scaled(float factor, bool alpha) const {
	     if(factor <  0.0f) { return Transparent; }
	else if(factor == 0.0f) {
		return Colour(0, 0, 0, alpha ? 0 : a);
	}

	float scaledRed   = Math::clamp(static_cast<float>(r) * factor, 0.0f, 255.0f);
	float scaledGreen = Math::clamp(static_cast<float>(g) * factor, 0.0f, 255.0f);
	float scaledBlue  = Math::clamp(static_cast<float>(b) * factor, 0.0f, 255.0f);
	float scaledAlpha = Math::clamp(static_cast<float>(a) * factor, 0.0f, 255.0f);

	return Colour(static_cast<uint8_t>(scaledRed),
				  static_cast<uint8_t>(scaledGreen),
				  static_cast<uint8_t>(scaledBlue),
		  alpha ? static_cast<uint8_t>(scaledAlpha) : a);
}

void Colour::invert() {
	r = 255 - r;
	g = 255 - g;
	b = 255 - b;
}

Colour Colour::inverted() const {
	return Colour(255 - r, 255 - g, 255 - b, a);
}

Colour Colour::lerp(const Colour & c, const Colour & d, float amount) {
	float lerpedRed   = Math::clamp(Math::lerp(static_cast<float>(c.r), static_cast<float>(d.r), amount), 0.0f, 255.0f);
	float lerpedGreen = Math::clamp(Math::lerp(static_cast<float>(c.g), static_cast<float>(d.g), amount), 0.0f, 255.0f);
	float lerpedBlue  = Math::clamp(Math::lerp(static_cast<float>(c.b), static_cast<float>(d.b), amount), 0.0f, 255.0f);
	float lerpedAlpha = Math::clamp(Math::lerp(static_cast<float>(c.a), static_cast<float>(d.a), amount), 0.0f, 255.0f);

	return Colour(static_cast<uint8_t>(lerpedRed),
				  static_cast<uint8_t>(lerpedGreen),
				  static_cast<uint8_t>(lerpedBlue),
				  static_cast<uint8_t>(lerpedAlpha));
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
