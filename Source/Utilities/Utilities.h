#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <any>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Utilities {

	void randomizeSeed();
	void randomSeed(unsigned int seed);
	int randomInteger(int min, int max, bool randomize = false);
	float randomFloat(float min, float max, bool randomize = false);
	std::string anyToString(const std::any & value);
	std::string anyVectorToString(const std::vector<std::any> & values);
	std::string anyMapToString(const std::map<std::string, std::any> & valueMap);
	template <typename D>
	std::string getDurationName();

	template <typename D>
	std::string getDurationName() {
		if(std::is_same<D, std::chrono::nanoseconds>::value) {
			return "Nanoseconds";
		}
		else if(std::is_same<D, std::chrono::microseconds>::value) {
			return "Microseconds";
		}
		else if(std::is_same<D, std::chrono::milliseconds>::value) {
			return "Milliseconds";
		}
		else if(std::is_same<D, std::chrono::seconds>::value) {
			return "Seconds";
		}
		else if(std::is_same<D, std::chrono::minutes>::value) {
			return "Minutes";
		}
		else if(std::is_same<D, std::chrono::hours>::value) {
			return "Hours";
		}
		else if(std::is_same<D, std::chrono::duration<int, std::ratio<86400>>>::value) {
			return "Days";
		}
		else if(std::is_same<D, std::chrono::duration<int, std::ratio<604800>>>::value) {
			return "Weeks";
		}
		else if(std::is_same<D, std::chrono::duration<int, std::ratio<2629746>>>::value) {
			return "Months";
		}
		else if(std::is_same<D, std::chrono::duration<int, std::ratio<31556952>>>::value) {
			return "Years";
		}

		return {};
	}

}

#endif // _UTILITIES_H_
