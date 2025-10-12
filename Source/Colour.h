#ifndef _COLOUR_H_
#define _COLOUR_H_

class ByteBuffer;

#include <cstdint>
#include <limits>
#include <optional>
#include <string>

class Colour final {
public:
	enum class ByteOrder {
		RGBA,
		BGRA,
		ARGB,
		ABGR
	};

	Colour              (uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = std::numeric_limits<uint8_t>::max());
	Colour              (const uint8_t colour[3], uint8_t alpha, ByteOrder byteOrder = DEFAULT_BYTE_ORDER);
	Colour              (const uint8_t colour[4], ByteOrder byteOrder = DEFAULT_BYTE_ORDER);
	Colour              (uint32_t packedColour, ByteOrder byteOrder = DEFAULT_BYTE_ORDER);
	Colour              (const Colour & c, uint8_t a);
	Colour              (const Colour & c);
	Colour & operator = (const Colour & c);
	~Colour             ();

	bool isVisible() const;

	void setColour(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = std::numeric_limits<uint8_t>::max());
	void setColour(const uint8_t colour[3], uint8_t alpha = std::numeric_limits<uint8_t>::max());
	void setColour(const uint8_t colour[4]);
	void setColour(uint32_t packedColour, ByteOrder byteOrder = DEFAULT_BYTE_ORDER);
	void setColour(const Colour & c, uint8_t alpha);
	void setColour(const Colour & c);

	uint32_t pack       (ByteOrder byteOrder = DEFAULT_BYTE_ORDER) const;
	static Colour unpack(uint32_t packedColour, ByteOrder byteOrder = DEFAULT_BYTE_ORDER);

	void scale          (float factor, bool alpha = false);
	Colour scaled       (float factor, bool alpha = false) const;

	void invert         ();
	Colour inverted     ()                  const;

	static Colour lerp  (const Colour & c, const Colour & d, float amount);

	static Colour getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error);
	static Colour getFrom(const ByteBuffer & byteBuffer, size_t offset, bool alpha, bool * error);
	static Colour getFrom(const ByteBuffer & byteBuffer, size_t offset, bool alpha, ByteOrder byteOrder, bool * error);
	static std::optional<Colour> getFrom(const ByteBuffer & byteBuffer, size_t offset, bool alpha = true, ByteOrder byteOrder = DEFAULT_BYTE_ORDER);
	static Colour readFrom(const ByteBuffer & byteBuffer, bool * error);
	static Colour readFrom(const ByteBuffer & byteBuffer, bool alpha, bool * error);
	static Colour readFrom(const ByteBuffer & byteBuffer, bool alpha, ByteOrder byteOrder, bool * error);
	static std::optional<Colour> readFrom(const ByteBuffer & byteBuffer, bool alpha = true, ByteOrder byteOrder = DEFAULT_BYTE_ORDER);
	bool putIn(ByteBuffer & byteBuffer, size_t offset, bool alpha = true, ByteOrder byteOrder = DEFAULT_BYTE_ORDER) const;
	bool insertIn(ByteBuffer & byteBuffer, size_t offset, bool alpha = true, ByteOrder byteOrder = DEFAULT_BYTE_ORDER) const;
	bool writeTo(ByteBuffer & byteBuffer, bool alpha = true, ByteOrder byteOrder = DEFAULT_BYTE_ORDER) const;

	std::string toString() const;

	uint8_t operator [] (size_t index) const;

	bool operator ==    (const Colour & c)  const;
	bool operator !=    (const Colour & c)  const;

	static const ByteOrder DEFAULT_BYTE_ORDER;

	static const Colour ALICE_BLUE;
	static const Colour ANTIQUE_WHITE;
	static const Colour AQUA;
	static const Colour AQUAMARINE;
	static const Colour AZURE;
	static const Colour BEIGE;
	static const Colour BISQUE;
	static const Colour BLACK;
	static const Colour BLANCHED_ALMOND;
	static const Colour BLUE;
	static const Colour BLUE_VIOLET;
	static const Colour BROWN;
	static const Colour BURLY_WOOD;
	static const Colour CADET_BLUE;
	static const Colour CHARTREUSE;
	static const Colour CHOCOLATE;
	static const Colour CORAL;
	static const Colour CORNFLOWER_BLUE;
	static const Colour CORNSILK;
	static const Colour CRIMSON;
	static const Colour CYAN;
	static const Colour DARK_BLUE;
	static const Colour DARK_BROWN;
	static const Colour DARK_CYAN;
	static const Colour DARK_GOLDENROD;
	static const Colour DARK_GRAY;
	static const Colour DARK_GREEN;
	static const Colour DARK_KHAKI;
	static const Colour DARK_MAGENTA;
	static const Colour DARK_OLIVE_GREEN;
	static const Colour DARK_ORANGE;
	static const Colour DARK_ORCHID;
	static const Colour DARK_RED;
	static const Colour DARK_SALMON;
	static const Colour DARK_SEA_GREEN;
	static const Colour DARK_SLATE_BLUE;
	static const Colour DARK_SLATE_GRAY;
	static const Colour DARK_TURQUOISE;
	static const Colour DARK_VIOLET;
	static const Colour DEEP_PINK;
	static const Colour DEEP_SKY_BLUE;
	static const Colour DIM_GRAY;
	static const Colour DODGER_BLUE;
	static const Colour FIREBRICK;
	static const Colour FLORAL_WHITE;
	static const Colour FOREST_GREEN;
	static const Colour FUCHSIA;
	static const Colour GAINSBORO;
	static const Colour GHOST_WHITE;
	static const Colour GOLD;
	static const Colour GOLDENROD;
	static const Colour GRAY;
	static const Colour GREEN;
	static const Colour GREEN_YELLOW;
	static const Colour HONEYDEW;
	static const Colour HOT_PINK;
	static const Colour INDIAN_RED;
	static const Colour INDIGO;
	static const Colour IVORY;
	static const Colour KHAKI;
	static const Colour LAVENDER;
	static const Colour LAVENDER_BLUSH;
	static const Colour LAWN_GREEN;
	static const Colour LEMON_CHIFFON;
	static const Colour LIGHT_BLUE;
	static const Colour LIGHT_BROWN;
	static const Colour LIGHT_CORAL;
	static const Colour LIGHT_CYAN;
	static const Colour LIGHT_GOLDENROD_YELLOW;
	static const Colour LIGHT_GRAY;
	static const Colour LIGHT_GREEN;
	static const Colour LIGHT_PINK;
	static const Colour LIGHT_RED;
	static const Colour LIGHT_SALMON;
	static const Colour LIGHT_SEA_GREEN;
	static const Colour LIGHT_SKY_BLUE;
	static const Colour LIGHT_SLATE_GRAY;
	static const Colour LIGHT_STEEL_BLUE;
	static const Colour LIGHT_YELLOW;
	static const Colour LIME;
	static const Colour LIME_GREEN;
	static const Colour LINEN;
	static const Colour MAGENTA;
	static const Colour MAROON;
	static const Colour MEDIUM_AQUAMARINE;
	static const Colour MEDIUM_BLUE;
	static const Colour MEDIUM_ORCHID;
	static const Colour MEDIUM_PURPLE;
	static const Colour MEDIUM_SEA_GREEN;
	static const Colour MEDIUM_SLATE_BLUE;
	static const Colour MEDIUM_SPRING_GREEN;
	static const Colour MEDIUM_TURQUOISE;
	static const Colour MEDIUM_VIOLET_RED;
	static const Colour MIDNIGHT_BLUE;
	static const Colour MINT_CREAM;
	static const Colour MISTY_ROSE;
	static const Colour MOCCASIN;
	static const Colour NAVAJO_WHITE;
	static const Colour NAVY;
	static const Colour OLD_LACE;
	static const Colour OLIVE;
	static const Colour OLIVE_DRAB;
	static const Colour ORANGE;
	static const Colour ORANGE_RED;
	static const Colour ORCHID;
	static const Colour PALE_GOLDENROD;
	static const Colour PALE_GREEN;
	static const Colour PALE_TURQUOISE;
	static const Colour PALE_VIOLET_RED;
	static const Colour PAPAYA_WHIP;
	static const Colour PEACH_PUFF;
	static const Colour PERU;
	static const Colour PINK;
	static const Colour PLUM;
	static const Colour POWDER_BLUE;
	static const Colour PURPLE;
	static const Colour RED;
	static const Colour ROSY_BROWN;
	static const Colour ROYAL_BLUE;
	static const Colour SADDLE_BROWN;
	static const Colour SALMON;
	static const Colour SANDY_BROWN;
	static const Colour SEA_GREEN;
	static const Colour SEA_SHELL;
	static const Colour SIENNA;
	static const Colour SILVER;
	static const Colour SKY_BLUE;
	static const Colour SLATE_BLUE;
	static const Colour SLATE_GRAY;
	static const Colour SNOW;
	static const Colour SPRING_GREEN;
	static const Colour STEEL_BLUE;
	static const Colour TAN;
	static const Colour TEAL;
	static const Colour THISTLE;
	static const Colour TOMATO;
	static const Colour INVISIBLE;
	static const Colour TURQUOISE;
	static const Colour VIOLET;
	static const Colour WHEAT;
	static const Colour WHITE;
	static const Colour WHITE_SMOKE;
	static const Colour YELLOW;
	static const Colour YELLOW_GREEN;

public:
	union {
		struct {
			uint8_t r, g, b, a;
		};

		uint8_t c[4];
	};
};

#endif // _COLOUR_H_
