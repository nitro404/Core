#include "Utilities.h"

#include "StringUtilities.h"

#include <regex>
#include <time.h>

static bool s_initialRandomize = true;

void Utilities::randomizeSeed() {
	srand(static_cast<unsigned int>(time(NULL)));
}

void Utilities::randomSeed(unsigned int seed) {
	srand(seed);
}

int Utilities::randomInteger(int min, int max, bool randomize) {
	if(max <= min) { return min; }

	if(randomize || s_initialRandomize) {
		Utilities::randomizeSeed();
		s_initialRandomize = false;
	}

	return (rand() % (max - min + 1)) + min;
}

float Utilities::randomFloat(float min, float max, bool randomize) {
	if(max <= min) { return min; }

	if(randomize || s_initialRandomize) {
		Utilities::randomizeSeed();
		s_initialRandomize = false;
	}

	return ((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (max - min)) + min;
}
