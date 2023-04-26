#ifndef _DATE_H_
#define _DATE_H_

#include "Endianness.h"

#include <rapidjson/document.h>

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

class Date final {
public:
	enum class Month {
		January = 1,
		February,
		March,
		April,
		May,
		June,
		July,
		August,
		September,
		October,
		November,
		December
	};

	enum class ByteOrder {
		DMY,
		MDY,
		YMD,
		YDM
	};

	Date();
	Date(uint8_t day, Month month, uint16_t year);
	Date(std::chrono::time_point<std::chrono::system_clock> timestamp);
	Date(const Date & date);
	Date & operator = (const Date & date);
	~Date();

	uint8_t getDay() const;
	bool setDay(uint8_t day);
	Month getMonth() const;
	uint8_t getMonthNumber() const;
	std::string_view getMonthName() const;
	static std::string_view getMonthName(Month month);
	static std::optional<std::string_view> getMonthName(uint8_t month);
	static uint8_t numberOfDaysInMonth(Month month);
	void setMonth(Month month);
	bool setMonth(uint8_t month);
	bool setMonth(const std::string & data);
	uint16_t getYear() const;
	void setYear(uint16_t year);

	bool setValue(uint8_t day, Month month, uint16_t year);
	bool setValue(uint8_t day, uint8_t month, uint16_t year);
	bool setValue(std::chrono::time_point<std::chrono::system_clock> timestamp);
	bool setValue(const std::string & data);
	bool setValue(uint32_t packedValue, ByteOrder byteOrder = DEFAULT_BYTE_ORDER, Endianness endianness = DEFAULT_ENDIANNESS);
	bool setValue(const rapidjson::Value & dateValue);

	int32_t pack(ByteOrder byteOrder = DEFAULT_BYTE_ORDER, Endianness endianness = DEFAULT_ENDIANNESS) const;
	static std::optional<Date> unpack(uint32_t packedValue, ByteOrder byteOrder = DEFAULT_BYTE_ORDER, Endianness endianness = DEFAULT_ENDIANNESS);
	static Date unpack(uint32_t packedValue, bool * error, ByteOrder byteOrder = DEFAULT_BYTE_ORDER, Endianness endianness = DEFAULT_ENDIANNESS);

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::optional<Date> parseFrom(const rapidjson::Value & dateValue);
	static Date parseFrom(const rapidjson::Value & dateValue, bool * error);

	std::string toString() const;
	static std::optional<Date> parseFrom(const std::string & data);
	static Date parseFrom(const std::string & data, bool * error);

	bool isValid() const;
	static bool isValid(const Date * date);
	static bool isDayValid(uint8_t day, Month month);
	static bool isMonthValid(uint8_t month);

	bool operator <  (const Date & date) const;
	bool operator <= (const Date & date) const;
	bool operator >  (const Date & date) const;
	bool operator >= (const Date & date) const;
	bool operator == (const Date & date) const;
	bool operator != (const Date & date) const;

	static const ByteOrder DEFAULT_BYTE_ORDER;
	static const Endianness DEFAULT_ENDIANNESS;

private:
	union {
		struct {
			uint8_t m_day;
			uint8_t m_month;
			uint16_t m_year;
		};

		uint32_t m_data;
	};
};

#endif // _DATE_H_
