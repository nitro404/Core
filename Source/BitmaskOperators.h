#ifndef _BITMASK_OPERATORS_H_
#define _BITMASK_OPERATORS_H_

#include <type_traits>

template<typename T>
struct BitmaskOperators
{
	static const bool enabled = false;
};

template<typename T>
constexpr inline typename std::enable_if<BitmaskOperators<T>::enabled, T>::type
operator ~(T e) {
	typedef typename std::underlying_type<T>::type Type;

	return static_cast<T> (
		~static_cast<Type>(e)
	);
}

template<typename T>
constexpr inline typename std::enable_if<BitmaskOperators<T>::enabled, T>::type
operator |(T a, T b) {
	typedef typename std::underlying_type<T>::type Type;

	return static_cast<T> (
		static_cast<Type>(a) |
		static_cast<Type>(b)
	);
}

template<typename T>
constexpr inline typename std::enable_if<BitmaskOperators<T>::enabled, T>::type
operator &(T a, T b) {
	typedef typename std::underlying_type<T>::type Type;

	return static_cast<T> (
		static_cast<Type>(a) &
		static_cast<Type>(b)
	);
}

template<typename T>
constexpr inline typename std::enable_if<BitmaskOperators<T>::enabled, T>::type
operator ^(T a, T b) {
	typedef typename std::underlying_type<T>::type Type;

	return static_cast<T> (
		static_cast<Type>(a) ^
		static_cast<Type>(b)
	);
}

template<typename T>
constexpr inline typename std::enable_if<BitmaskOperators<T>::enabled, T>::type
operator |=(T & a, T b) {
	typedef typename std::underlying_type<T>::type Type;

	return a = static_cast<T>(
		static_cast<Type>(a) |
		static_cast<Type>(b)
	);
}

template<typename T>
constexpr inline typename std::enable_if<BitmaskOperators<T>::enabled, T>::type
operator &=(T & a, T b) {
	typedef typename std::underlying_type<T>::type Type;

	return a = static_cast<T>(
		static_cast<Type>(a) &
		static_cast<Type>(b)
	);
}

template<typename T>
constexpr inline typename std::enable_if<BitmaskOperators<T>::enabled, T>::type
operator ^=(T & a, T b) {
	typedef typename std::underlying_type<T>::type Type;

	return a = static_cast<T>(
		static_cast<Type>(a) ^
		static_cast<Type>(b)
	);
}

template<typename T>
constexpr inline typename std::enable_if<BitmaskOperators<T>::enabled, bool>::type
Any(T e) {
	typedef typename std::underlying_type<T>::type Type;

	return static_cast<Type>(e) != 0;
}

template<typename T>
constexpr inline typename std::enable_if<BitmaskOperators<T>::enabled, bool>::type
None(T e) {
	typedef typename std::underlying_type<T>::type Type;

	return static_cast<Type>(e) == 0;
}

template<typename T>
constexpr inline typename std::enable_if<BitmaskOperators<T>::enabled, typename std::underlying_type<T>::type>::type
Value(T e) {
	typedef typename std::underlying_type<T>::type Type;

	return static_cast<Type>(e);
}

#endif // _BITMASK_OPERATORS_H_
