#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <any>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Utilities {

	void randomizeSeed();
	void randomSeed(unsigned int seed);
	int randomInteger(int min, int max, bool randomize = false);
	float randomFloat(float min, float max, bool randomize = false);
	std::string anyToString(const std::any & value);

}

#endif // _UTILITIES_H_
