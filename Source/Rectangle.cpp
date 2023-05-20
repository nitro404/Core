#include "Rectangle.h"

#include "Math/Vector2.h"
#include "Utilities/RapidJSONUtilities.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

static constexpr const char * JSON_X_POSITION_PROPERTY_NAME = "x";
static constexpr const char * JSON_Y_POSITION_PROPERTY_NAME = "y";
static constexpr const char * JSON_WIDTH_PROPERTY_NAME = "width";
static constexpr const char * JSON_HEIGHT_PROPERTY_NAME = "height";

const Rect Rect::Zero(0, 0, 0, 0);

Rect::Rect(int32_t xPos, int32_t yPos, uint32_t width, uint32_t height)
	: x(xPos)
	, y(yPos)
	, w(width)
	, h(height) { }

Rect::Rect(const int32_t p[2], const uint32_t d[2])
	: x(p[0])
	, y(p[1])
	, w(d[2])
	, h(d[3]) { }

Rect::Rect(int32_t xPos, int32_t yPos, const Dimension & dimensions)
	: x(xPos)
	, y(yPos)
	, w(dimensions.w)
	, h(dimensions.h) { }

Rect::Rect(const Point2D & position, uint32_t width, uint32_t height)
	: x(position.x)
	, y(position.y)
	, w(width)
	, h(height) { }

Rect::Rect(uint64_t packedPosition, uint64_t packedDimension)
	: x(static_cast<int32_t>( (packedPosition  >> 32) & 0xffff))
	, y(static_cast<int32_t>(  packedPosition         & 0xffff))
	, w(static_cast<uint32_t>((packedDimension >> 32) & 0xffff))
	, h(static_cast<uint32_t>( packedDimension        & 0xffff)) { }

Rect::Rect(const Rect & rectangle)
	: x(rectangle.x)
	, y(rectangle.y)
	, w(rectangle.w)
	, h(rectangle.h) { }

Rect & Rect::operator = (const Rect & rectangle) {
	x = rectangle.x;
	y = rectangle.y;
	w = rectangle.w;
	h = rectangle.h;

	return *this;
}

Rect::~Rect() = default;

bool Rect::isEmpty() const {
	return w == 0 || h == 0;
}

bool Rect::isNotEmpty() const {
	return w != 0 && h != 0;
}

uint64_t Rect::getPerimeter() const {
	return 2 * (w + h);
}

uint64_t Rect::getArea() const {
	return w * h;
}

Point2D Rect::getPosition() const {
	return Point2D(x, y);
}

Dimension Rect::getDimensions() const {
	return Dimension(w, h);
}

Point2D Rect::getCenter() const {
	return Point2D(x + (w / 2), y + (h / 2));
}

Point2D Rect::getLeft() const {
	return Point2D(x, y + (h / 2));
}

Point2D Rect::getRight() const {
	return Point2D(x + w, y + (h / 2));
}

Point2D Rect::getTop() const {
	return Point2D(x + (w / 2), y + h);
}

Point2D Rect::getBottom() const {
	return Point2D(x + (w / 2), y);
}

Point2D Rect::getTopLeft() const {
	return Point2D(x, y + h);
}

Point2D Rect::getTopRight() const {
	return Point2D(x + w, y + h);
}

Point2D Rect::getBottomLeft() const {
	return Point2D(x, y);
}

Point2D Rect::getBottomRight() const {
	return Point2D(x + w, y);
}

bool Rect::contains(const Point2D & point) const {
	return point.x >= x     && point.y >= y &&
		   point.x <= x + w && point.y <= y + h;
}

bool Rect::contains(const Vector2 & point) const {
	return point.x >= static_cast<float>(x)     && point.y >= static_cast<float>(y) &&
		   point.x <= static_cast<float>(x + w) && point.y <= static_cast<float>(y + h);
}

bool Rect::contains(const Rect & rectangle) const {
	return rectangle.x >= x         && rectangle.y >= y &&
		   rectangle.x + w <= x + w && rectangle.y + h <= y + h;
}

bool Rect::intersects(const Rect & rectangle) const {
	uint32_t tw = w;
	uint32_t th = h;
	uint32_t rw = rectangle.w;
	uint32_t rh = rectangle.h;

	if(rw == 0 || rh == 0 || tw == 0 || th == 0) { return false; }

	int32_t tx = x;
	int32_t ty = y;
	int32_t rx = rectangle.x;
	int32_t ry = rectangle.y;
	rw += rx;
	rh += ry;
	tw += tx;
	th += ty;

	return ((rw < rx || rw > tx) &&
			(rh < ry || rh > ty) &&
			(tw < tx || tw > rx) &&
			(th < ty || th > ry));
}

Rect Rect::intersection(const Rect & rectangle) const {
	int32_t tx1 = x;
	int32_t ty1 = y;
	int32_t rx1 = rectangle.x;
	int32_t ry1 = rectangle.y;
	int64_t tx2 = x + w;
	int64_t ty2 = y + h;
	int64_t rx2 = rectangle.x + rectangle.w;
	int64_t ry2 = rectangle.y + rectangle.h;

	if(tx1 < rx1) { tx1 = rx1; }
	if(ty1 < ry1) { ty1 = ry1; }
	if(tx2 > rx2) { tx2 = rx2; }
	if(ty2 > ry2) { ty2 = ry2; }

	tx2 -= tx1;
	ty2 -= ty1;

	if(tx1 < std::numeric_limits<int32_t>::min()) { tx2 = std::numeric_limits<int32_t>::min(); }
	if(ty2 < std::numeric_limits<int32_t>::min()) { ty2 = std::numeric_limits<int32_t>::min(); }

	return Rect(tx1, ty1, static_cast<uint32_t>(tx2), static_cast<uint32_t>(ty2));
}

Rect Rect::unioned(const Rect & rectangle) const {
	int64_t tx2 = w;
	int64_t ty2 = h;

	if((tx2 | ty2) < 0) { return rectangle; }

	int64_t rx2 = rectangle.w;
	int64_t ry2 = rectangle.h;

	if((rx2 | ry2) < 0) { return *this; }

	int32_t tx1 = x;
	int32_t ty1 = y;
	tx2 += tx1;
	ty2 += ty1;

	int32_t rx1 = rectangle.x;
	int32_t ry1 = rectangle.y;
	rx2 += rx1;
	ry2 += ry1;

	if(tx1 > rx1) { tx1 = rx1; }
	if(ty1 > ry1) { ty1 = ry1; }
	if(tx2 < rx2) { tx2 = rx2; }
	if(ty2 < ry2) { ty2 = ry2; }

	tx2 -= tx1;
	ty2 -= ty1;

	if(tx2 > std::numeric_limits<int32_t>::max()) { tx2 = std::numeric_limits<int32_t>::max(); }
	if(ty2 > std::numeric_limits<int32_t>::max()) { ty2 = std::numeric_limits<int32_t>::max(); }

	return Rect(tx1, ty1, static_cast<uint32_t>(tx2), static_cast<uint32_t>(ty2));
}

rapidjson::Value Rect::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value rectangleValue(rapidjson::kObjectType);

	rectangleValue.AddMember(rapidjson::StringRef(JSON_X_POSITION_PROPERTY_NAME), rapidjson::Value(x), allocator);
	rectangleValue.AddMember(rapidjson::StringRef(JSON_Y_POSITION_PROPERTY_NAME), rapidjson::Value(y), allocator);
	rectangleValue.AddMember(rapidjson::StringRef(JSON_WIDTH_PROPERTY_NAME), rapidjson::Value(w), allocator);
	rectangleValue.AddMember(rapidjson::StringRef(JSON_HEIGHT_PROPERTY_NAME), rapidjson::Value(h), allocator);

	return rectangleValue;
}

Rect Rect::parseFrom(const rapidjson::Value & rectangleValue, bool * error) {
	if(!rectangleValue.IsObject()) {
		spdlog::error("Invalid rectangle type: '{}', expected 'object'.", Utilities::typeToString(rectangleValue.GetType()));
		return {};
	}

	// parse x position
	if(!rectangleValue.HasMember(JSON_X_POSITION_PROPERTY_NAME)) {
		spdlog::error("Rectangle is missing '{}' property'.", JSON_X_POSITION_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & xPositionValue = rectangleValue[JSON_X_POSITION_PROPERTY_NAME];

	if(!xPositionValue.IsInt()) {
		spdlog::error("Rectangle has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_X_POSITION_PROPERTY_NAME, Utilities::typeToString(xPositionValue.GetType()));
		return {};
	}

	// parse y position
	if(!rectangleValue.HasMember(JSON_Y_POSITION_PROPERTY_NAME)) {
		spdlog::error("Rectangle is missing '{}' property'.", JSON_Y_POSITION_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & yPositionValue = rectangleValue[JSON_Y_POSITION_PROPERTY_NAME];

	if(!yPositionValue.IsInt()) {
		spdlog::error("Rectangle has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_Y_POSITION_PROPERTY_NAME, Utilities::typeToString(yPositionValue.GetType()));
		return {};
	}

	// parse width
	if(!rectangleValue.HasMember(JSON_WIDTH_PROPERTY_NAME)) {
		spdlog::error("Rectangle is missing '{}' property'.", JSON_WIDTH_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & widthValue = rectangleValue[JSON_WIDTH_PROPERTY_NAME];

	if(!widthValue.IsUint()) {
		spdlog::error("Rectangle has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_WIDTH_PROPERTY_NAME, Utilities::typeToString(widthValue.GetType()));
		return {};
	}

	// parse height
	if(!rectangleValue.HasMember(JSON_HEIGHT_PROPERTY_NAME)) {
		spdlog::error("Rectangle is missing '{}' property'.", JSON_HEIGHT_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & heightValue = rectangleValue[JSON_HEIGHT_PROPERTY_NAME];

	if(!heightValue.IsUint()) {
		spdlog::error("Rectangle has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_HEIGHT_PROPERTY_NAME, Utilities::typeToString(heightValue.GetType()));
		return {};
	}

	return Rect(xPositionValue.GetInt(), yPositionValue.GetInt(), widthValue.GetUint(), heightValue.GetUint());
}

std::optional<Rect> Rect::parseFrom(const rapidjson::Value & rectangleValue) {
	bool error = false;

	Rect rectangle(parseFrom(rectangleValue, &error));

	if(error) {
		return {};
	}

	return rectangle;
}

std::string Rect::toString() const {
	return fmt::format("{}, {}, {}, {}", x, y, w, h);
}

bool Rect::operator == (const Rect & rectangle) const {
	return x == rectangle.x &&
		   y == rectangle.y &&
		   w == rectangle.w &&
		   h == rectangle.h;
}

bool Rect::operator != (const Rect & rectangle) const {
	return !operator == (rectangle);
}
