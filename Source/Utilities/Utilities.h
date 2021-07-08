#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <string>
#include <regex>
#include <iostream>

namespace Utilities {

	extern const char newLine[];
	extern bool initialRandomize;
	extern const char * monthStrings[];

	void randomizeSeed();
	void randomSeed(unsigned int seed);
	int randomInteger(int min, int max, bool randomize = false);
	float randomFloat(float min, float max, bool randomize = false);
	std::string generateFullPath(const char * path, const char * fileName);
	std::string generateFullPath(const std::string & path, const std::string & fileName);
	char * getFileNameNoExtension(const char * fileName);
	char * getFileExtension(const char * fileName);
	bool fileHasExtension(const char * fileName, const char * fileExtension);

}

#endif // _UTILITIES_H_
