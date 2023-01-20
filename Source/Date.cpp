#include "Date.h"

#include "Utilities/RapidJSONUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"

#include <fmt/core.h>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <ctime>
#include <regex>
#include <vector>

static constexpr const char * JSON_DATE_DAY_PROPERTY_NAME = "day";
static constexpr const char * JSON_DATE_MONTH_PROPERTY_NAME = "month";
static constexpr const char * JSON_DATE_YEAR_PROPERTY_NAME = "year";
static const std::vector<std::string_view> JSON_DATE_PROPERTY_NAMES = {
	JSON_DATE_DAY_PROPERTY_NAME,
	JSON_DATE_MONTH_PROPERTY_NAME,
	JSON_DATE_YEAR_PROPERTY_NAME
};

static constexpr const uint8_t NUMBER_OF_DAYS_IN_MONTH[] = {
	31, // January
	29, // February (28 in non-leap years)
	31, // March
	30, // April
	31, // Map
	30, // June
	31, // July
	31, // August
	30, // September
	31, // October
	30, // November
	31  // December
};

static const std::regex DATE_REGEX("^([A-Z][a-z]+)[ \t]+([1-9][0-9]*),?[ \t]+([1-9][0-9]*)$");

const Endianness Date::DEFAULT_ENDIANNESS = Endianness::BigEndian;

Date::Date()
	: m_day(0)
	, m_month(0)
	, m_year(0) {
	bool error = false;
	std::tm localTime = Utilities::getLocalTime(&error);

	if(!error) {
		m_day = localTime.tm_mday;
		m_month = localTime.tm_mon + 1;
		m_year = localTime.tm_year + 1900;
	}
}

Date::Date(uint8_t day, Date::Month month, uint16_t year)
	: m_day(day)
	, m_month(static_cast<uint8_t>(magic_enum::enum_integer(month)))
	, m_year(year) { }

Date::Date(const Date & date)
	: m_day(date.m_day)
	, m_month(date.m_month)
	, m_year(date.m_year) { }

Date & Date::operator = (const Date & date) {
	m_day = date.m_day;
	m_month = date.m_month;
	m_year = date.m_year;

	return *this;
}

Date::~Date() = default;

uint8_t Date::getDay() const {
	return m_day;
}

bool Date::setDay(uint8_t day) {
	if(!isDayValid(day, magic_enum::enum_cast<Month>(m_month).value())) {
		return false;
	}

	m_day = day;

	return true;
}

Date::Month Date::getMonth() const {
	return magic_enum::enum_cast<Month>(m_month).value();
}

uint8_t Date::getMonthNumber() const {
	return m_month;
}

std::string_view Date::getMonthName() const {
	return magic_enum::enum_name(magic_enum::enum_cast<Date::Month>(m_month).value());
}

std::string_view Date::getMonthName(Date::Month month) {
	return magic_enum::enum_name(month);
}

std::optional<std::string_view> Date::getMonthName(uint8_t month) {
	std::optional<Date::Month> monthOptional = magic_enum::enum_cast<Date::Month>(month);

	if(!monthOptional.has_value()) {
		return {};
	}

	return magic_enum::enum_name(monthOptional.value());
}

uint8_t Date::numberOfDaysInMonth(Date::Month month) {
	return NUMBER_OF_DAYS_IN_MONTH[magic_enum::enum_integer(month) - 1];
}

void Date::setMonth(Date::Month month) {
	m_month = static_cast<uint8_t>(magic_enum::enum_integer(month));
}

bool Date::setMonth(uint8_t month) {
	if(!isMonthValid(month)) {
		return false;
	}

	m_month = month;

	return true;
}

bool Date::setMonth(const std::string & data) {
	std::optional<Date::Month> monthOptional = magic_enum::enum_cast<Date::Month>(data);

	if(!monthOptional.has_value()) {
		return false;
	}

	m_month = static_cast<uint8_t>(magic_enum::enum_integer(monthOptional.value()));

	return true;
}

uint16_t Date::getYear() const {
	return m_year;
}

void Date::setYear(uint16_t year) {
	m_year = year;
}

bool Date::setValue(uint8_t day, Date::Month month, uint16_t year) {
	if(!isDayValid(day, month)) {
		return false;
	}

	m_day = day;
	m_month = static_cast<uint8_t>(magic_enum::enum_integer(month));
	m_year = year;

	return true;
}

bool Date::setValue(uint8_t day, uint8_t month, uint16_t year) {
	std::optional<Month> monthOptional = magic_enum::enum_cast<Month>(month);

	if(!monthOptional.has_value()) {
		return {};
	}

	return setValue(day, monthOptional.value(), year);
}

bool Date::setValue(const std::string & data) {
	std::smatch dateParts;
	std::regex_search(data, dateParts, DATE_REGEX);

	if(dateParts.size() != 4) {
		return false;
	}

	std::optional<Date::Month> monthOptional = magic_enum::enum_cast<Date::Month>(dateParts[1].str());

	if(!monthOptional.has_value()) {
		return false;
	}

	bool error = false;

	uint8_t day = Utilities::parseUnsignedByte(dateParts[2].str(), &error);

	if(error) {
		return false;
	}

	uint16_t year = Utilities::parseUnsignedShort(dateParts[3].str(), &error);

	if(error) {
		return false;
	}

	return setValue(day, monthOptional.value(), year);
}

bool Date::setValue(uint32_t packedValue, ByteOrder byteOrder, Endianness endianness) {
	switch(byteOrder) {
		case Date::ByteOrder::DMY: {
			return setValue(
						   static_cast<uint8_t> ((packedValue >> 24) & 0xff),
						   static_cast<uint8_t> ((packedValue >> 16) & 0xff),
				fromEndian(static_cast<uint16_t>((packedValue)       & 0xff), endianness)
			);
		}

		case Date::ByteOrder::MDY: {
			return setValue(
						   static_cast<uint8_t> ((packedValue >> 16) & 0xff),
						   static_cast<uint8_t> ((packedValue >> 24) & 0xff),
				fromEndian(static_cast<uint16_t>((packedValue)       & 0xff), endianness)
			);
		}

		case Date::ByteOrder::YMD: {
			return setValue(
						   static_cast<uint8_t> ((packedValue)       & 0xff),
						   static_cast<uint8_t> ((packedValue >>  8) & 0xff),
				fromEndian(static_cast<uint16_t>((packedValue >> 16) & 0xff), endianness)
			);
		}

		case Date::ByteOrder::YDM: {
			return setValue(
						   static_cast<uint8_t> ((packedValue >> 8)  & 0xff),
						   static_cast<uint8_t> ((packedValue)       & 0xff),
				fromEndian(static_cast<uint16_t>((packedValue >> 16) & 0xff), endianness)
			);
		}
	}

	return false;
}

bool Date::setValue(const rapidjson::Value & dateValue) {
	if(!dateValue.IsObject()) {
		spdlog::error("Invalid date type: '{}', expected 'object'.", Utilities::typeToString(dateValue.GetType()));
		return false;
	}

	// check for unhandled date properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = dateValue.MemberBegin(); i != dateValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_DATE_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Date has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse month
	if(!dateValue.HasMember(JSON_DATE_MONTH_PROPERTY_NAME)) {
		spdlog::error("Date is missing '{}' property'.", JSON_DATE_MONTH_PROPERTY_NAME);
		return false;
	}

	std::optional<Month> monthOptional;
	const rapidjson::Value & monthValue = dateValue[JSON_DATE_MONTH_PROPERTY_NAME];

	if(monthValue.IsString()) {
		monthOptional = magic_enum::enum_cast<Date::Month>(monthValue.GetString());
	}
	else if(monthValue.IsUint()) {
		monthOptional = magic_enum::enum_cast<Date::Month>(monthValue.GetUint());
	}
	else {
		spdlog::error("Date has an invalid '{}' property type: '{}', expected 'number' or 'string'.", JSON_DATE_MONTH_PROPERTY_NAME, Utilities::typeToString(monthValue.GetType()));
		return false;
	}

	if(!monthOptional.has_value()) {
		spdlog::error("Date '{}' property has invalid value: '{}'.", JSON_DATE_MONTH_PROPERTY_NAME, Utilities::valueToString(monthValue));
		return false;
	}

	// parse day
	if(!dateValue.HasMember(JSON_DATE_DAY_PROPERTY_NAME)) {
		spdlog::error("Date is missing '{}' property'.", JSON_DATE_DAY_PROPERTY_NAME);
		return false;
	}

	const rapidjson::Value & dayValue = dateValue[JSON_DATE_DAY_PROPERTY_NAME];

	if(!dayValue.IsUint()) {
		spdlog::error("Date has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_DATE_DAY_PROPERTY_NAME, Utilities::typeToString(dayValue.GetType()));
		return false;
	}

	uint32_t day = dayValue.GetUint();

	if(day > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Date '{}' property value has an invalid value: '{}', expected unsigned integer 'number' between 1 and {} inclusively.", JSON_DATE_DAY_PROPERTY_NAME, day, std::numeric_limits<uint8_t>::max());
		return false;
	}

	// parse year
	if(!dateValue.HasMember(JSON_DATE_YEAR_PROPERTY_NAME)) {
		spdlog::error("Date is missing '{}' property'.", JSON_DATE_YEAR_PROPERTY_NAME);
		return false;
	}

	const rapidjson::Value & yearValue = dateValue[JSON_DATE_YEAR_PROPERTY_NAME];

	if(!yearValue.IsUint()) {
		spdlog::error("Date has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_DATE_YEAR_PROPERTY_NAME, Utilities::typeToString(yearValue.GetType()));
		return false;
	}

	uint32_t year = yearValue.GetUint();

	if(year > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Date '{}' property value has an invalid value: '{}', expected unsigned integer 'number' between 0 and {} inclusively.", JSON_DATE_YEAR_PROPERTY_NAME, year, std::numeric_limits<uint8_t>::max());
		return false;
	}

	return setValue(static_cast<uint8_t>(day), monthOptional.value(), static_cast<uint16_t>(year));
}

int32_t Date::pack(ByteOrder byteOrder, Endianness endianness) const {
	uint16_t year = toEndian(m_year, endianness);

	switch(byteOrder) {
		case Date::ByteOrder::DMY:
			return m_day   << 24 | m_month << 16 | year;

		case Date::ByteOrder::MDY:
			return m_month << 24 | m_day   << 16 | year;

		case Date::ByteOrder::YMD:
			return year    << 16 | m_month <<  8 | m_day;

		case Date::ByteOrder::YDM:
			return year    << 16 | m_day   <<  8 | m_month;
	}

	return 0;
}

std::optional<Date> Date::unpack(uint32_t packedValue, ByteOrder byteOrder, Endianness endianness) {
	Date date;

	if(!date.setValue(packedValue, byteOrder, endianness)) {
		return {};
	}

	return date;
}

Date Date::unpack(uint32_t packedValue, bool * error, ByteOrder byteOrder, Endianness endianness) {
	std::optional<Date> optionalDate(unpack(packedValue, byteOrder, endianness));

	if(!optionalDate.has_value()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return std::move(optionalDate.value());
}

rapidjson::Value Date::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value dateValue(rapidjson::kObjectType);

	dateValue.AddMember(rapidjson::StringRef(JSON_DATE_DAY_PROPERTY_NAME), rapidjson::Value(m_day), allocator);

	rapidjson::Value monthNameValue(std::string(getMonthName()).c_str(), allocator);
	dateValue.AddMember(rapidjson::StringRef(JSON_DATE_MONTH_PROPERTY_NAME), monthNameValue, allocator);

	dateValue.AddMember(rapidjson::StringRef(JSON_DATE_YEAR_PROPERTY_NAME), rapidjson::Value(m_year), allocator);

	return dateValue;
}

std::optional<Date> Date::parseFrom(const rapidjson::Value & dateValue) {
	Date date;

	if(date.setValue(dateValue)) {
		return {};
	}

	return date;
}

Date Date::parseFrom(const rapidjson::Value & dateValue, bool * error) {
	std::optional<Date> optionalDate(parseFrom(dateValue));

	if(!optionalDate.has_value()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return std::move(optionalDate.value());
}

std::string Date::toString() const {
	return fmt::format("{} {}, {}", getMonthName(), m_day, m_year);
}

std::optional<Date> Date::parseFrom(const std::string & data) {
	Date date;

	if(!date.setValue(data)) {
		return {};
	}

	return date;
}

Date Date::parseFrom(const std::string & data, bool * error) {
	std::optional<Date> optionalDate(parseFrom(data));

	if(!optionalDate.has_value()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return std::move(optionalDate.value());
}

bool Date::isValid() const {
	return isMonthValid(m_month) &&
		   isDayValid(m_day, magic_enum::enum_cast<Date::Month>(m_month).value());
}

bool Date::isValid(const Date * date) {
	return date != nullptr && date->isValid();
}

bool Date::isDayValid(uint8_t day, Date::Month month) {
	return day >= 1 && day <= numberOfDaysInMonth(month);
}

bool Date::isMonthValid(uint8_t month) {
	return month >= 1 && month <= 12;
}

bool Date::operator <  (const Date & date) const {
	return m_data < date.m_data;
}

bool Date::operator <= (const Date & date) const {
	return m_data <= date.m_data;
}

bool Date::operator >  (const Date & date) const {
	return m_data > date.m_data;
}

bool Date::operator >= (const Date & date) const {
	return m_data >= date.m_data;
}

bool Date::operator == (const Date & date) const {
	return m_data == date.m_data;
}

bool Date::operator != (const Date & date) const {
	return m_data != date.m_data;
}
